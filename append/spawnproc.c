/* spawnproc */

/* spawn a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_ISAEXEC	0		/* use Solaris 'isaexec(3c)'? */
#define	CF_ENVSORT	0		/* sort the environment? */
#define	CF_UCFORK	1		/* use 'uc_fork(3uc)' */


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

	Remember that the 'pipe(2)' system call creates two pipe file
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
	'fork(2)' and before any 'exit(2)'.  The thing to avoid that might be
	used by "accident" is a hidden 'malloc(3c)' for friend after the
	'fork(2)' someplace.


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
#include	<vechand.h>
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

extern int	snshellunder(char *,int,pid_t,const char *) ;
extern int	sncpy1(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	dupup(int,int) ;
extern int	sigdefaults(const int *) ;
extern int	sigignores(const int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */

extern const char	**environ ;


/* forward reference */

static int	loadpath(VECSTR *,const char *) ;
static int	envadd(VECHAND *,VECSTR *,const char *,const char *,int) ;

static int	findxfile(IDS *,char *,const char *) ;

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
	"A__z",
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


int spawnproc(SPAWNPROC *psap,cchar fname[],cchar *argv[],cchar *envv[])
{
	vechand		env ;
	vecstr		tmpstore ;
	pid_t		pid = 0 ; /* ¥ GCC false complaint */
	int		rs = SR_OK ;
	int		pwdlen = -1 ;
	int		i, w ;
	int		opens[3] ;
	int		pipes[3][2] ;
	int		dupes[3] ;
	int		opts ;
	const char	*execfname ;
	const char	**av ;
	const char	**ev ;
	const char	*arg[2] ;
	const char	*cp ;
	char		pwd[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("spawnproc: fname=%s\n", fname) ;
	if (argv != NULL) {
	    for (i = 0 ; argv[i] != NULL ; i += 1) {
	        debugprintf("spawnproc: argv[%u]=%s\n",i,argv[i]) ;
	    }
	}
	if (envv != NULL) {
	    for (i = 0 ; envv[i] != NULL ; i += 1) {
	        debugprintf("spawnproc: envv%03u=%t\n",i,
	            envv[i],strlinelen(envv[i],-1,50)) ;
	    }
	}
#endif /* CF_DEBUGS */

/* check for bad input */

	if (psap == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("spawnproc: got in\n") ;
#endif

	pwd[0] = '\0' ;
	for (i = 0 ; i < 3; i += 1) {
	    opens[i] = -1 ;
	    pipes[i][0] = -1 ;
	    pipes[i][1] = -1 ;
	    dupes[i] = -1 ;
	} /* end for */

/* find the program */

	execfname = fname ;
	if (fname[0] != '/') {

	    if (strchr(fname,'/') != NULL) {

	        if (pwd[0] == '\0') {
	           rs = getpwd(pwd,MAXPATHLEN) ;
		   pwdlen = rs ;
		}

	        if (rs >= 0)
	            rs = mkpath2(tmpfname,pwd,fname) ;

	        if (rs >= 0)
	            rs = perm(tmpfname,-1,-1,NULL,X_OK) ;

	    } else {
	        IDS	id ;

	        if ((rs = ids_load(&id)) >= 0) {

	            rs = findxfile(&id,tmpfname,fname) ;
	            if (rs > 0)
	                execfname = tmpfname ;

	            ids_release(&id) ;
	        } /* end if */

	    } /* end if */

	} else
	    rs = perm(fname,-1,-1,NULL,X_OK) ;

	if (rs < 0)
	    goto badnoprog ;

#if	CF_DEBUGS
	debugprintf("spawnproc: execfname=%s\n",execfname) ;
#endif

	opts = 0 ;
#if	CF_ENVSORT
	opts |= VECSTR_OSORTED ;
#endif
	opts |= VECSTR_OCOMPACT ;
	rs = vechand_start(&env,NENV,opts) ;
	if (rs < 0)
	    goto badenv1 ;

	rs = vecstr_start(&tmpstore,5,0) ;
	if (rs < 0)
	    goto badenv2 ;

/* add the program filename ('envbuf' from above) */

	{
	    pid_t	pid = ucgetpid() ;
	    const int	tlen = MAXPATHLEN ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = snshellunder(tbuf,tlen,pid,execfname)) > 0) {
		rs = envadd(&env,&tmpstore,"_",tbuf,rs) ;
	    }
	}

/* add the A0 variable */

	if (rs >= 0) {
	    int		zl = -1 ;
	    const char	*zp = NULL ;
	    if (argv != NULL) zp = argv[0] ;
	    if (zp == NULL) {
		zl = sfbasename(execfname,-1,&zp) ;
	    }
	    rs = envadd(&env,&tmpstore,"_A0",zp,zl) ;
	} /* end block */

	if (rs >= 0) {
	    rs = envadd(&env,&tmpstore,"_EF",execfname,-1) ;
	}

/* add all other specified variables */

#if	CF_DEBUGS
	debugprintf("spawnproc: other variables\n") ;
#endif

	if (rs >= 0) {
	    const char	**evv = envv ;
	    const void	*vp ;

#if	CF_DEBUGS
	    debugprintf("spawnproc: environ(%p)\n",environ) ;
	    debugprintf("spawnproc: evv(%p)\n",evv) ;
#endif

	    evv = envv ;
	    if (evv == NULL) {
	        if (environ == NULL) rs = SR_FAULT ;
	        evv = environ ;
	    }

	    for (i = 0 ; (rs >= 0) && (evv[i] != NULL) ; i += 1) {

	        if (matkeystr(envbads,evv[i],-1) < 0) {
#if	CF_DEBUGS
	            debugprintf("spawnproc: adding evv%03u=>%t<\n",i,
	                evv[i],strlinelen(evv[i],-1,50)) ;
#endif

	            vp = (const void *) evv[i] ;
	            rs = vechand_add(&env,vp) ;
	        }

	    } /* end for */

	} /* end block */

/* add other things that may not already be there */

#if	CF_DEBUGS
	debugprintf("spawnproc: def-path\n") ;
#endif

	if (rs >= 0) {
	    const char	*var = VARPWD ;
	    if (vechand_search(&env,var,vstrkeycmp,NULL) == SR_NOTFOUND) {
		if (pwd[0] == '\0') {
		    rs = getpwd(pwd,MAXPATHLEN) ;
		    pwdlen = rs ;
		}
		if (rs >= 0)
	            rs = envadd(&env,&tmpstore,var,pwd,pwdlen) ;
	    }
	}

	if (rs >= 0) {
	    const char	*var = VARPATH ;
	    cp = DEFPATH ;
	    if (vechand_search(&env,var,vstrkeycmp,NULL) == SR_NOTFOUND) {
	        rs = envadd(&env,&tmpstore,var,cp,-1) ;
	    }
	}

/* sort all environment variables */

#if	CF_ENVSORT
	if (rs >= 0)
	    vechand_sort(&env,vstrkeycmp) ;
#endif

/* prepapre to spawn the program */

#if	CF_DEBUGS
	debugprintf("spawnproc: prepare to spawn\n") ;
#endif

	if (rs >= 0)
	    rs = vechand_getvec(&env,&ev) ;

#if	CF_DEBUGS
	debugprintf("spawnproc: sorted vechand_getvec() rs=%d ev=%p\n",rs,ev) ;
	for (i = 0 ; ev[i] != NULL ; i += 1)
	    debugprintf("spawnproc: ev%03u=>%t<\n",i,
	        ev[i],strlinelen(ev[i],-1,50)) ;
#endif /* CF_DEBUGS */

/* process the file descriptors as specified */

	if (rs >= 0) {
	    struct ustat	sb ;
	    int		oflags ;

#if	CF_DEBUGS
	    debugprintf("spawnproc: file descriptors\n") ;
#endif

	    for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
	        if (u_fstat(i,&sb) < 0) {
	            oflags = (i == 0) ? O_RDONLY : O_WRONLY ;
	            rs = u_open(NULLFNAME,oflags,0600) ;
	            if (rs >= 0) opens[i] = i ;
	        } /* end if (file wasn't open) */
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("spawnproc: preopen rs=%d\n",rs) ;
#endif

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
	            rs = dupup(psap->fd[i],3) ;
	            if (rs >= 0) dupes[i] = rs ;
	            break ;

	        } /* end switch */

#if	CF_DEBUGS
	        debugprintf("spawnproc: checked fd=%u rs=%d\n",i,rs) ;
#endif

	    } /* end for */

	} /* end if (disposition) */

#if	CF_DEBUGS
	debugprintf("spawnproc: disposition rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badopen ;

/* we fork */

#if	CF_DEBUGS
	debugprintf("spawnproc: uc_fork() \n") ;
#endif

#if	CF_UCFORK
	rs = uc_fork() ;
#else
	rs = u_fork() ;
#endif /* CF_UCFORK */

	pid = rs ;
	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("spawnproc: inside fork\n") ;
	    showdev(pipes[1][1]) ;
#endif

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
	        for (i = 0 ; i < 3 ; i += 1) {
	            switch (psap->disp[i]) {
	            case SPAWNPROC_DINHERIT:
	                break ;
	            case SPAWNPROC_DDUP:
	                u_dup2(dupes[i],i) ;
	                u_close(dupes[i]) ;
	                break ;
	            case SPAWNPROC_DCREATE:
	                w = (i == 0) ? 1 : 0 ;
	                u_close(pipes[i][w]) ;
	                w = (i != 0) ? 1 : 0 ;
	                u_dup2(pipes[i][w],i) ;
	                u_close(pipes[i][w]) ;
	                break ;
	            case SPAWNPROC_DCLOSE:
	            default:
	                u_close(i) ;
	                break ;
	            } /* end switch */
	        } /* end for */
	    } /* end if (disposition) */

#if	CF_DEBUGS
	    showdev(0) ;
	    showdev(1) ;
#endif

	    av = argv ;
	    if ((rs >= 0) && (argv == NULL)) {
	        if (sfbasename(execfname,-1,&cp) > 0) {
	            arg[0] = cp ;
	            arg[1] = NULL ;
	            av = arg ;
		} else
		    rs = SR_NOENT ;
	    } /* end if (argument check) */

/* do the exec */

#if	CF_DEBUGS
	    debugprintf("spawnproc: u_execve() \n") ;
	    debugprintf("spawnproc: execfname=>%s<\n",execfname) ;
	    for (i = 0 ; av[i] != NULL ; i += 1)
	        debugprintf("spawnproc: av[%d]=>%s<\n",i,av[i]) ;
	    debugprintf("spawnproc: ev=%p\n",ev) ;
	    for (i = 0 ; ev[i] != NULL ; i += 1)
	        debugprintf("spawnproc: ev%03u=>%t<\n",i,
	            ev[i],strlinelen(ev[i],-1,50)) ;
#endif /* CF_DEBUGS */

	    if ((rs >= 0) && (psap->nice > 0)) {
	        rs = u_nice(psap->nice) ;
	    }

	    if (rs >= 0) {

#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	        rs = uc_isaexecve(execfname,av,ev) ;
#else
	        rs = uc_execve(execfname,av,ev) ;
#endif

#if	CF_DEBUGS
	        debugprintf("spawnproc: u_execve() rs=%d\n",rs) ;
#endif

	    } /* end if (exec) */

	    uc_exit(EX_NOEXEC) ;
	} else if (rs < 0)
	    goto badfork ;

#if	CF_DEBUGS
	debugprintf("spawnproc: pid_child=%u\n",pid) ;
#endif

/* close some pipe ends */

	if (psap != NULL) {
	    for (i = 0 ; i < 3 ; i += 1) {
	        switch (psap->disp[i]) {
	        case SPAWNPROC_DCREATE:
	            w = (i != 0) ? 1 : 0 ;
	            u_close(pipes[i][w]) ;
	            w = (i == 0) ? 1 : 0 ;
	            psap->fd[i] = pipes[i][w] ;
	            break ;
	        default:
	            psap->fd[i] = -1 ;
	            break ;
	        } /* end switch */
	    } /* end for */
	} /* end if (disposition) */

/* close whatever */
ret3:
	for (i = 0 ; i < 3 ; i += 1) {
	    if (opens[i] >= 0) u_close(i) ;
	    if (dupes[i] >= 0) u_close(dupes[i]) ;
	} /* end for */

	vecstr_finish(&tmpstore) ;

	vechand_finish(&env) ;

ret0:
	if (rs >= 0) rs = pid ;
	return rs ;

/* bad stuff */
badfork:
badopen:
	if (psap != NULL) {
	    for (i = 0 ; i < 3 ; i += 1) {
	        if (pipes[i][0] >= 0) {
	            u_close(pipes[i][0]) ;
	            u_close(pipes[i][1]) ;
	        }
	    } /* end for */
	} /* end if (had arguments) */

	if (psap != NULL) {
	    for (i = 0 ; i < 3 ; i += 1)
	        psap->fd[i] = -1 ;
	}

	goto ret3 ;

badenv2:
	vechand_finish(&env) ;

badenv1:
badnoprog:
	if (psap != NULL) {
	    for (i = 0 ; i < 3 ; i += 1)
	        psap->fd[i] = -1 ;
	}

	goto ret0 ;
}
/* end subroutine (spawnproc) */


/* local subroutines */


static int envadd(VECHAND *ehp,VECSTR *tsp,cchar *kp,cchar *ep,int el)
{
	int	rs ;

	if ((rs = vecstr_envadd(tsp,kp,ep,el)) >= 0) {
	    int		i = rs ;
	    const char	*cp ;
	    if ((rs = vecstr_get(tsp,i,&cp)) >= 0)
	        rs = vechand_add(ehp,cp) ;
	}

	return rs ;
}
/* end subroutine (envadd) */


static int findxfile(IDS *idp,char rbuf[],cchar pn[])
{
	VECSTR		pl ;
	int		rs = SR_NOENT ;
	int		rs1 ;
	int		f_pwd = FALSE ;
	const char	*sp = getenv(VARPATH) ;
	char		pathbuf[PATHBUFLEN + 1] ;

	rbuf[0] = '\0' ;

	if ((sp == NULL) || (sp[0] == '\0')) {
	    rs1 = uc_confstr(_CS_PATH,pathbuf,PATHBUFLEN) ;
	    if (rs1 >= 0)
	        sp = pathbuf ;
	}

	if (sp != NULL) {
	    if ((rs = vecstr_start(&pl,40,0)) >= 0) {

		if ((rs = loadpath(&pl,sp)) >= 0) {
		    f_pwd = rs ;
	            rs = getprogpath(idp,&pl,rbuf,pn,-1) ;
		}

	        vecstr_finish(&pl) ;
	    } /* end if (vecstr) */
	} /* end if */

	return rs ;
}
/* end subroutine (findxfile) */


static int loadpath(VECSTR *plp,const char *sp)
{
	int		rs = SR_OK ;
	int		f_pwd = FALSE ;
	const char	*tp ;

	        while ((tp = strchr(sp,':')) != NULL) {
	            if ((tp - sp) == 0) f_pwd = TRUE ;
	            rs = vecstr_adduniq(plp,sp,(tp - sp)) ;
	            sp = (tp + 1) ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (sp[0] != '\0'))
	            rs = vecstr_adduniq(plp,sp,-1) ;

	return (rs >= 0) ? f_pwd : rs ;
}
/* end subroutine (loadpath) */


#if	CF_DEBUGS
static int showdev(int fd)
{
	struct ustat	sb ;
	int	rs = SR_OK ;
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


