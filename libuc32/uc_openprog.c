/* uc_openprog */

/* interface component for UNIX® library-3c */
/* connect to a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ENVLOAD	0		/* |mkprogenv_load()| */


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
#include	<ugetpid.h>
#include	<vecstr.h>
#include	<mkprogenv.h>
#include	<spawnproc.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3w(char *,int,const char *,const char *,const char *,int) ;
extern int	snshellunder(char *,int,pid_t,cchar *) ;
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

#if	CF_ENVLOAD
static int	mkprogenv_load(MKPROGENV *,cchar *,cchar **) ;
#endif

static int	openproger(cchar *,int,cchar **,cchar **,int *) ;
static int	spawnit(cchar *,int,cchar **,cchar **,int *) ;

static int	accmode(int) ;


/* local variables */

enum accmodes {
	accmode_rdonly,
	accmode_wronly,
	accmode_rdwr,
	accmode_overlast
} ;


/* exported subroutines */


int uc_openprogerr(cchar *pfn,int of,cchar **argv,cchar **envv,int *efdp)
{
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("uc_openprogerr: ent pfn=%s\n",pfn) ;
	if (argv != NULL) {
	    int	i ;
	    for (i = 0 ; argv[i] != NULL; i += 1) {
		debugprintf("uc_openprogerr: a[%u]=%s\n",i,argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	if (pfn == NULL) return SR_FAULT ;

	if (pfn[0] == '\0') return SR_INVALID ;

	while ((pfn[0] == '/') && (pfn[1] == '/')) {
	    pfn += 1 ;
	}

/* argument check */

	if ((rs = accmode(of)) >= 0) {
	    char	ebuf[MAXPATHLEN + 1] ;
	    if ((rs = mkepath(ebuf,pfn)) >= 0) {
		if (rs > 0) pfn = ebuf ;
		rs = openproger(pfn,of,argv,envv,efdp) ;
		fd = rs ;
	    } /* end if (mkepath) */
	} /* end if (accmode) */

#if	CF_DEBUGS
	debugprintf("uc_openprogerr: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_openprogerr) */


int uc_openprog(cchar *pfn,int of,cchar **argv,cchar **envv)
{
#if	CF_DEBUGS
	debugprintf("uc_openprog: ent pfn=%s\n",pfn) ;
#endif
	return uc_openprogerr(pfn,of,argv,envv,NULL) ;
}
/* end subroutine (uc_openprog) */


/* local subroutines */


static int mkepath(char *ebuf,cchar *pfn)
{
	int		rs ;
	int		el = 0 ;
#if	CF_DEBUGS
	debugprintf("uc_openprog/mkepath: ent pfn=%s\n",pfn) ;
#endif
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
		const int	rsn = SR_NOENT ;
		if ((rs = perm(pfn,-1,-1,NULL,X_OK)) == rsn) {
		    cchar	*tp ;
		    if ((tp = strchr(pfn,':')) != NULL) {
			if (((tp-pfn) == 3) && (strncmp(pfn,"sys",3) == 0)) {
			    pfn = (tp+1) ;
			}
		    }
		        rs = mkfindpath(ebuf,pfn) ;
		        el = rs ;
		} /* end if (perm) */
	    } /* end if (ok) */
	} /* end if (mkvarpath) */
#if	CF_DEBUGS
	debugprintf("uc_openprog/mkepath: ret rs=%d el=%u\n",rs,el) ;
#endif
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
		    rs = getprogpath(&id,&ps,ebuf,pfn,-1) ;
		    el = rs ;
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


static int openproger(cchar *pfn,int of,cchar **argv,cchar **envv,int *efdp)
{
	MKPROGENV	pe ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("uc_openprog/openproger: ent pfn=%s\n",pfn) ;
#endif
	if ((rs = mkprogenv_start(&pe,envv)) >= 0) {
	    cchar	**ev ;
	    if ((rs = mkprogenv_getvec(&pe,&ev)) >= 0) {
		rs = spawnit(pfn,of,argv,ev,efdp) ;
		fd = rs ;
	    }
	    rs1 = mkprogenv_finish(&pe) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mkprogenv) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;
#if	CF_DEBUGS
	debugprintf("uc_openprog/openproger: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openproger) */


#if	CF_ENVLOAD
static int mkprogenv_load(MKPROGENV *pep,cchar *pfn,cchar **argv)
{
	int		rs ;
	if ((rs = mkprogenv_envset(pep,"_EF",pfn,-1)) >= 0) {
	    int		al = -1 ;
	    cchar	*ap = NULL ;
	    if (argv != NULL) ap = argv[0] ;
	    if (ap == NULL) al = sfbasename(pfn,-1,&ap) ;
	    if ((rs = mkprogenv_envset(pep,"_A0",ap,al)) >= 0) {
		const int	sulen = (strlen(pfn)+22) ;
		char		*subuf ;
		if ((rs = uc_malloc((sulen+1),&subuf)) >= 0) {
	    	    const pid_t	pid = ugetpid() ;
	    	    if ((rs = snshellunder(subuf,sulen,pid,pfn)) > 0) {
	        	rs = mkprogenv_envset(pep,"_",subuf,rs) ;
	    	    }
	    	    uc_free(subuf) ;
		} /* end if (m-a-f) */
	    } /* end if (mkprogenv_envset) */
	} /* end if (mkprogenv_envset) */
	return rs ;
}
/* end subroutine (mkprogenv_load) */
#endif /* CF_ENVLOAD */


int spawnit(cchar *pfn,int of,cchar **argv,cchar **envv,int *fd2p)
{
	int		rs ;
	int		fd = -1 ;
	int		pout[2] ;

#if	CF_DEBUGS
	debugprintf("uc_openprogerr/spawnit: ent pfn=%s\n",pfn) ;
#endif

	if ((rs = uc_piper(pout,3)) >= 0) {
	    if ((of&O_NDELAY) || (of&O_NONBLOCK)) {
		if ((rs >= 0) && (of & O_NDELAY)) {
		    rs = uc_ndelay(pout[0],TRUE) ;
		}
		if ((rs >= 0) && (of & O_NONBLOCK)) {
		    rs = uc_nonblock(pout[0],TRUE) ;
		}
	    } /* end if (options) */
	    if (rs >= 0) {
		SPAWNPROC	ps ;
		memset(&ps,0,sizeof(SPAWNPROC)) ;
		ps.fd[0] = pout[1] ;
		ps.fd[1] = pout[1] ;
		ps.disp[0] = SPAWNPROC_DDUP ;
		ps.disp[1] = SPAWNPROC_DDUP ;
		if (fd2p != NULL) {
		    ps.disp[2] = SPAWNPROC_DCREATE ;
		} else {
		    ps.disp[2] = SPAWNPROC_DNULL ;
		}
	        if (of & O_NOCTTY) {
		    ps.opts |= SPAWNPROC_OIGNINTR ;
		    ps.opts |= SPAWNPROC_OSETSID ;
		}
#if	CF_DEBUGS
		debugprintf("uc_openprogerr/spawnit: spawnproc()\n") ;
#endif
	   	if ((rs = spawnproc(&ps,pfn,argv,envv)) >= 0) {
		    fd = pout[0] ;
		    u_close(pout[1]) ;
		    pout[1] = -1 ;
		    if (fd2p != NULL) {
			*fd2p = ps.fd[2] ;
		    }
		} /* end if (spawnproc) */
#if	CF_DEBUGS
	debugprintf("uc_openprogerr/spawnit: spawnproc-out rs=%d\n",rs) ;
#endif
	    } /* end if (ok) */
	    if (rs < 0) {
		int	i ;
		for (i = 0 ; i < 2 ; i += 1) {
		    if (pout[i] >= 0) {
			u_close(pout[i]) ;
			pout[i] = -1 ;
		    }
		} /* end for */
	    } /* end if (error-cleanup) */
	} /* end if (uc_piper) */

#if	CF_DEBUGS
	debugprintf("uc_openprogerr/spawnit: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (spawnit) */


static int accmode(int oflags)
{
	const int	am = (oflags & O_ACCMODE) ;
	int		rs = SR_INVALID ;
	switch (am) {
	case O_RDONLY:
	    rs = accmode_rdonly ;
	    break ;
	case O_WRONLY:
	    rs = accmode_wronly ;
	    break ;
	case O_RDWR:
	    rs = accmode_rdwr ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (accmode) */


