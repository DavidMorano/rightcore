/* spawner */

/* spawn a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_ENVSORT	0		/* sort the environment? */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we spawn a process while specifying options for its start-up.

	Synopsis:

	int spawner_start(op,fname,argv,envv)
	SPAWNER		*op ;
	const char	fname[] ;
	const char	*argv[] ;
	const char	*envv[] ;

	Arguments:

	op		pointer to optional file descriptor dispositions
	fname		program to execute
	argv		arguments to program
	envv		environment to program

	Returns:

	>=0		OK
	<0		error

	Implementation notes:  

	Remember that the 'pipe(2)' system call creates two pipe
	file descriptors.  Both of these file descriptors are open for
	reading and writing on System V UNIX®.  However, on BSD systems
	(or older BSD systems assuming that they have not yet upgraded
	to the correct System V behavior) the first file descriptor,
	the one in the zeroth array element, is only open for reading.
	The second file descriptor, the one in the oneth array element,
	is only open for writing.  We will follow the BSD semantics
	for portability but hopefully, someday, the BSD version of
	UNIX® will get up to speed with the rest of the world!

	Also, note that since we are (very) likely to be running in a
	(hotly) mutli-threaded environment, we have to be quite sure
	that we try to only call async-signal-safe (really fork-safe)
	subroutines after the 'fork(2)' and before any 'exit(2)'.
	The thing to avoid that might be used by "accident" is a
	hidden 'malloc(3c)' for friend after the 'fork(2)' someplace.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<sigign.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"spawner.h"


/* local defines */

#define	SCMD		struct spawner_cmd

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

#define	DEFPATH		"/usr/preroot/bin:/usr/xpg4/bin:/usr/bin:/usr/extra/bin"


/* external subroutines */

extern int	sncpy1(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snshellunder(char *,int,pid_t,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getpwd(char *,int) ;
extern int	dupup(int,int) ;
extern int	sigignores(const int *) ;
extern int	sigdefaults(const int *) ;
extern int	findxfile(IDS *,char *,const char *) ;
extern int	xfile(IDS *,const char *) ;
extern int	isIOError(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

enum cmds {
	cmd_setctty,
	cmd_seteuid,
	cmd_setegid,
	cmd_sigign,
	cmd_sigdfl,
	cmd_sighold,
	cmd_sigrlse,
	cmd_fdclose,
	cmd_fdnull,
	cmd_fddup,
	cmd_fddup2,
	cmd_fdcreate,
	cmd_overlast
} ;

struct spawner_cmd {
	uid_t		euid ;
	gid_t		egid ;
	int		cmd ;
	int		pfdend ;
	int		pfd ;		/* "parent" FD */
	int		cfd ;		/* "child" FD */
} ;


/* forward reference */

int		spawner_fddup2(SPAWNER *,int,int) ;

static int	child(SPAWNER *,SCMD **,const char **,const char **) ;
static int	procparent(SCMD **) ;
static int	defaultfds(SCMD **) ;
static int	closefds(SCMD **) ;
static int	isUsed(SCMD **,int) ;
static int	isChildFD(int) ;

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

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_OVERFLOW, EX_SOFTWARE },
	{ SR_DOM, EX_SOFTWARE },
	{ SR_RANGE, EX_SOFTWARE },
	{ SR_NOANODE, EX_SOFTWARE },
	{ SR_NOMEM, EX_OSERR },
	{ SR_PROTO, EX_PROTOCOL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int spawner_start(op,fname,argv,envv)
SPAWNER		*op ;
const char	fname[] ;
const char	*argv[] ;
const char	*envv[] ;
{
	pid_t		pid = ucgetpid() ;
	int		rs = SR_OK ;
	int		pwdlen = -1 ;
	int		size ;
	const char	*execfname ;
	const char	*cp ;
	char		pwd[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("spawner: fname=%s\n",fname) ;
	if (argv != NULL) {
	    int	i ;
	    for (i = 0 ; argv[i] != NULL ; i += 1)
	        debugprintf("spawner: argv[%u]=%s\n",i,argv[i]) ;
	}
	if (envv != NULL) {
	    int	i ;
	    for (i = 0 ; envv[i] != NULL ; i += 1)
		debugprintf("spawner: envv[%u]=%s\n",i,envv[i]) ;
	}
#endif /* CF_DEBUGS */

/* check for bad input */

	if (fname[0] == '\0') return SR_INVAL ;

	memset(op,0,sizeof(SPAWNER)) ;
	op->argv = argv ;
	op->pid = -1 ;

	size = sizeof(SCMD) ;
	rs = vecobj_start(&op->cmds,size,2,0) ;
	if (rs < 0) goto bad0 ;

	rs = envhelp_start(&op->env,envbads,envv) ;
	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	debugprintf("spawner: got in\n") ;
#endif

	pwd[0] = '\0' ;

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
	debugprintf("spawner: execfname=%s\n",execfname) ;
#endif

	rs = uc_mallocstrw(execfname,-1,&cp) ;
	if (rs >= 0) op->execfname = cp ;
	if (rs < 0) goto badnoprog ;

	{
	    const int	sulen = (MAXPATHLEN+22) ;
	    int		sl ;
	    char	subuf[MAXPATHLEN+22+1] ;
	    if ((rs = snshellunder(subuf,sulen,pid,execfname)) > 0) {
	        sl = rs ;
	        rs = envhelp_envset(&op->env,"_",subuf,sl) ;
	    }
	}

/* add the A0 variable */

	if (rs >= 0) {
	    int		al = -1 ;
	    const char	*ap = NULL ;
	    if (argv != NULL) ap = argv[0] ;
	    if (ap == NULL) al = sfbasename(execfname,-1,&ap) ;
#if	CF_DEBUGS
	    debugprintf("spawner: al=%d a=>%t<\n",al,ap,al) ;
#endif
	    rs = envhelp_envset(&op->env,"_A0",ap,al) ;
	} /* end block */

	if (rs >= 0)
	    rs = envhelp_envset(&op->env,"_EF",execfname,-1) ;

#if	CF_DEBUGS
	debugprintf("spawner: def-path\n") ;
#endif

	if (rs >= 0) {
	    const char	*var = VARPWD ;
	    if (envhelp_present(&op->env,var,-1,NULL) == SR_NOTFOUND) {
		if (pwd[0] == '\0') {
		    rs = getpwd(pwd,MAXPATHLEN) ;
		    pwdlen = rs ;
		}
		if (rs >= 0)
	            rs = envhelp_envset(&op->env,var,pwd,pwdlen) ;
	    }
	}

	if (rs >= 0) {
	    const char	*var = VARPATH ;
	    cp = DEFPATH ;
	    if (envhelp_present(&op->env,var,-1,NULL) == SR_NOTFOUND)
	        rs = envhelp_envset(&op->env,var,cp,-1) ;
	}

	if (rs < 0) goto badenvadd ;

ret0:
	return rs ;

/* bad stuff */
badenvadd:
badnoprog:
	envhelp_finish(&op->env) ;

bad1:
	vecobj_finish(&op->cmds) ;

bad0:
	goto ret0 ;
}
/* end subroutine (spawner_start) */


int spawner_finish(op)
SPAWNER		*op ;
{
	SCMD		**cv ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs1 = vecobj_getvec(&op->cmds,&cv)) >= 0) {
	    int	i ;
	    for (i = 0 ; cv[i] != NULL ; i += 1) {
	    	SCMD	*cmdp = cv[i] ;
		switch (cmdp->cmd) {
	        case cmd_fdcreate:
		    if (cmdp->pfd >= 0) u_close(cmdp->pfd) ;
		    break ;
		} /* end switch */
	    } /* end for */
	}
	if (rs >= 0) rs = rs1 ;

	if (op->execfname != NULL) {
	    rs1 = uc_free(op->execfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->execfname = NULL ;
	}

	rs1 = envhelp_finish(&op->env) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->cmds) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (spawner_finish) */


int spawner_setsid(op)
SPAWNER		*op ;
{
	if (op == NULL) return SR_FAULT ;
	if (op->opts & SPAWNER_OSETPGRP) return SR_INVALID ;
	op->opts |= SPAWNER_OSETSID ;
	return SR_OK ;
}
/* end subroutine (spawner_setsid) */


int spawner_setpgrp(op,pgrp)
SPAWNER		*op ;
pid_t		pgrp ;
{
	if (op == NULL) return SR_FAULT ;
	if (pgrp < 0) return SR_INVALID ;
	if (op->opts & SPAWNER_OSETSID) return SR_INVALID ;
	op->opts |= SPAWNER_OSETPGRP ;
	op->pgrp = pgrp ;
	return SR_OK ;
}
/* end subroutine (spawner_setpgrp) */


int spawner_setctty(op,fdterm,pgrp)
SPAWNER		*op ;
int		fdterm ;
pid_t		pgrp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pgrp < 0) return SR_INVALID ;

	op->pgrp = pgrp ;

	if ((rs = dupup(fdterm,3)) >= 0) {
	    SCMD	sc ;
	    int		nfd = rs ;

	    sc.cmd = cmd_setctty ;
	    sc.pfd = nfd ;
	    sc.cfd = -1 ;
	    rs = vecobj_add(&op->cmds,&sc) ;
	    if (rs < 0)
		u_close(nfd) ;

	} /* end if (dupup) */

	return rs ;
}
/* end subroutine (spawner_setctty) */


int spawner_seteuid(SPAWNER *op,uid_t uid)
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_seteuid ;
	sc.pfd = -1 ;
	sc.cfd = -1 ;
	sc.euid = uid ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_seteuid) */


int spawner_setegid(SPAWNER *op,gid_t gid)
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_setegid ;
	sc.pfd = -1 ;
	sc.cfd = -1 ;
	sc.egid = gid ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_setegid) */


int spawner_sigignores(op)
SPAWNER		*op ;
{

	if (op == NULL) return SR_FAULT ;
	op->opts |= SPAWNER_OIGNINTR ;

	return SR_OK ;
}
/* end subroutine (spawner_sigignores) */


int spawner_sigignore(op,sn)
SPAWNER		*op ;
int		sn ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_sigign ;
	sc.pfd = -1 ;
	sc.cfd = sn ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_sigignore) */


int spawner_sigdefault(op,sn)
SPAWNER		*op ;
int		sn ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_sigdfl ;
	sc.pfd = -1 ;
	sc.cfd = sn ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_sigdefault) */


int spawner_sighold(op,sn)
SPAWNER		*op ;
int		sn ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_sighold ;
	sc.pfd = -1 ;
	sc.cfd = sn ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_sighold) */


int spawner_sigrelease(op,sn)
SPAWNER		*op ;
int		sn ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_sigrlse ;
	sc.pfd = -1 ;
	sc.cfd = sn ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_sigrelease) */


int spawner_fdclose(op,cfd)
SPAWNER		*op ;
int		cfd ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_fdclose ;
	sc.pfd = -1 ;
	sc.cfd = cfd ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_fdclose) */


int spawner_fdnull(op,of)
SPAWNER		*op ;
int		of ;
{
	SCMD		sc ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	sc.cmd = cmd_fdnull ;
	sc.pfd = of ;
	sc.cfd = -1 ;
	rs = vecobj_add(&op->cmds,&sc) ;

	return rs ;
}
/* end subroutine (spawner_fdnull) */


int spawner_fddup(op,pfd)
SPAWNER		*op ;
int		pfd ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = dupup(pfd,3)) >= 0) {
	    SCMD	sc ;
	    int		nfd = rs ;

	    sc.cmd = cmd_fddup ;
	    sc.pfd = nfd ;
	    sc.cfd = -1 ;
	    rs = vecobj_add(&op->cmds,&sc) ;
	    if (rs < 0)
		u_close(nfd) ;

	} /* end if (dupup) */

	return rs ;
}
/* end subroutine (spawner_fddup) */


int spawner_fddup2(op,pfd,tfd)
SPAWNER		*op ;
int		pfd ;
int		tfd ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = dupup(pfd,3)) >= 0) {
	    SCMD	sc ;
	    int		nfd = rs ;

	    sc.cmd = cmd_fddup2 ;
	    sc.pfd = nfd ;
	    sc.cfd = tfd ;
	    rs = vecobj_add(&op->cmds,&sc) ;
	    if (rs < 0)
		u_close(nfd) ;

	} /* end if (dupup) */

	return rs ;
}
/* end subroutine (spawner_fddup2) */


int spawner_fddupto(SPAWNER *op,int pfd,int tfd)
{
	return spawner_fddup2(op,pfd,tfd) ;
}
/* end subroutine (spawner_fddupto) */


int spawner_fdcreate(op,cfd)
SPAWNER		*op ;
int		cfd ;
{
	SCMD		sc ;
	int		rs ;
	int		pipes[2] ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = uc_piper(pipes,3)) >= 0) {
	    sc.cmd = cmd_fdcreate ;
	    sc.pfdend = pipes[0] ;
	    sc.pfd = pipes[1] ;
	    sc.cfd = cfd ;
	    rs = vecobj_add(&op->cmds,&sc) ;
	    if (rs < 0) {
		int	i ;
		for (i = 0 ; i < 2 ; i += 1) u_close(pipes[i]) ;
	    }
	} /* end if */

	return (rs >= 0) ? pipes[0] : rs ;
}
/* end subroutine (spawner_fdcreate) */


int spawner_envset(op,kp,vp,vl)
SPAWNER		*op ;
const char	*kp ;
const char	*vp ;
int		vl ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = envhelp_envset(&op->env,kp,vp,vl) ;

	return rs ;
}
/* end subroutine (spawner_envset) */


int spawner_run(op)
SPAWNER		*op ;
{
	int		rs = SR_OK ;
	SCMD		**cv ;
	const char	*args[2] ;
	const char	**av ;
	const char	**ev ;

	if (op == NULL) return SR_FAULT ;

	av = op->argv ;
	if ((av == NULL) || (av[0] == NULL)) {
	    av = args ;
	    args[0] = op->execfname ;
	    args[1] = NULL ;
	}

/* sort all environment variables */

#if	CF_ENVSORT
	if (rs >= 0)
	    rs = envhelp_sort(&op->env) ;
#endif

	if (rs >= 0)
	    rs = envhelp_getvec(&op->env,&ev) ;
	if (rs >= 0)
	    rs = vecobj_getvec(&op->cmds,&cv) ;

	if (rs >= 0) {
	    if ((rs = uc_fork()) == 0) {
		int	ex ;
	        rs = child(op,cv,av,ev) ;
		if (isNotPresent(rs)) {
		    ex = EX_CANTCREAT ;
		} else if (isIOError(rs)) {
		    ex = EX_IOERR ;
		} else
		    ex = mapex(mapexs,rs) ;
		u_exit(ex) ; /* no STDIO flush, no exit-handlers */
	    } /* end if (child) */
	    if (rs >= 0) {
		op->pid = (pid_t) rs ;
		rs = procparent(cv) ;
	    }
	} /* end if */

	if (rs >= 0) rs = op->pid ;

	return rs ;
}
/* end subroutine (spawner_run) */


int spawner_wait(op,csp,opts)
SPAWNER		*op ;
int		*csp ;
int		opts ;
{
	int		rs ;

	if (op->pid >= 0) {
	    rs = u_waitpid(op->pid,csp,opts) ;
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (spawner_wait) */


/* local subroutines */


static int child(op,cv,av,ev)
SPAWNER		*op ;
SCMD		**cv ;
const char	**av ;
const char	**ev ;
{
	const int	opts = op->opts ;
	int		rs = SR_OK ;
	int		i ;
	const char	*execfname = op->execfname ;

	if ((rs >= 0) && (opts & SPAWNER_OIGNINTR))
	    rs = sigignores(sigigns) ;

	if (rs >= 0) {
	    if (opts & SPAWNER_OSETSID) {
	        rs = u_setsid() ;
	    } else if (opts & SPAWNER_OSETPGRP) {
	        rs = u_setpgid(0,op->pgrp) ;
	    }
	}

	for (i = 0 ; (rs >= 0) && (cv[i] != NULL) ; i += 1) {
	    SCMD	*cmdp = cv[i] ;
	    switch (cmdp->cmd) {
	    case cmd_setctty:
		{
		    SIGIGN	si ;
		    pid_t	pgrp = getpgrp() ;
		    if ((rs = sigign_start(&si,sigouts)) >= 0) {
		        rs = uc_tcsetpgrp(cmdp->pfd,pgrp) ;
			u_close(cmdp->pfd) ;
			cmdp->pfd = -1 ;
		        sigign_finish(&si) ;
		    } /* end if (sigign) */
		}
		break ;
	    case cmd_seteuid:
		rs = u_seteuid(cmdp->euid) ;
		break ;
	    case cmd_setegid:
		rs = u_setegid(cmdp->egid) ;
		break ;
	    case cmd_sigign:
		rs = uc_sigignore(cmdp->cfd) ;
		break ;
	    case cmd_sigdfl:
		rs = uc_sigdefault(cmdp->cfd) ;
		break ;
	    case cmd_sighold:
		rs = uc_sighold(cmdp->cfd) ;
		break ;
	    case cmd_sigrlse:
		rs = uc_sigrelease(cmdp->cfd) ;
		break ;
	    case cmd_fdclose:
		rs = u_close(cmdp->cfd) ;
		cmdp->cfd = -1 ;
		break ;
	    case cmd_fdnull:
		{
		    const int	of = cmdp->pfd ;
		    rs = u_open(NULLFNAME,of,0666) ;
		}
		break ;
	    case cmd_fddup:
		rs = u_dup(cmdp->pfd) ;
		u_close(cmdp->pfd) ;
		cmdp->pfd = -1 ;
		break ;
	    case cmd_fddup2:
		if (cmdp->pfd != cmdp->cfd) {
		    rs = u_dup2(cmdp->pfd,cmdp->cfd) ;
		    u_close(cmdp->pfd) ;
		}
		cmdp->pfd = -1 ;
		break ;
	    case cmd_fdcreate:
		if (cmdp->pfd != cmdp->cfd) {
		    rs = u_dup2(cmdp->pfd,cmdp->cfd) ;
		    u_close(cmdp->pfd) ;
		}
		cmdp->pfd = -1 ;
		u_close(cmdp->pfdend) ;
		cmdp->pfdend = -1 ;
		break ;
	    } /* end switch */
	} /* end for */

	if ((rs >= 0) && (op->opts & SPAWNER_OSIGDEFS)) {
	    rs = sigdefaults(sigdefs) ;
	}

	if (rs >= 0)
	   rs = closefds(cv) ;

	if (rs >= 0)
	    rs = defaultfds(cv) ;

	if (rs >= 0)
	   rs = uc_execve(execfname,av,ev) ;

	return rs ;
}
/* end subroutine (child) */


static int defaultfds(SCMD **cv)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
	    if (! isUsed(cv,i)) {
		if (u_fcntl(i,F_GETFD,0) == SR_BADF) {
		    const int of = (i == 0) ? O_RDONLY : O_WRONLY ;
		    rs = u_open(NULLFNAME,of,0666) ;
		}
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (defaultfds) */


static int closefds(SCMD **cv)
{
	int		i ;

	for (i = 3 ; i < NOFILE ; i += 1) {
	    if (! isUsed(cv,i)) u_close(i) ;
	}

	return SR_OK ;
}
/* end subroutine (closefds) */


static int procparent(SCMD **cv)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; cv[i] != NULL ; i += 1) {
	    SCMD	*cmdp = cv[i] ;
	    switch (cmdp->cmd) {
	    case cmd_setctty:
	    case cmd_fdcreate:
	    case cmd_fddup:
	    case cmd_fddup2:
		rs = u_close(cmdp->pfd) ;
		cmdp->pfd = -1 ;
		break ;
	    } /* end switch */
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (procparent) */


static int isUsed(SCMD **cv,int fd)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; cv[i] != NULL ; i += 1) {
	    SCMD	*cmdp = cv[i] ;
	    if (isChildFD(cmdp->cmd)) {
	        f = (cmdp->cfd == fd) ;
	        if (f) break ;
	    }
	} /* end for */

	return f ;
}
/* end subroutine (isUsed) */


static int isChildFD(int cmd)
{
	int		f = FALSE ;

	switch (cmd) {
	case cmd_fddup2:
	case cmd_fdcreate:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isChildFD) */


#if	CF_DEBUGS
static int showdev(fd)
int	fd ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	memset(&sb,0,sizeof(struct ustat)) ;
	if (fd >= 0)
	    rs = u_fstat(fd,&sb) ;
	debugprintf("spawner/showdev: fd=%d rs=%d ino=%u dev=%08x\n",
	    fd,rs,sb.st_ino,sb.st_dev) ;
	return rs ;
}
/* end subroutine (showdev) */
#endif /* CF_DEBUGS */


