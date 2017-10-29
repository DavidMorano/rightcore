/* spawnproc */

/* spawn a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGENV	0		/* debug environment */
#define	CF_ISAEXEC	0		/* use Solaris® |isaexec(3c)| */
#define	CF_ENVSORT	0		/* sort the environment? */
#define	CF_UCFORK	1		/* use |uc_fork(3uc)| */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Spawn a process while specifying some options for its start-up.

	Synopsis:

	int spawnproc(psap,fname,argv,envv)
	SPAWNPROC	*psap ;
	const char	fname[] ;
	const char	*argv[] ;
	const char	*envv[] ;

	Arguments:

	psap		pointer to optional file descriptor dispositions
	fname		program to execute
	argv		arguments to program
	envv		environment to program

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error

	Implementation notes:  

	Remember that the |pipe(2)| system call creates two pipe file
	descriptors.  Both of these file descriptors are open for reading and
	writing on System V UNIX®.  However, on BSD systems (or older BSD
	systems assuming that they have not yet upgraded to the correct System
	V behavior) the first file descriptor, the one in the zeroth array
	element, is only open for reading.  The second file descriptor, the one
	in the oneth array element, is only open for writing.  We will follow
	the BSD semantics for portability but hopefully, someday, the BSD
	version of UNIX® will get up to speed with the rest of the world!

	Also, note that since we are (very) likely to be running in a (hotly)
	mutli-threaded environment, we have to be quite sure that we try to
	only call async-signal-safe (really fork-safe) subroutines after the
	|fork(2)| and before any |exit(2)|.  The thing to avoid that might be
	used by "accident" is a hidden |malloc(3c)| (or friends) after the
	|fork(2)| someplace.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<envhelp.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<sigign.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"spawnproc.h"


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	NENV		100

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + 40)
#endif

#ifndef	PATHBUFLEN
#define	PATHBUFLEN	(8 * MAXPATHLEN)
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#define	DEFPATH		"/usr/xpg4/bin:/usr/bin:/usr/extra/bin"


/* external subroutines */

extern int	snshellunder(char *,int,pid_t,cchar *) ;
extern int	sncpy1(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	dupup(int,int) ;
extern int	sigdefaults(const int *) ;
extern int	sigignores(const int *) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	vecstr_addpath(vecstr *,cchar *,int) ;
extern int	vecstr_addcspath(vecstr *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */

extern cchar	**environ ;


/* forward reference */

static int	spawnproc_pipes(SPAWNPROC *,cchar *,cchar **,cchar **) ;
static int	spawnproc_parfin(SPAWNPROC *,int,int *,int (*)[2]) ;

static void	spawnproc_child(SPAWNPROC *,cchar *,cchar **,cchar **,
			int,int *,int (*)[2]) ;

static int	envhelp_load(ENVHELP *,char *,cchar *,cchar **) ;

static int	findprog(char *,char *,cchar *) ;
static int	findxfile(IDS *,char *,const char *) ;
static int	ourfork() ;
static int	opendevnull(int *,int) ;

#if	CF_DEBUGS
static int showdev(int) ;
#endif


/* local variables */

static const char	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"SECONDS",
	NULL
} ;

static const int	sigigns[] = {
	SIGTERM,
	SIGINT,
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigdefs[] = {
	SIGQUIT,
	SIGTERM,
	SIGINT,
	SIGPOLL,
	0
} ;

static const int	sigouts[] = {
	SIGTTOU,
	0
} ;


/* exported subroutines */


int spawnproc(SPAWNPROC *psap,cchar *fname,cchar **argv,cchar **envv)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pid = 0 ;
	cchar		*efname = fname ;
	char		pwd[MAXPATHLEN + 1] = { 0 } ;
	char		pbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("spawnproc: ent fname=%s\n",fname) ;
	if (argv != NULL) {
	    int	i ;
	    for (i = 0 ; argv[i] != NULL ; i += 1) {
	        debugprintf("spawnproc: argv[%u]=%s\n",i,argv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGENV
	if (envv != NULL) {
	    int	i ;
	    for (i = 0 ; envv[i] != NULL ; i += 1) {
	        debugprintf("spawnproc: envv%03u=%t\n",i,
	            envv[i],strlinelen(envv[i],-1,55)) ;
	    }
	}
#endif /* CF_DEBUGS */

/* check for bad input */

	if (psap == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("spawnproc: fname=%s\n",fname) ;
#endif

/* find the program */

	if (fname[0] != '/') {
	    if ((rs = findprog(pwd,pbuf,fname)) > 0) {
	        efname = pbuf ;
	    }
	} else {
	    rs = perm(fname,-1,-1,NULL,X_OK) ;
	}

#if	CF_DEBUGS
	debugprintf("spawnproc: mid1 rs=%d efname=%s\n",rs,efname) ;
#endif

	if (rs >= 0) {
	    ENVHELP	e, *ehp = &e ;
	    if ((rs = envhelp_start(ehp,envbads,envv)) >= 0) {
	        if ((rs = envhelp_load(ehp,pwd,efname,argv)) >= 0) {
	            cchar	**ev ;
	            if ((rs = envhelp_getvec(ehp,&ev)) >= 0) {
	                rs = spawnproc_pipes(psap,efname,argv,ev) ;
	                pid = rs ;
	            }
	        } /* end if (envhelp_load) */
	        rs1 = envhelp_finish(ehp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (envhelp_start) */
	} /* end if (ok) */

	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (spawnproc) */


/* local subroutines */


static int spawnproc_pipes(SPAWNPROC *psap,cchar *fname,
		cchar **argv,cchar **ev)
{
	int		rs ;
	int		i ;
	int		pid = 0 ;
	int		con[2] ;
	int		dupes[3] ;
	int		pipes[3][2] ;

#if	CF_DEBUGS && CF_DEBUGENV
	debugprintf("spawnproc: sorted "
	    "vechand_getvec() rs=%d ev=%p\n",
	    rs,ev) ;
	for (i = 0 ; ev[i] != NULL ; i += 1) {
	    debugprintf("spawnproc: ev%03u=>%t<\n",i,
	        ev[i],strlinelen(ev[i],-1,50)) ;
	}
#endif /* CF_DEBUGS */

	for (i = 0 ; i < 3 ; i += 1) {
	    pipes[i][0] = -1 ;
	    pipes[i][1] = -1 ;
	    dupes[i] = -1 ;
	} /* end for */

/* process the file descriptors as specified */

#if	CF_DEBUGS
	debugprintf("spawnproc: file descriptors\n") ;
#endif

	if ((rs = uc_piper(con,3)) >= 0) {
	    const int	pfd = con[0] ;
	    const int	cfd = con[1] ;
	    if ((rs = uc_closeonexec(cfd,TRUE)) >= 0) {

	        for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
#if	CF_DEBUGS
	            debugprintf("spawnproc: checking fd=%u\n",i) ;
#endif
	            switch (psap->disp[i]) {
	            case SPAWNPROC_DINHERIT:
	                break ;
	            case SPAWNPROC_DCREATE:
	                rs = u_pipe(pipes[i]) ;
	                if (rs < 0) {
	                    pipes[i][0] = -1 ;
	                    pipes[i][1] = -1 ;
	                }
	                break ;
	            case SPAWNPROC_DDUP:
	                if ((rs = dupup(psap->fd[i],3)) >= 0) {
	                    dupes[i] = rs ;
	                }
	                break ;
	            } /* end switch */
#if	CF_DEBUGS
	            debugprintf("spawnproc: checked fd=%u rs=%d\n",i,rs) ;
#endif
	        } /* end for */

	        if (rs >= 0) {
	            if ((rs = ourfork()) == 0) { /* child */
	                u_close(pfd) ;
	                spawnproc_child(psap,fname,argv,ev,cfd,dupes,pipes) ;
	            } else if (rs > 0) { /* parent */
	                pid = rs ;
	                if ((rs = u_close(cfd)) >= 0) {
	                    rs = spawnproc_parfin(psap,pfd,dupes,pipes) ;
	                }
	            } /* end if (ourfork) */
	        } /* end if (ok) */

	        if (rs < 0) { /* error */
	            for (i = 0 ; i < 3 ; i += 1) {
	                int	j ;
	                for (j = 0 ; j < 2 ; j += 1) {
	                    if (pipes[i][j] >= 0) u_close(pipes[i][j]) ;
	                }
	                if (dupes[i] >= 0) u_close(dupes[i]) ;
	                psap->fd[i] = -1 ;
	            } /* end for */
	        } /* end if (error) */

	    } /* end if uc_closeonexec) */
	    u_close(pfd) ; /* parent-file-descriptor */
	} /* end if (uc_pipes) */

	return (rs >= 0) ? pid : rs ;
}
/* end subroutine (spawnproc_pipes) */


static int spawnproc_parfin(SPAWNPROC *psap,int pfd,int *dupes,int (*pipes)[2])
{
	int		rs = SR_OK ;
	int		i ;
	int		w ;
	for (i = 0 ; i < 3 ; i += 1) {
	    switch (psap->disp[i]) {
	    case SPAWNPROC_DCREATE:
	        w = (i != 0) ? 1 : 0 ;
	        u_close(pipes[i][w]) ;
	        pipes[i][w] = -1 ;
	        w = (i == 0) ? 1 : 0 ;
	        psap->fd[i] = pipes[i][w] ;
	        break ;
	    default:
	        psap->fd[i] = -1 ;
	        break ;
	    } /* end switch */
	} /* end for */
	for (i = 0 ; i < 3 ; i += 1) {
	    if (dupes[i] >= 0) {
	        u_close(dupes[i]) ;
	        dupes[i] = -1 ;
	    }
	} /* end for */
	if (rs >= 0) {
	    int	res ;
	    if ((rs = u_read(pfd,&res,sizeof(int))) > 0) {
	        rs = res ;
	    }
	} /* end if (ok) */
	return rs ;
}
/* end subroutine (spawnproc_parfin) */


static void spawnproc_child(SPAWNPROC *psap,cchar *fname,
		cchar **argv,cchar **ev,int cfd,int *dupes,int (*pipes)[2])
{
	int		rs = SR_OK ;
	int		i ;
	int		opens[3] ;
	cchar		**av ;
	cchar		*arg[2] ;

#if	CF_DEBUGS
	debugprintf("spawnproc: inside fork\n") ;
	showdev(pipes[1][1]) ;
#endif

	for (i = 0 ; i < 3 ; i += 1) {
	    opens[i] = -1 ;
	}

	if (psap->opts & SPAWNPROC_OIGNINTR) {
	    sigignores(sigigns) ;
	}

	if (psap->opts & SPAWNPROC_OSETSID) {
	    setsid() ;
	} else if (psap->opts & SPAWNPROC_OSETPGRP) {
	    setpgid(0,psap->pgrp) ;
	}

	if (psap->opts & SPAWNPROC_OSETCTTY) {
	    SIGIGN	si ;
	    pid_t	pgrp = getpgrp() ;
	    if ((rs = sigign_start(&si,sigouts)) >= 0) {
	        rs = uc_tcsetpgrp(psap->fd_ctty,pgrp) ;
	        sigign_finish(&si) ;
	    } /* end if (sigign) */
	} /* end if (set PGID for controlling terminal) */

	if (psap->opts & SPAWNPROC_OSIGDEFS) {
	    sigdefaults(sigdefs) ;
	}

	if (rs >= 0) {
	    int	w ;
	    for (i = 0 ; i < 3 ; i += 1) {
#if	CF_DEBUGS
		debugprintf("spawnproc/child: tfd=%u w=%u\n",i,psap->disp[i]) ;
#endif
	        switch (psap->disp[i]) {
	        case SPAWNPROC_DINHERIT:
	            rs = opendevnull(opens,i) ;
	            break ;
	        case SPAWNPROC_DDUP:
		    u_close(i) ;
	            u_dup2(dupes[i],i) ;
	            u_close(dupes[i]) ;
	            break ;
	        case SPAWNPROC_DCREATE:
	            {
	                w = (i == 0) ? 1 : 0 ;
	                u_close(pipes[i][w]) ;
	            }
	            {
	                w = (i != 0) ? 1 : 0 ;
			u_close(i) ;
	                u_dup2(pipes[i][w],i) ;
	                u_close(pipes[i][w]) ;
	            }
	            break ;
	        case SPAWNPROC_DCLOSE:
	        default:
	            break ;
	        case SPAWNPROC_DNULL:
	            u_close(i) ; /* may fail (already closed) */
	            rs = opendevnull(opens,i) ;
#if	CF_DEBUGS
		    debugprintf("spawnproc: opendevnull() rs=%d\n",rs) ;
#endif
	            break ;
	        } /* end switch */
	        if (rs < 0) break ;
	    } /* end for */
	    for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
	        switch (psap->disp[i]) {
	        case SPAWNPROC_DCLOSE:
	            u_close(i) ; /* may fail (already closed) */
	            break ;
	        } /* end switch */
	    } /* end for */
	} /* end if (disposition) */

#if	CF_DEBUGS
	debugprintf("spawnproc: mid2\n") ;
	showdev(0) ;
	showdev(1) ;
	showdev(2) ;
#endif

	av = argv ;
	if ((rs >= 0) && (argv == NULL)) {
	    cchar	*cp ;
	    if (sfbasename(fname,-1,&cp) > 0) {
	        arg[0] = cp ;
	        arg[1] = NULL ;
	        av = arg ;
	    } else {
	        rs = SR_NOENT ;
	    }
	} /* end if (argument check) */

/* do the exec */

#if	CF_DEBUGS
	debugprintf("spawnproc: efname=>%s<\n",fname) ;
#if	CF_DEBUGENV
	for (i = 0 ; av[i] != NULL ; i += 1) {
	    debugprintf("spawnproc: av[%d]=>%s<\n",i,av[i]) ;
	}
	debugprintf("spawnproc: ev=%p\n",ev) ;
	for (i = 0 ; ev[i] != NULL ; i += 1) {
	    debugprintf("spawnproc: ev%03u=>%t<\n",i,
	        ev[i],strlinelen(ev[i],-1,50)) ;
	}
#endif /* CF_DEBUGENV */
#endif /* CF_DEBUGS */

	if ((rs >= 0) && (psap->nice > 0)) {
	    rs = u_nice(psap->nice) ;
	}

	if (rs >= 0) {

#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    rs = uc_isaexecve(fname,av,ev) ;
#else
	    rs = uc_execve(fname,av,ev) ;
#endif

#if	CF_DEBUGS
	    debugprintf("spawnproc: u_execve() rs=%d\n",rs) ;
#endif

	    u_write(cfd,&rs,sizeof(int)) ;
	} /* end if (exec) */

	uc_exit(EX_NOEXEC) ;
}
/* end subroutine (spawnproc_child) */


static int envhelp_load(ENVHELP *ehp,char *pwd,cchar *efname,cchar **argv)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("spawnproc/envhelp_load: ent efn=%s\n",efname) ;
#endif

	if ((rs = envhelp_envset(ehp,"_EF",efname,-1)) >= 0) {
	    int		al = -1 ;
	    cchar	*ap = NULL ;
	    if (argv != NULL) ap = argv[0] ;
	    if (ap == NULL) al = sfbasename(efname,-1,&ap) ;
	    if ((rs = envhelp_envset(ehp,"_A0",ap,al)) >= 0) {
		const int	sulen = (strlen(efname)+22) ;
		char		*subuf ;
		if ((rs = uc_malloc((sulen+1),&subuf)) >= 0) {
	    	    const pid_t	pid = ugetpid() ;
#if	CF_DEBUGS
		    debugprintf("spawnproc/envhelp_load: pid=%u\n",pid) ;
#endif
	    	    if ((rs = snshellunder(subuf,sulen,pid,efname)) > 0) {
#if	CF_DEBUGS
		    debugprintf("spawnproc/envhelp_load: su=%s\n",subuf) ;
#endif
	       		rs = envhelp_envset(ehp,"_",subuf,rs) ;
	    	    }
	    	    uc_free(subuf) ;
	        } /* end if (m-a-f) */
	    } /* end if (envhelp_envset) */
	} /* end if (envhelp_envset) */

#if	CF_DEBUGS
	debugprintf("spawnproc/envhelp_load: mid1 rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    cchar	*var = VARPWD ;
	    if ((rs = envhelp_present(ehp,var,-1,NULL)) == rsn) {
	        int	pwdl = -1 ;
	        rs = SR_OK ;
	        if (pwd[0] == '\0') {
	            rs = getpwd(pwd,MAXPATHLEN) ;
	            pwdl = rs ;
	        }
	        if (rs >= 0) {
	            rs = envhelp_envset(ehp,var,pwd,pwdl) ;
	        }
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("spawnproc/envhelp_load: mid2 rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    cchar	*var = VARPATH ;
	    if ((rs = envhelp_present(ehp,var,-1,NULL)) == rsn) {
	        const int	plen = (2*MAXPATHLEN) ;
	        char		*pbuf ;
	        if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	            if ((rs = uc_confstr(_CS_PATH,pbuf,plen)) >= 0) {
	                rs = envhelp_envset(ehp,var,pbuf,rs) ;
	            } /* end if */
	            uc_free(pbuf) ;
	        } /* end if (m-a-f) */
	    } /* end if (envhelp_present) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("spawnproc/envhelp_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envhelp_load) */


static int findprog(char *pwd,char *pbuf,cchar *fname)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;
	if ((rs = ids_load(&id)) >= 0) {
	    if (strchr(fname,'/') != NULL) {
	        if (pwd[0] == '\0') {
	            rs = getpwd(pwd,MAXPATHLEN) ;
	        }
	        if (rs >= 0) {
	            if ((rs = mkpath2(pbuf,pwd,fname)) >= 0) {
	                USTAT	sb ;
	                pl = rs ;
	                if ((rs = uc_stat(pbuf,&sb)) >= 0) {
	                    const int	am = X_OK ;
	                    rs = sperm(&id,&sb,am) ;
	                }
	            } /* end if (mkpath) */
	        } /* end if (ok) */
	    } else {
	        rs = findxfile(&id,pbuf,fname) ;
	        pl = rs ;
	    } /* end if */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprog) */


static int findxfile(IDS *idp,char *rbuf,cchar *pn)
{
	VECSTR		plist ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

#if	CF_DEBUGS
	debugprintf("spawnproc/findxfile: ent pn=%s\n",pn) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = vecstr_start(&plist,40,0)) >= 0) {
	    cchar	*path = getenv(VARPATH) ;

	    if ((path != NULL) && (path[0] != '\0')) {
	        rs = vecstr_addpath(&plist,path,-1) ;
	    } else {
	        rs = vecstr_addcspath(&plist) ;
	    }

	    if (rs >= 0) {
	        rs = getprogpath(idp,&plist,rbuf,pn,-1) ;
	        pl = rs ;
	    }

	    rs1 = vecstr_finish(&plist) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUGS
	debugprintf("spawnproc/findxfile: ret rs=%d pl=%u\n",rs,pl) ;
	debugprintf("spawnproc/findxfile: ret rbuf=%s\n",rbuf) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findxfile) */


static int ourfork()
{
	int		rs ;

#if	CF_UCFORK
	rs = uc_fork() ;
#else
	rs = u_fork() ;
#endif /* CF_UCFORK */

	return rs ;
}
/* end subroutine (ourfork) */


static int opendevnull(int *opens,int i)
{
	struct ustat	sb ;
	const mode_t	om = 0666 ;
	const int	of = (i == 0) ? O_RDONLY : O_WRONLY ;
	int		rs = SR_OK ;
	if (u_fstat(i,&sb) < 0) {
	    if ((rs = u_open(NULLFNAME,of,om)) >= 0) {
	        const int	fd = rs ;
	        if (fd != i) {
	            if ((rs = u_dup2(fd,i)) >= 0) {
	                opens[i] = rs ;
	                u_close(fd) ;
	            } /* end if (dup2) */
	        } else {
	            opens[i] = i ;
	        }
	        if (rs < 0) {
	            u_close(fd) ;
	        }
	    } /* end if (open) */
	} /* end if (stat) */
	return rs ;
}
/* end subroutine (opendevnull) */


#if	CF_DEBUGS
static int showdev(int fd)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
#ifdef	COMMENT
	debugprintf("spawnproc/showdev: sizeof(st_ino)=%u sizeof(st_dev)=%u\n",
	    sizeof(sb.st_ino),sizeof(sb.st_dev)) ;
#endif
	memset(&sb,0,sizeof(struct ustat)) ;
	if (fd >= 0) rs = u_fstat(fd,&sb) ;
	debugprintf("spawnproc/showdev: fd=%d rs=%d ino=%llu dev=%08x\n",
	    fd,rs,sb.st_ino,sb.st_dev) ;
	return rs ;
}
/* end subroutine (showdev) */
#endif /* CF_DEBUGS */


