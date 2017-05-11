/* isproc */

/* is a process (specified by PID) currently in the system? */


#define	CF_DEBUGS	0		/* debug prints */
#define	CF_GETPGID	1		/* try to use 'u_getpgid(2)' */
#define	CF_KILL		1		/* try to use 'u_kill(2)' */
#define	CF_READPS	1		/* try to use 'ps(1)' */
#define	CF_FAKEPS	0		/* use a fake 'ps(1)' */
#define	CF_ISAEXEC	1		/* use 'isaexec(3c)' */
#define	CF_STATVFS	1		/* require 'statvfs(2)' */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks to see if the specific process identified by its
        PID, is still on the system. Note that even zombie processes can satisfy
        the search requirement on some systems!

	Synopsis:

	int isproc(pid)
	pid_t	pid ;

	Arguments:

	pid			PID of process to search for

	Returns:

	TRUE			process was found on system
	FALSE			process was not found on system

	Implementation strategy:

        Most processes that any single process might be interested in are also
        owned by the caller. So we try to take advantage of this and use the
        'u_getpgid(2)' or 'u_kill(2)' calls first to see if it gives us an
        "alive" indication or a "not present" indication. If it returns one of
        those two things, then we are good and we return the corresponding
        information to our caller. However, if it returns something else, like
        "no permission" then we have to resort to harsher methods to determine
        if the process is present or not.

        If harder methds are required, we first try to use the PROC filesystem
        if we can. If it is mounted, we use it and the pain wasn't too great.
        However, if the PROC FS is not mounted we have to resort to using
        '/bin/ps(1)' and that is not the most pleasant experience (relatively
        slow)! Some system administrators (I won't be naming any names) like to
        unmount some of the special file systems that were meant to help people
        (and processes) in the first place. They (the admins) think that they
        are somehow doing a service to the "world" by not having some of the
        special file systems mounted!

        If we do have to resort to using '/bin/ps(1)' and if it fails for some
        weirdo reason, we just say that the PID that the caller is looking for
        is gone! This could be wrong, but if '/bin/ps(1)' is failing, maybe the
        system is on its way to la-la land anyway!

	Note:

        On BSD systems, 'pipe(2)' does not open both ends of the pipe for both
        reading and writing, so we observe the old BSD behavior of the zeroth
        element FD being only open for reading and the oneth element FD only
        open for writing.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<filebuf.h>
#include	<spawnproc.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PROCDNAME
#define	PROCDNAME	"/proc"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	ASFNAME
#define	ASFNAME		"as"		/* Solaris "Address Space" file */
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARHZ
#define	VARHZ		"HZ"
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	PIDBUFLEN
#define	PIDBUFLEN	200		/* we only want to look at this much */
#endif

#define	BUFLEN		(MAXPATHLEN + 20)

#if	CF_FAKEPS
#define	PROG_PS1	"ps"
#define	PROG_PS2	"/usr/bin/ps"
#else
#define	PROG_PS1	"/bin/ps"
#define	PROG_PS2	"/usr/bin/ps"
#endif

#define	PIDBUF		struct pidbuf		/* local object */
#define	PIDBUF_ENT	struct pidbuf_e

#ifndef	NOFILE
#define	NOFILE		20	/* modern value is 64 (old value was 20) */
#endif

#define	TO_READ		4	/* time-out for reading from process pipe */

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo {
	pid_t	spid ;
	int	f_present ;
} ;

struct pidbuf_e {
	pid_t	pid ;
	int	state ;			/* this is a character */
} ;

struct pidbuf {
	FILEBUF	fb ;
	int	to ;
	int	fn_pid ;
	int	fn_state ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,pid_t) ;
static int	subinfo_finish(SUBINFO *) ;

static int	try_getpgid(SUBINFO *) ;
static int	try_kill(SUBINFO *) ;
static int	try_procfs(SUBINFO *) ;
static int	try_readps(SUBINFO *) ;

static int	progok(const char *) ;
static int	readps(const char *,pid_t) ;
static int	readpsdata(int,pid_t) ;
static int	envaddprog(const char **,int,const char *) ;
static int	envaddvar(const char **,int,const char *) ;

#ifdef	COMMENT
static int	matfield(const char *,const char *,int) ;
#endif /* COMMENT */

static int	pidbuf_start(PIDBUF *,int) ;
static int	pidbuf_readpid(PIDBUF *,PIDBUF_ENT *) ;
static int	pidbuf_parsefmt(PIDBUF *,char *,int) ;
static int	pidbuf_parsedata(PIDBUF *,PIDBUF_ENT *,char *,int) ;
static int	pidbuf_trashline(PIDBUF *) ;
static int	pidbuf_finish(PIDBUF *) ;


/* local variables */

static int	(*tries[])(SUBINFO *) = {
	try_getpgid,
	try_kill,
	try_procfs,
	try_readps,
	NULL
} ;

static const char	*envs[] = {
	VARHOME,
	VARUSERNAME,
	VARLOGNAME,
	VARPWD,
	VARHZ,
	NULL
} ;

static const char	*matfields[] = {
	"pid",
	"s",
	NULL
} ;

enum {
	matfield_pid,
	matfield_state,
	matfield_overlast
} ;


/* exported subroutines */


int isproc(pid_t spid)
{
	SUBINFO		mi, *sip = &mi ;
	int		rs ;
	int		f_present = FALSE ;

	if ((rs = subinfo_start(sip,spid)) >= 0) {
	    int		i ;
	    for (i = 0 ; tries[i] != NULL ; i += 1) {
	        rs = (*tries[i])(sip) ;
	        if (rs < 0) break ;
	        if (rs > 0) break ;
	    } /* end for */
	    f_present = subinfo_finish(sip) ;
	} /* end if (subinfo) */

	return (rs >= 0) ? f_present : FALSE ;
}
/* end subroutine (isproc) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,pid_t spid)
{

	sip->f_present = FALSE ;
	sip->spid = spid ;
	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{

	return sip->f_present ;
}
/* end subroutine (subinfo_finish) */


static int try_getpgid(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_done = FALSE ;

	rs1 = u_getpgid(sip->spid) ;

	if (rs1 == SR_SRCH) {
	    f_done = TRUE ;
	    sip->f_present = FALSE ;
	}

	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (try_getpgid) */


static int try_kill(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_done = FALSE ;

	rs1 = u_kill(sip->spid,0) ;

	if (rs1 == SR_SRCH) {
	    f_done = TRUE ;
	    sip->f_present = FALSE ;
	}

	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (try_kill) */


static int try_procfs(SUBINFO *sip)
{
	struct statvfs	vsb ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		v ;
	int		f_done = FALSE ;
	const char	*procdname = PROCDNAME ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		digbuf[DIGBUFLEN + 1] ;

#if	CF_STATVFS /* require a 'statvfs(2)' confirmation */

	rs1 = u_statvfs(procdname,&vsb) ;
	if (rs1 < 0)
	    goto ret0 ;

	if (strncmp(vsb.f_basetype,"proc",FSTYPSZ) != 0)
	    goto ret0 ;

#endif /* CF_STATVFS */

/* we have a PROC filesystem ('proc(4)') */

	v = (uint) sip->spid ;
	rs = ctdecui(digbuf,DIGBUFLEN,v) ;

	if (rs >= 0)
	    rs = mkpath3(tmpfname,procdname,digbuf,ASFNAME) ;

	if (rs >= 0) {
	    rs1 = u_stat(tmpfname,&sb) ;
	    sip->f_present = (rs1 >= 0) ? TRUE : FALSE ;
	    f_done = TRUE ;
	}

ret0:
	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (try_procfs) */


static int try_readps(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_done = FALSE ;
	char		*progfname ;

	progfname = PROG_PS1 ;
	rs1 = progok(progfname) ;
	if (rs1 < 0) {
	    progfname = PROG_PS2 ;
	    rs1 = progok(progfname) ;
	}

	if (rs1 >= 0) {
	    rs1 = readps(progfname,sip->spid) ;
	    sip->f_present = (rs1 > 0) ;
	    f_done = TRUE ;		/* always true with this */
	}

	return (rs >= 0) ? f_done : rs ;
}
/* end subroutine (try_readps) */


static int progok(cchar *progfname)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(progfname,&sb)) >= 0) {
	    rs = SR_NOENT ;
	    if (S_ISREG(sb.st_mode) && ((S_IXOTH & sb.st_mode) != 0))
	        rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (progok) */


static int readps(cchar *progfname,pid_t spid)
{
	int		rs ;
	int		i ;
	int		pid, childstat ;
	int		pfds[2] ;
	int		f = FALSE ;

	pfds[0] = -1 ;
	pfds[1] = -1 ;
	rs = u_pipe(pfds) ;
	if (rs < 0)
	    goto bad0 ;

/* we use pfds[0] for reading, so we take that one, the other is for child */

	rs = uc_fork() ;
	pid = rs ;
	if (pid == 0) {
	    int		fd, j ;
	    const char	*argv[5] ;
	    const char	*envv[nelem(envs) + 3] ;
	    const char	*nullfname = NULLFNAME ;

	    u_close(pfds[0]) ;

	    if ((fd = pfds[1]) < 3)
	        fd = uc_moveup(pfds[1],3) ;

	    for (i = 0 ; i < NOFILE ; i += 1) {
	        if (i != fd)
	            u_close(i) ;
	    } /* end for */

	    u_open(nullfname,O_RDONLY,0666) ;	/* standard-input */

	    u_dup(fd) ;				/* standard-output */

	    u_open(nullfname,O_WRONLY,0666) ;	/* standard-error */

	    u_close(fd) ;

	    i = 0 ;
	    i = envaddprog(envv,i,progfname) ;

	    envv[i++] = "PATH=/usr/bin:/bin" ;

	    for (j = 0 ; envs[j] != NULL ; j += 1)
	        i = envaddvar(envv,i,envs[j]) ;

	    envv[i] = NULL ;

	    i = 0 ;
	    argv[i++] = "ps" ;
	    argv[i++] = "-el" ;
	    argv[i++] = NULL ;

#if	CF_DEBUGS
	    debugprintf("isproc: progfname=%s\n",progfname) ;
#endif

#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    rs = uc_isaexecve(progfname,argv,envv) ;
#endif

	    rs = uc_execve(progfname,argv,envv) ;

#if	CF_DEBUGS
	    debugprintf("isproc: u_execv() rs=%d\n",rs) ;
#endif

	    uc_exit(EX_NOEXEC) ;

	} else if (rs < 0)
	    goto bad1 ;

/* close ours that is open for writing (the child does the writing) */

	u_close(pfds[1]) ;

	pfds[1] = -1 ;

/* read the data from the child */

	rs = readpsdata(pfds[0],spid) ;
	f = (rs > 0) ;

/* close our pipe end and wait for child to exit */

	u_close(pfds[0]) ;

	pfds[0] = -1 ;
	u_waitpid(pid,&childstat,0) ;

/* we're out of here */
ret0:
	return (rs >= 0) ? f : rs ;

/* bad things come here */
bad1:
	if (pfds[0] >= 0)
	    u_close(pfds[0]) ;

	if (pfds[1] >= 0)
	    u_close(pfds[1]) ;

bad0:
	goto ret0 ;
}
/* end subroutine (readps) */


static int readpsdata(int fd,pid_t spid)
{
	PIDBUF		pb ;
	PIDBUF_ENT	pe ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = pidbuf_start(&pb,fd)) >= 0) {

	    while ((rs = pidbuf_readpid(&pb,&pe)) > 0) {
	        if (spid == pe.pid) break ;
	    } /* end while */

	f = (rs > 0) ;
	f = f && (spid == pe.pid) ;
	f = f && (tolower(pe.state) != 'z') ;

	    pidbuf_finish(&pb) ;
	} /* end if (pidbuf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (readpsdata) */


static int envaddprog(cchar **envv,int i,cchar *progfname)
{
	int		bl ;
	char		buf[BUFLEN + 1] ;

	bl = sncpy3(buf,BUFLEN,"_","=",progfname) ;

	if (bl >= 0)
	    envv[i++] = mallocstrw(buf,bl) ;

	return i ;
}
/* end subroutine (envaddprog) */


static int envaddvar(cchar **envv,int i,cchar *varname)
{
	const char	*cp ;

	if ((cp = getenv(varname)) != NULL) {
	    int		bl ;
	    char	buf[BUFLEN + 1] ;
	    if ((bl = sncpy3(buf,BUFLEN,varname,"=",cp)) >= 0) {
	        envv[i++] = mallocstrw(buf,bl) ;
	    }
	}

	return i ;
}
/* end subroutine (envaddvar) */


/* this is the PIDBUF stuff */

static int pidbuf_start(PIDBUF *op,int fd)
{
	int		rs ;
	int		len = 0 ;
	char		pidbuf[PIDBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (fd < 0) return SR_BADF ;

	memset(op,0,sizeof(PIDBUF)) ;

	op->to = TO_READ ;
	rs = filebuf_start(&op->fb,fd,0,0,0) ;

	if (rs >= 0) {
	    rs = filebuf_readline(&op->fb,pidbuf,PIDBUFLEN,op->to) ;
	    len = rs ;
	    if ((rs > 0) && (pidbuf[len-1] != '\n'))
	        rs = pidbuf_trashline(op) ;
	}

	if ((rs >= 0) && (len > 0))
	    rs = pidbuf_parsefmt(op,pidbuf,len) ;

	return rs ;
}
/* end subroutine (pidbuf_start) */


static int pidbuf_readpid(PIDBUF *op,PIDBUF_ENT *ep)
{
	int		rs ;
	int		len ;
	int		f_eol = FALSE ;
	int		f = FALSE ;
	char		pidbuf[PIDBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if ((rs = filebuf_readline(&op->fb,pidbuf,PIDBUFLEN,op->to)) >= 0) {
	    len = rs ;
	    f_eol = ((len > 0) && (pidbuf[len-1] == '\n')) ;
	    if (f_eol) len -= 1 ;
	    pidbuf[len] = '\0' ;
	    if ((len > 0) && (! f_eol)) {
	        rs = pidbuf_trashline(op) ;
	    }
	    if (rs >= 0) {
		rs = pidbuf_parsedata(op,ep,pidbuf,len) ;
		f = (rs > 0) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("pidbuf_readpid: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pidbuf_readpid) */

static int pidbuf_parsefmt(PIDBUF *op,char *pidbuf,int len)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		i ;
	int		fn = 0 ;
	int		f_pid = FALSE ;
	int		f_state = FALSE ;
	int		f = FALSE ;
	const char	*sp, *cp ;

	sp = pidbuf ;
	sl = len ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    if ((i = matcasestr(matfields,cp,cl)) >= 0) {
	        switch (i) {
	        case matfield_pid:
	            f_pid = TRUE ;
	            op->fn_pid = fn ;
	            break ;
	        case matfield_state:
	            f_state = TRUE ;
	            op->fn_state = fn ;
	            break ;
	        } /* end switch */
	    } /* end if */

	    f = (f_state && f_pid) ;
	    if (f)
	        break ;

	    fn += 1 ;
	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	} /* end while */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pidbuf_parsefmt) */


static int pidbuf_parsedata(PIDBUF *op,PIDBUF_ENT *ep,char *pidbuf,int len)
{
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		sl, cl ;
	int		fn = 0 ;
	int		v ;
	int		f_pid = FALSE ;
	int		f_state = FALSE ;
	int		f = FALSE ;
	const char	*sp, *cp ;

	sp = pidbuf ;
	sl = len ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    if (fn == op->fn_pid) {

	        f_pid = TRUE ;
	        ep->pid = 0 ;
	        rs1 = cfdeci(cp,cl,&v) ;
	        if (rs1 >= 0) {
	            ep->pid = (pid_t) v ;
	        }

#if	CF_DEBUGS
	        debugprintf("isproc/pidbuf_parsedata: pid rs1=%d v=%u\n",
			rs1,v) ;
#endif

	    } else if (fn == op->fn_state) {

	        f_state = TRUE ;
	        ep->state = (uint) cp[0] ;

#if	CF_DEBUGS
	        debugprintf("isproc/pidbuf_parsedata: state=%c\n",cp[0]) ;
#endif

	    } /* end if */

	    f = (f_state && f_pid) ;
	    if (f)
	        break ;

	    fn += 1 ;
	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("isproc/pidbuf_parsedata: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pidbuf_parsedata) */

static int pidbuf_trashline(PIDBUF *op)
{
	int		rs = SR_OK ;
	int		f_eol = FALSE ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((rs >= 0) && (! f_eol)) {
	    rs = filebuf_readline(&op->fb,lbuf,LINEBUFLEN,op->to) ;
	    f_eol = ((rs > 0) && (lbuf[rs-1] == '\n')) ;
	}

	return (rs >= 0) ? f_eol : rs ;
}
/* end subroutine (pidbuf_trashline) */

static int pidbuf_finish(PIDBUF *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = filebuf_finish(&op->fb) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pidbuf_finish) */


#ifdef	COMMENT
static int matfield(cchar *ts,cchar *s,int slen)
{
	int		m ;
	m = nleadcasestr(ts,s,slen) ;
	return (m == slen) ;
}
/* end subroutine (matfield) */
#endif /* COMMENT */


