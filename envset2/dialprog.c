/* dialprog */

/* connect to a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ENVSORT	0		/* sort the environment? */
#define	CF_MKVARPATH	0		/* somehow use 'mkvarpath(3dam)' */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer to connect to a local program.

	Synopsis:

	int dialprog(fname,of,argv,envv,fd2p)
	const char	fname[] ;
	int		of ;
	char		*argv[] ;
	char		*envv[] ;
	int		*fd2p ;

	Arguments:

	fname		program to execute
	of		open-flags
	argv		arguments to program
	envv		environment to program
	fd2p		pointer to integer to receive STDERR descriptor

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error

	fd2p		if it was supplied, the pointed-to integer
			received a file descriptor to the STDERR


	Importand note on debugging:

	One some (maybe many) OS systems, turning on any debugging in this
	subroutine can cause hangs after the 'fork(2)'.  This is due to the
	famous (infamous) fork-safety problem on many UNIX®i®.  One UNIX® OS
	that has fork-safe lib-C subroutines (for the greater most part) is
	Solaris®.  They (the Solaris® people) seem to be among the only ones
	who took fork-safety seriously in their OS.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mkprogenv.h>
#include	<ids.h>
#include	<localmisc.h>
#include	<exitcodes.h>


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXHOSTNAMELEN + 40)
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"		/* program-root */
#endif

#ifndef	VARPREXTRA
#define	VARPREXTRA	"EXTRA"		/* program-root */
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
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

#ifndef	VARDISPLAY
#define	VARDISPLAY	"DISPLAY"
#endif

#define	TO_CREAD	10

#define	DEBFNAME	"child.deb"


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	hasvarpathprefix(const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	findxfile(IDS *,char *,const char *) ;
extern int	ignoresigs(const int *) ;

#if	CF_DEBUGS
extern int	nprintf(const char *,const char *,...) ;
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward reference */

static int spawnit(cchar *,cchar *,int,cchar **,cchar **,int *) ;


/* local variables */

static const int	sigignores[] = {
	SIGTERM,
	SIGINT,
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
	0
} ;


/* exported subroutines */


int dialprog(fname,of,argv,envv,fd2p)
const char	fname[] ;
int		of ;
const char	*argv[] ;
const char	*envv[] ;
int		*fd2p ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;
	const char	*progfname ;
	const char	*execfname ;
	char		pwd[MAXPATHLEN + 1] = { 0 } ;
	char		buf[MAXPATHLEN + 3], *tmpfname = (buf + 2) ;

#if	CF_MKVARPATH
	char	eprogfname[MAXPATHLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("dialprog: ent fname=%s argv[0]=%s\n",
	    fname,argv[0]) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dialprog: got in\n") ;
#endif

/* start in */

#if	CF_MKVARPATH && defined(COMMENT)
	rs = mkvarpath(eprogfname,progfname,-1) ;
	if (rs > 0) progfname = eprogfname ;
	if (rs < 0) goto badarg ;
#endif

/* find the program */

	progfname = fname ;
	execfname = fname ;
	if ((rs >= 0) && (fname[0] != '/')) {

	    if (pwd[0] == '\0')
	        rs = getpwd(pwd,MAXPATHLEN) ;

	    progfname = tmpfname ;
	    if (rs >= 0)
	        rs = mkpath2(tmpfname,pwd,fname) ;

	    if ((rs >= 0) && (strchr(fname,'/') == NULL)) {

	        rs = perm(tmpfname,-1,-1,NULL,X_OK) ;

	        if (rs == SR_NOENT) {
		    IDS	id ;

		    if ((rs = ids_load(&id)) >= 0) {

	                rs = findxfile(&id,tmpfname,fname) ;

		        ids_release(&id) ;
		    } /* end if */

	            if (rs > 0) {
	                progfname = tmpfname ;
	                execfname = tmpfname ;
	            } else
	                rs = mkpath1(tmpfname,fname) ;

	        } /* end if */

	    } /* end if (relative program filename) */

	} else
	    rs = mkpath1(tmpfname,fname) ;

#if	CF_DEBUGS
	debugprintf("dialprog: progfname=%s\n",progfname) ;
	debugprintf("dialprog: execfname=%s\n",execfname) ;
#endif

	if (rs >= 0) {
	    MKPROGENV	pe ;
	    if ((rs = mkprogenv_start(&pe,envv)) >= 0) {

	        if ((rs = mkprogenv_envset(&pe,"_EF",progfname,-1)) >= 0) {
		    int		al = -1 ;
	            const char	*ap = NULL ;
		    if (argv != NULL) ap = argv[0] ;
		    if (ap == NULL) al = sfbasename(progfname,-1,&ap) ;
	            if ((rs = mkprogenv_envset(&pe,"_A0",ap,al)) >= 0) {
	        	cchar	**ev ;
		        if ((rs = mkprogenv_getvec(&pe,&ev)) >= 0) {
	                    rs = spawnit(progfname,execfname,of,argv,ev,fd2p) ;
		            fd = rs ;
		        }
		    }
	        } /* end if */

	        rs1 = mkprogenv_finish(&pe) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mkprogenv) */
	    if ((rs < 0) && (fd >= 0)) {
		if (fd2p != NULL) {
		    if (*fd2p >= 0) u_close(*fd2p) ;
		}
		u_close(fd) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("dialprog: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialprog) */


/* local subroutines */


int spawnit(progfname,execfname,of,argv,nenvv,fd2p)
const char	progfname[] ;
const char	execfname[] ;
int		of ;
const char	*argv[] ;
const char	*nenvv[] ;
int		*fd2p ;
{
	int		rs = SR_OK ;
	int		i, j ;
	int		pout[2] ;
	int		perr[2] ;
	const char	**nargv ;

/* open up the necessary pipes */

	pout[0] = -1 ;
	pout[1] = -1 ;
	rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,pout) ;
	if (rs < 0) goto badpipe ;

	if ((rs >= 0) && (of & O_NDELAY)) rs = uc_ndelay(pout[0],TRUE) ;
	if ((rs >= 0) && (of & O_NONBLOCK)) rs = uc_nonblock(pout[0],TRUE) ;

	if (rs < 0) goto badpipe ;

	perr[0] = -1 ;
	perr[1] = -1 ;
	if (fd2p != NULL) {
	    rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,perr) ;
	    if (rs < 0) {
	        for (j = 0 ; j < 2 ; j += 1)
	            u_close(pout[j]) ;
	        goto badpipe ;
	    }
	} /* end if (caller wanted standard-error) */

#ifdef	COMMENT
	if ((rs >= 0) && (of & O_NDELAY)) rs = uc_ndelay(perr[0],TRUE) ;
	if ((rs >= 0) && (of & O_NONBLOCK)) rs = uc_nonblock(perr[0],TRUE) ;
	if (rs < 0) goto badpipe ;
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("dialprog: got pipes \n") ;
#endif

/* we fork */

#if	CF_DEBUGS
	debugprintf("dialprog: about to fork\n") ;
#endif

	if ((rs = uc_fork()) == 0) {
	    int		fd_out = -1 ;
	    int		fd_err = -1 ;
	    const char	*arg[2] ;
	    const char	*cp ;

	    if (of & O_NOCTTY) {
		ignoresigs(sigignores) ;
		setsid() ;
	    }

	    u_close(pout[0]) ;

	    if (perr[0] >= 0)
	        u_close(perr[0]) ;

	    for (i = 3 ; i < NOFILE ; i += 1) {
		if ((i != pout[1]) && (i != perr[1]))
	            u_close(i) ;
	    }

	    fd_out = (pout[1] >= 3) ? pout[1] : uc_moveup(pout[1],3) ;

	    if (perr[1] >= 0)
	        fd_err = (perr[1] >= 3) ? perr[1] : uc_moveup(perr[1],3) ;

	    for (i = 0 ; i < 3 ; i += 1)
		u_close(i) ;

	    u_dup(fd_out) ;			/* standard-input */

	    u_dup(fd_out) ;			/* standard-output */

	    if (fd_err >= 0) {
		u_dup(fd_err) ;		/* standard-error */
	        u_close(fd_err) ;
	    } else
		u_open(NULLFNAME,O_WRONLY,0666) ;

	    u_close(fd_out) ;

/* arguments */

	    nargv = argv ;
	    if (argv == NULL) {
	        sfbasename(progfname,-1,&cp) ;
	        arg[0] = cp ;
	        arg[1] = NULL ;
	        nargv = arg ;
	    } /* end if (argument check) */

/* do the exec */

#if	CF_DEBUGS
	    nprintf(DEBFNAME,"dialprog: u_execve() \n") ;
	    nprintf(DEBFNAME,"dialprog: execfname=%s\n",execfname) ;
	    for (i = 0 ; nargv[i] != NULL ; i += 1)
	        nprintf(DEBFNAME,"dialprog: av[%u]=>%s<\n",i,nargv[i]) ;
	    for (i = 0 ; nenvv[i] != NULL ; i += 1)
	        nprintf(DEBFNAME,"dialprog: ev[%u]=>%s<\n",i,nenvv[i]) ;
#endif /* CF_DEBUGS */

	    {
	    const char	**av = (const char **) nargv ;
	    const char	**ev = (const char **) nenvv ;
	    rs = u_execve(execfname,av,ev) ;
	    }

#if	CF_DEBUGS
	    nprintf(DEBFNAME,"dialprog: u_execve() rs=%d\n",rs) ;
#endif

	    uc_exit(EX_NOEXEC) ;

	} else if (rs < 0)
	    goto badfork ;

#if	CF_DEBUGS
	debugprintf("dialprog: main line continue\n") ;
#endif

/* close some pipe ends */

	u_close(pout[1]) ;

	if (perr[1] >= 0)
	    u_close(perr[1]) ;

	if (fd2p != NULL)
	    *fd2p = perr[0] ;

ret0:
	return (rs >= 0) ? pout[0] : rs ;

/* bad stuff */
badfork:
	for (i = 0 ; i < 2 ; i += 1) {
	    if (pout[i] >= 0)
	        u_close(pout[i]) ;
	}

	if (fd2p != NULL) {
	    for (i = 0 ; i < 2 ; i += 1) {
	        if (perr[i] >= 0)
	            u_close(perr[i]) ;
	    }
	}

badpipe:
	goto ret0 ;
}
/* end subroutine (spawnit) */


