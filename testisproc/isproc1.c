/* isproc */

/* is a process (specified by PID) currently in the system? */


#define	CF_DEBUGS	1		/* debug prints */
#define	CF_GETPGID	1		/* try to use 'u_getpgid(2)' */
#define	CF_KILL		0		/* use 'u_kill(2)' */
#define	CF_OTHER	0		/* try "other" techniques */
#define	CF_READPS	0		/* use 'ps(1)' only */
#define	CF_MEMCCPY	0		/* we are faster than 'memccpy()' */
#define	CF_FAKEPS	0		/* use a fake 'ps(1)' */
#define	CF_ISAEXEC	1		/* use 'isaexec(3c)' */
#define	CF_STATVFS	1		/* require 'statvfs(2)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

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

	Most processes that any single process might be interested in are
	also owned by the caller.  So we try to take advantage of this
	and use the 'u_getpgid(2)' or 'u_kill(2)' calls first to see if
	it gives us an "alive" indication or a "not present" indication.
	If it returns one of those two things, then we are good and we
	return the corresponding information to our caller.  However, if
	it returns something else, like "no permission" then we have to
	resort to harsher methods to determine if the process is present
	or not.

	If harder methds are required, we first try to use the PROC
	filesystem if we can.  If it is mounted, we use it and the pain
	wasn't too great.  However, if the PROC FS is not mounted we
	have to resort to using '/bin/ps(1)' and that is not the most
	pleasant experience (relatively slow)!	Some system administrators
	(I won't be naming any names) like to unmount some of the special
	file systems that were meant to help people (and processes) in
	the first place.  They (the admins) think that they are somehow
	doing a service to the "world" by not having some of the special
	file systems mounted!

	If we do have to resort to using '/bin/ps(1)' and if it fails for
	some weirdo reason, we just say that the PID that the caller is
	looking for is gone!   This could be wrong, but if '/bin/ps(1)'
	is failing, maybe the system is on its way to la-la land anyway!


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PROCDNAME
#define	PROCDNAME	"/proc"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"HOME"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#define	BUFLEN		(MAXPATHLEN + 20)

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	PIDBUFLEN
#define	PIDBUFLEN	20
#endif

#if	CF_FAKEPS
#define	PROG_PS1	"ps"
#define	PROG_PS2	"/usr/bin/ps"
#else
#define	PROG_PS1	"/bin/ps"
#define	PROG_PS2	"/usr/bin/ps"
#endif

#define	PIDBUF		struct pidbuf		/* local object */

#ifndef	NOFILE
#define	NOFILE		20	/* modern value is 64 (old value was 20) */
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct modinfo {
	pid_t	spid ;
	int	f_answer ;
} ;

struct pidbuf {
	char	*bp ;
	int	fd ;
	int	bufsize ;
	int	len ;
	char	buf[BUFLEN + 1] ;
} ;


/* forward references */

static int	modinfo_init(struct modinfo *,pid_t) ;
static int	modinfo_free(struct modinfo *) ;

static int	try_getpgid(struct modinfo *) ;
static int	try_kill(struct modinfo *) ;
static int	try_procfs(struct modinfo *) ;
static int	try_readps(struct modinfo *) ;

static int	progok(const char *) ;
static int	readps(const char *,pid_t) ;
static int	envadd(char **,int,const char *) ;

static int	pidbuf_start(PIDBUF *,int) ;
static int	pidbuf_readline(PIDBUF *,char *,int) ;
static int	pidbuf_readpid(PIDBUF *,int *) ;
static int	pidbuf_finish(PIDBUF *) ;


/* local variables */

static int	(*tries[])(struct modinfo *) = {
	try_readps,
	NULL
} ;

#ifdef	COMMENT
	try_getpgid,
	try_kill,
	try_procfs,
#endif /* COMMENT */






int isproc(spid)
pid_t	spid ;
{
	struct modinfo	mi, *mip = &mi ;

	int	rs = SR_NOSYS ;
	int	i ;
	int	f_answer ;


	modinfo_init(mip,spid) ;

	for (i = 0 ; tries[i] != NULL ; i += 1) {

	    rs = (*tries[i])(mip) ;

	    if (rs >= 0)
		break ;

	} /* end for */

	f_answer = modinfo_free(mip) ;

	return (rs >= 0) ? f_answer : FALSE ;
}
/* end subroutine (isproc) */



/* LOCAL SUBROUTINES */



static int modinfo_init(mip,spid)
struct modinfo	*mip ;
pid_t		spid ;
{


	mip->f_answer = FALSE ;
	mip->spid = spid ;
	return SR_OK ;
}
/* end subroutine (modinfo_init) */


static int modinfo_free(mip)
struct modinfo	*mip ;
{


	return mip->f_answer ;
}
/* end subroutine (modinfo_free) */


static int try_getpgid(mip)
struct modinfo	*mip ;
{
	int	rs ;


	rs = u_getpgid(mip->spid) ;

	if (rs == SR_SRCH)
	    mip->f_answer = FALSE ;

	else if (rs >= 0)
	    mip->f_answer = TRUE ;

	return (rs >= 0) ? mip->f_answer : rs ;
}
/* end subroutine (try_getpgid) */


static int try_kill(mip)
struct modinfo	*mip ;
{
	int	rs ;


	rs = u_kill(mip->spid,0) ;

	if (rs == SR_SRCH)
	    mip->f_answer = FALSE ;

	else if (rs >= 0)
	    mip->f_answer = TRUE ;

	return (rs >= 0) ? mip->f_answer : rs ;
}
/* end subroutine (try_kill) */


static int try_procfs(mip)
struct modinfo	*mip ;
{
	struct statvfs	vsb ;

	struct ustat	sb ;

	int	rs = SR_OK, rs1 ;
	int	v ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	digbuf[DIGBUFLEN + 1] ;
	char	*procdname = PROCDNAME ;


#if	CF_STATVFS /* require a 'statvfs(2)' confirmation */

	mkpath1(tmpfname,procdname) ;

	rs = u_statvfs(tmpfname,&vsb) ;

	if (rs < 0)
	    goto ret0 ;

	rs = SR_NOSYS ;
	if (strncmp(vsb.f_basetype,"proc",FSTYPSZ) != 0)
	    goto ret0 ;

#endif /* CF_STATVFS */

/* we have a PROC filesystem ('proc(4)') */

	v = (uint) mip->spid ;
	rs = ctdecui(digbuf,DIGBUFLEN,v) ;

	if (rs >= 0)
	    rs = mkpath2(tmpfname,procdname,digbuf) ;

	if (rs >= 0) {

	    rs1 = u_stat(tmpfname,&sb) ;

	    mip->f_answer = (rs1 >= 0) ? TRUE : FALSE ;

	}

ret0:
	return (rs >= 0) ? mip->f_answer : rs ;
}
/* end subroutine (try_procfs) */


static int try_readps(mip)
struct modinfo	*mip ;
{
	int	rs ;
	int	rs1 ;

	char	*progfname ;


	progfname = PROG_PS1 ;
	rs = progok(progfname) ;

	if (rs < 0) {

		progfname = PROG_PS2 ;
		rs = progok(progfname) ;

	}

	if (rs >= 0) {

	    rs1 = readps(progfname,mip->spid) ;

	    mip->f_answer = (rs1 > 0) ;
	}

	return (rs >= 0) ? mip->f_answer : rs ;
}
/* end subroutine (try_readps) */


static int progok(progfname)
const char	progfname[] ;
{
	struct ustat	sb ;

	int	rs ;


	rs = u_stat(progfname,&sb) ;

	if (rs >= 0) {

	    rs = SR_NOENT ;
	    if (S_ISREG(sb.st_mode) && ((S_IXOTH & sb.st_mode) != 0))
		rs = SR_OK ;

	}

	return rs ;
}
/* end subroutine (progok) */


static int readps(progfname,searchpid)
const char	progfname[] ;
pid_t		searchpid ;
{
	int	rs ;
	int	i, pfds[2] ;
	int	spid, pid, childstat ;
	int	fd ;
	int	f ;

	const char	*argv[5] ;
	const char	*envv[5] ;


	spid = (int) searchpid ;

	pfds[0] = -1 ;
	pfds[1] = -1 ;
	rs = u_pipe(pfds) ;

	if (rs < 0)
	    goto bad0 ;

/* we use pfds[0] for reading, so we take that one, the other is for child */

	rs = uc_fork() ;
	pid = (pid_t) rs ;
	if (rs == 0) {
	    const char	*nullfname = NULLFNAME ;

	    u_close(pfds[0]) ;

	    if ((fd = pfds[1]) < 3)
	        fd = uc_moveup(pfds[1],3) ;

	    for (i = 0 ; i < NOFILE ; i += 1) {
	        if (i != fd) u_close(i) ;
	    } /* end for */

	    u_open(nullfname,O_RDONLY,0666) ;

	    u_dup(fd) ;

	    u_open(nullfname,O_WRONLY,0666) ;

	    u_close(fd) ;

	    i = 0 ;
	    envv[i++] = "PATH=/usr/bin:/bin" ;

	    i = envadd(envv,i,VARLOGNAME) ;

	    i = envadd(envv,i,VARUSERNAME) ;

	    i = envadd(envv,i,VARLOGNAME) ;

	    envv[i] = NULL ;

	    argv[0] = "ps" ;
	    argv[1] = "-e" ;
	    argv[2] = "-o" ;
	    argv[3] = "pid" ;
	    argv[4] = NULL ;

#if	CF_DEBUGS
	debugprintf("isproc: execing %s\n",progfname) ;
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

	{
	    PIDBUF	pb ;

	    int		apid ;


	    pidbuf_start(&pb,pfds[0]) ;

	    apid = -1 ;
	    while ((rs = pidbuf_readpid(&pb,&apid)) > 0) {

#if	CF_DEBUGS
	    debugprintf("isproc/readps: pidbuf_readpid() rs=%d pid=%d\n",
		rs,apid) ;
#endif

	        if (spid == apid)
	            break ;

	    } /* end while */

	    pidbuf_finish(&pb) ;

	    f = ((rs > 0) && (spid == apid)) ;

	} /* end block */

/* close our pipe end and wait for child to exit */

	u_close(pfds[0]) ;

	u_waitpid(pid,&childstat,0) ;

/* we're out of here */
ret0:
	return (rs >= 0) ? f : rs ;

/* bad things come here */
bad2:
bad1:
	if (pfds[0] >= 0)
	    u_close(pfds[0]) ;

	if (pfds[1] >= 0)
	    u_close(pfds[1]) ;

bad0:
	goto ret0 ;
}
/* end subroutine (readps) */


static int envadd(envv,i,varname)
char		**envv ;
int		i ;
const char	*varname ;
{
	int	bl ;

	const char	*cp ;

	char	buf[BUFLEN + 1] ;


	    if ((cp = getenv(varname)) != NULL) {

		bl = sncpy3(buf,BUFLEN,varname,"=",cp) ;

	        envv[i++] = mallocstrw(buf,bl) ;

	    }

	return i ;
}
/* end subroutine (envadd) */


/* this is the PIDBUF stuff */

static int pidbuf_start(op,fd)
PIDBUF	*op ;
int	fd ;
{


	memset(op,0,sizeof(PIDBUF)) ;
	op->fd = fd ;
	op->bufsize = BUFLEN ;

	return SR_OK ;
}
/* end subroutine (pidbuf_start) */

static int pidbuf_readline(op,ubuf,ubuflen)
PIDBUF	*op ;
char	ubuf[] ;
int	ubuflen ;
{
	int	rs = SR_OK ;
	int	inc, mlen, tlen ;
	int	f_already ;

	char	*ubp ;


#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: ubuflen=%d\n",ubuflen) ;
#endif

	f_already = FALSE ;
	tlen = 0 ;
	ubp = ubuf ;
	while (ubuflen > 0) {

#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: op->len=%d\n",op->len) ;
#endif

	    if (op->len <= 0) {

#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: need refill, bufsize=%d\n",
		op->bufsize) ;
#endif

#ifdef	COMMENT
	        if (f_already)
	            break ;
#endif /* COMMENT */

	        rs = u_read(op->fd,op->buf,op->bufsize) ;

#if	CF_DEBUGS
	        debugprintf("pidbuf_readline: u_read() rs=%d\n",rs) ;
#endif

	        op->len = rs ;
	        if (rs <= 0)
	            break ;

	        if (op->len < op->bufsize)
	            f_already = TRUE ;

	        op->bp = op->buf ;

	    } /* end if (refilling up buffer) */

	    mlen = (op->len < ubuflen) ? op->len : ubuflen ;

#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: mlen=%d\n",mlen) ;
#endif

	    if (mlen > 0) {

	        register char	*lastp ;


#if	CF_MEMCCPY
	        if ((lastp = memccpy(ubp,fp->bp,'\n',mlen)) == NULL)
	            lastp = ubp + mlen ;

	        inc = lastp - ubp ;
	        ubp += inc ;
	        op->bp += inc ;
#else
	        {
	            register char	*bp ;


	            bp = op->bp ;
	            lastp = op->bp + mlen ;
	            while (bp < lastp) {

	                if ((*ubp++ = *bp++) == '\n')
	                    break ;

	            }

	            inc = (bp - op->bp) ;
	            op->bp += inc ;

	        } /* end block */
#endif /* CF_MEMCCPY */

#if	CF_DEBUGS
	        debugprintf("pidbuf_readline: inc=%d\n",inc) ;
#endif

	        op->len -= inc ;
	        tlen += inc ;
	        if ((inc > 0) && (ubp[-1] == '\n'))
	            break ;

	        ubuflen -= mlen ;

	    } /* end if (move it) */

#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: bottom while\n") ;
#endif

	} /* end while (trying to satisfy request) */

#if	CF_DEBUGS
	    debugprintf("pidbuf_readline: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (pidbuf_readline) */

static int pidbuf_readpid(op,pp)
PIDBUF	*op ;
int	*pp ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	len ;

	char	pidbuf[PIDBUFLEN + 1] ;


	while ((rs = pidbuf_readline(op,pidbuf,PIDBUFLEN)) > 0) {

	    len = rs ;
	    pidbuf[--len] = '\0' ;

#if	CF_DEBUGS
	debugprintf("pidbuf_readpid: pidbuf_readline() rs=%d\n",rs) ;
	debugprintf("pidbuf_readpid: sbuf=>%t<\n",
		pidbuf,len) ;
#endif

	    rs1 = cfdeci(pidbuf,len,pp) ;

	    if (rs1 >= 0)
		break ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("pidbuf_readpid: cfdeci() rs=%d pid=%d\n",rs,*pp) ;
#endif

	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (pidbuf_readpid) */

static int pidbuf_finish(op)
PIDBUF	*op ;
{


	op->bp = NULL ;
	op->buf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (pidbuf_finish) */



