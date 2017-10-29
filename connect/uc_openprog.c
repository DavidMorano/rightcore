/* uc_openprog */

/* interface component for UNIX® library-3c */
/* connect to a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* compile-time child */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer to connect to a local program.

	Synopsis:

	int uc_openprog(fname,oflags,argv,envv)
	const char	fname[] ;
	int		oflags ;
	const char	*argv[] ;
	const char	*envv[] ;

	Arguments:

	fname		program to execute
	oflags		options to specify read-only or write-only
	argv		arguments to program
	envv		environment to program

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error


	Note:

	On BSD systems, 'pipe(2)' does not open both ends of the pipe for both
	reading and writing, so we observe the old BSD behavior of the zeroth
	element FD being only open for reading and the oneth element FD only
	open for writing.

	Importand note on debugging:

	One some (maybe many) OS systems, turning on any debugging in this
	subroutine can cause hangs after the 'fork(2)'.  This is due to the
	famous (infamous) fork-safety problem on many UNIXi®.  One UNIX® OS
	that has fork-safe lib-C subroutines (for the greater most part) is
	Solaris®.  They (the Solaris® people) seem to be among the only ones
	who took fork-safety seriously in their OS.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<mkprogenv.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"		/* program-root */
#endif

#ifndef	VARPREXTRA
#define	VARPREXTRA	"EXTRA"		/* program-root */
#endif

#define	TO_CREAD	10

#ifndef	NDF
#define	NDF		"child.deb"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3w(char *,int,const char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	hasvarpathprefix(const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	mkuserpath(char *,const char *,const char *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,cchar *,int) ;
extern int	vecstr_addcspath(vecstr *) ;
extern int	ignoresigs(const int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward reference */

static int	mkepath(char *,cchar *) ;
static int	mkfindpath(char *,cchar *) ;
static int	openproger(cchar *,int,cchar **,cchar **) ;
static int	spawnit(const char *,int,const char **,const char **) ;
static int	accmode(int) ;


/* local variables */

enum accmodes {
	accmode_rdonly,
	accmode_wronly,
	accmode_rdwr,
	accmode_overlast
} ;

static const int	sigignores[] = {
	SIGTERM,
	SIGINT,
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
	0
} ;


/* exported subroutines */


int uc_openprog(cchar *progname,int of,cchar *argv[],cchar *envv[])
{
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("uc_openprog: ent progname=%s\n",progname) ;
	if (argv != NULL) {
	    int	i ;
	    for (i = 0 ; argv[i] != NULL; i += 1) {
		debugprintf("uc_openprog: a[%u]=%s\n",i,argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if (progname == NULL) return SR_FAULT ;

	if (progname[0] == '\0') return SR_INVALID ;

	while ((progname[0] == '/') && (progname[1] == '/')) {
	    progname += 1 ;
	}

/* argument check */

	if ((rs = accmode(of)) >= 0) {
		    cchar	*pfn = progname ;
		    char	ebuf[MAXPATHLEN + 1] ;
		    if ((rs = mkepath(ebuf,progname)) >= 0) {
			if (rs > 0) pfn = ebuf ;
			rs = openproger(pfn,of,argv,envv) ;
			fd = rs ;
		    } /* end if (mkepath) */
	} /* end if (accmode) */

#if	CF_DEBUGS
	debugprintf("uc_openprog: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openprog) */


/* local subroutines */


static int mkepath(char *ebuf,cchar *pfn)
{
	int		rs ;
	int		el = 0 ;
	if ((rs = mkvarpath(ebuf,pfn,-1)) >= 0) {
	    if (rs > 0) {
	        el = rs ;
		pfn = ebuf ;
	    }
	    if (strncmp(pfn,"/u/",3) == 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mkuserpath(tbuf,NULL,pfn,-1)) > 0) {
		    el = rs ;
	            rs = mkpath1(ebuf,tbuf) ;
	            pfn = ebuf ;
		}
	    }
	    if ((rs >= 0) && (strchr(pfn,'/') == NULL)) {
		const int	nrs = SR_NOENT ;
		if ((rs = perm(pfn,-1,-1,NULL,X_OK)) == nrs) {
		    cchar	*tp ;
		    rs = SR_OK ;
		    if ((tp = strchr(pfn,':')) != NULL) {
			if (((tp-pfn) == 3) && (strncmp(pfn,"sys",3) == 0)) {
			    pfn = (tp+1) ;
			}
		    }
		    if (rs >= 0) {
		        rs = mkfindpath(ebuf,pfn) ;
		        el = rs ;
		    }
		} /* end if (perm) */
	    } /* end if (ok) */
	} /* end if (mkvarpath) */
	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkepath) */


static int mkfindpath(char *ebuf,cchar *pfn)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	int		el = 0 ;
	if ((rs = ids_load(&id)) >= 0) {
	    vecstr	ps ;
	    if ((rs = vecstr_start(&ps,5,0)) >= 0) {
		if ((rs = vecstr_addcspath(&ps)) >= 0) {
		    char	tbuf[MAXPATHLEN+1] ;
		    if ((rs = getprogpath(&id,&ps,tbuf,pfn,-1)) >= 0) {
			el = rs ;
			rs = mkpath1w(ebuf,tbuf,rs) ;
		    }
		} /* end if (vecstr_addcspath) */
		rs1 = vecstr_finish(&ps) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */
	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkfindpath) */


static int openproger(cchar *pfn,int of,cchar **av,cchar **ev)
{
	MKPROGENV	pe ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	if ((rs = mkprogenv_start(&pe,ev)) >= 0) {
	    const char	**argv = av ;
	    const char	**envv ;

#if	CF_DEBUGS
	debugprintf("uc_openprog: argv[0]=%s\n",argv[0]) ;
#endif

	    if ((rs = mkprogenv_envset(&pe,"_EF",pfn,-1)) >= 0) {
		int		al = -1 ;
	        const char	*ap = NULL ;
		if (argv != NULL) ap = argv[0] ;
		if (ap == NULL) al = sfbasename(pfn,-1,&ap) ;
	        if ((rs = mkprogenv_envset(&pe,"_A0",ap,al)) >= 0) {
		    if ((rs = mkprogenv_getvec(&pe,&envv)) >= 0) {
	                rs = spawnit(pfn,of,argv,envv) ;
		        fd = rs ;
		    }
	        }
	    } /* end if (mkprovenv-stuff) */

	    rs1 = mkprogenv_finish(&pe) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mkprogenv) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproger) */


static int spawnit(progfname,of,argv,ev)
const char	*progfname ;
int		of ;
const char	**argv ;
const char	**ev ;
{
	const int	am = accmode(of) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		crs ;
	int		i ;
	int		dfd[2] ;
	int		cfd[2] ;

/* create the data-IPC pipe */

	dfd[0] = -1 ;
	dfd[1] = -1 ;
	rs = u_pipe(dfd) ;
	if (rs < 0) goto badpipe ;

	if ((rs >= 0) && (of & O_NDELAY)) rs = uc_ndelay(dfd[0],TRUE) ;
	if ((rs >= 0) && (of & O_NONBLOCK)) rs = uc_nonblock(dfd[0],TRUE) ;

	if (rs < 0) goto badcfd ;

/* create the control-IPC pipe */

	cfd[0] = -1 ;
	cfd[1] = -1 ;
	rs = u_pipe(cfd) ;
	if (rs < 0)
	    goto badcfd ;

/* fork the child */

	rs = uc_fork() ;

#if	CF_DEBUGS
	debugprintf("uc_openprog: uc_fork() rs=%d\n",rs) ;
#endif

	if (rs == 0) {
	    int		fd_data, fd_control ;
	    const char	**av ;
	    const char	*arg[2] ;
	    const char	*nullfname = NULLFNAME ;

	    if (of & O_NOCTTY) {
		ignoresigs(sigignores) ;
		setsid() ;
	    }

	    u_close(dfd[0]) ;

	    u_close(cfd[0]) ;

	    for (i = 3 ; i < NOFILE ; i += 1) {
	        if ((i != dfd[1]) && (i != cfd[1])) {
	            u_close(i) ;
		}
	    }

	    fd_data = (dfd[1] >= 3) ? dfd[1] : uc_moveup(dfd[1],3) ;

	    fd_control = (cfd[1] >= 3) ? cfd[1] : uc_moveup(cfd[1],3) ;

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

#if	CF_DEBUGN
	    nprintf(NDF,"uc_openprog: am=%u\n",am) ;
#endif

/* standard-input */

	    if (am == accmode_rdonly) {
	        u_open(nullfname,O_RDONLY,0666) ;
	    } else
	        u_dup(fd_data) ;

/* standard-output */

	    if (am == accmode_wronly) {
	        u_open(nullfname,O_WRONLY,0666) ;
	    } else
	        u_dup(fd_data) ;

/* standard-error */

	    u_open(nullfname,O_WRONLY,0666) ;

/* close the original data-IPC pipe */

	    u_close(fd_data) ;

	    uc_closeonexec(fd_control,TRUE) ;

/* arguments */

	    av = argv ;
	    if (argv == NULL) {
	        av = arg ;
	        arg[0] = progfname ;
	        arg[1] = NULL ;
	    } /* end if (argument check) */

#if	CF_DEBUGN
	    {
	        nprintf(NDF,"uc_openprog: args:\n") ;
	        for (i = 0 ; av[i] != NULL ; i += 1)
	            nprintf(NDF,"uc_openprog: a[%u]=>%s<\n",i,av[i]) ;
	    }
#endif

#if	CF_DEBUGN
	    {
	        nprintf(NDF,"uc_openprog: envs:\n") ;
	        for (i = 0 ; ev[i] != NULL ; i += 1)
	            nprintf(NDF,"uc_openprog: e[%u]=>%s<\n",i,ev[i]) ;
	    }
#endif

	    if (rs >= 0) {
	        const char **cav = (const char **) av ;
	        const char **cev = (const char **) ev ;
	        rs = u_execve(progfname,cav,cev) ;
	    }

#if	CF_DEBUGN
	    nprintf(NDF,"uc_openprog: u_execve() rs=%d\n",rs) ;
#endif

	    u_write(fd_control,&rs,sizeof(int)) ;

#ifdef	OPTIONAL
	    u_close(fd_control]) ;
#endif

	    uc_exit(EX_NOEXEC) ;

	} else if (rs < 0)
	    goto badfork ;

/* close the ends of the pipes that we are not using */

	u_close(dfd[1]) ;
	dfd[1] = -1 ;

	u_close(cfd[1]) ;
	cfd[1] = -1 ;

/* get the disposition back from the child */

	rs1 = uc_reade(cfd[0],&crs,sizeof(int),TO_CREAD,0) ;

#if	CF_DEBUGS
	debugprintf("uc_openprog: uc_reade() rs=%d\n",rs1) ;
#endif

	if (rs1 > 0)
	    rs = (rs1 == sizeof(int)) ? crs : SR_NOEXEC ;

#if	CF_DEBUGS
	debugprintf("uc_openprog: child rs=%d\n",rs) ;
#endif

/* close the control-IPC pipe */

	u_close(cfd[0]) ;
	cfd[0] = -1 ;

/* prepare to get out */

#if	CF_DEBUGS
	debugprintf("uc_openprog: prepare rs=%d fd=%d\n",rs,dfd[0]) ;
#endif

	if ((rs >= 0) && (of & O_CLOEXEC))
	    rs = uc_closeonexec(dfd[0],TRUE) ;

	if (rs < 0)
	    u_close(dfd[0]) ;

/* finish up and get out */
ret1:
badcfd:
	for (i = 0 ; i < 2 ; i += 1) {
	    if (cfd[i] >= 0)
	        u_close(cfd[i]) ;
	}

badpipe:
	return (rs >= 0) ? dfd[0] : rs ;

/* bad stuff */
badfork:
	for (i = 0 ; i < 2 ; i += 1) {
	    if (dfd[i] >= 0)
	        u_close(dfd[i]) ;
	}
	goto ret1 ;
}
/* end subroutine (spawnit) */


static int accmode(int oflags)
{
	const int	am = (oflags & O_ACCMODE) ;
	int		rs = SR_INVALID ;
	switch (am) {
	case (O_RDONLY):
	    rs = accmode_rdonly ;
	    break ;
	case (O_WRONLY):
	    rs = accmode_wronly ;
	    break ;
	case (O_RDWR):
	    rs = accmode_rdwr ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (accmode) */


