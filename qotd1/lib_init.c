/* kshlib */

/* library initialization for KSH built-in command libraries */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGENV	1		/* debug environment */
#define	CF_PLUGIN	1		/* define 'plugin_version()' */
#define	CF_LOCKMEMALLOC	1		/* call |lockmemalloc(3uc)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	KSH built-in library support.

	------------------------------------------------------------------------
	Name:

	lib_init

	Description:

	This subroutine is called by KSH when it loads this shared
	library.  We use this call to initialize some things that are
	partuclar to when executing from within KSH.  One of these is
	to set the underlying memory management facility to implement a
	MUTEX lock around its operations.  This helps guard against a
	failure if the KSH-native version of the normal memory
	management subroutines are somehow linked in (loaded) rather
	than the standard default UNIX® System subroutines.

	Synopsis:

	void lib_init(int flags,void *contextp)

	Arguments:

	flags		flags set by KSH
	contexto	KSH context

	Returns:

	-


	------------------------------------------------------------------------
	Name:

	lib_initenviron

	Description:

	This subroutine is used to set the environment inside the
	builtin-command (CMD) link-group to the same environment of our
	caller (usually the SHELL itself).  Depending on how this
	module is loaded, it may be in the link-group of its parent
	(not at all unusual) or it may be in its own link-group.  In
	theory it could even be on its own link-map, but that is not at
	all a typical situation so we ignore that for our purposes.  If
	this modeule is loaded into its own link-group, some means has
	to be provided to set the 'environ' variable (above) to the
	envionment of the calling parent (at least set to something).
	When the builtin commands are called by the SHELL, they (the
	builtin commands) are called directly from it.  But the CMD
	link-group has its own copy of the 'environ' variable which
	would not have yet been set at all by anybody (any subroutine
	anywhere).  So when CMDs are called by the SHELL, the CMD
	subroutine itself calls 'lib_initenviron()' in order to set the
	CMD link-group copy of the 'environ' variable to the same as
	what exists wihtin the SHELL itself.

	When CMDs are not called by the SHELL, but rather by some
	other means, some other way to set the 'environ' variable
	has to be established.  Possible other ways are:

	1) this subroutine is not linked in, so there is *no* separate
	copy of 'environ' in the first place (completely typical in
	regular programs)

	2) by the caller instead calling the CMD subroutine though
	an intermediate subroutine (like named 'lib_caller()') and
	which gets its internal 'environ' copy set with that subroutine
	before the CMD subroutine is called in turn.

	Synopsis:

	int lib_initenviron(void)

	Arguments:

	*none*

	Returns:

	<0	error
	>=0	OK


	------------------------------------------------------------------------
	Name:

	lib_caller

	Description:

	What in the world does this subroutine do? :-)

	This subroutine lets us call a command (CMD) function, otherwise
	known as a command "builtin" (from the SHELL language on the
	subject) while giving it an arbitrary environment determined
	by the caller.	In the infinite (short-sighted) wisdom of the
	creators of the builtin command interface, it was neglected to
	provide the capability to pass an arbitrary environment (like what
	is possible -- but not often used) with regular UNIX® process calls
	(using 'exec(2)' and friends).	Without this subroutine, and
	having to call the command function directory, there is no way
	to pass or to create a unique environment for the function since
	it is forced to simply inherit the environment of the caller.

	Synopsis:

	int lib_caller(func,argc,argv,envv,contextp)
	int		(*func)(int,const char **,void *) ;
	int		argc ;
	const char	*argv[] ;
	const char	*envv[] ;
	void		*contextp ;

	Arguments:

	func		function to call
	argc		ARGC
	argv		ARGV
	envv		ENVV
	contextp	KSH context

	Returns:

	ex		exit status (like a process exit status)


	------------------------------------------------------------------------
	Name:

	lib_initmemalloc

	Description:

	When CF_LOCKMEMALLOC is set (non-zero) above, the LOCKMEMALLOC
	facility is (possibly) made available for use.  Actual use
	depends on whether the module (LOCKMEMALLOC) is available
	somewhere in the current link-map.  If it is indeed available,
	we turn it on (with the proper command).  We do this (turn it
	ON) because the KSH program does not provide mutex locks around
	its memory allocation subroutines (which emulate |malloc(3c)|
	and friends).  Of course KSH does not use any of the standard
	system subroutines because, well, that would be way too easy
	wouldn't it?  The KSH program thinks that it is better than
	everyone else and so it uses its own memory-allocation
	facility.  One problem: it did not protect its own facility
	with mutex locks.  It did not do this because the shell is
	single threaded throughout.  But this causes problems (like
	program crashes) when some dynmically loaded code splits into a
	multi-threaded mode.  Yes, bang, a mess of the underlying
	memory-allocation system and the expected program crash as a
	result.  The use of the LOCKMEMALL facility places mutex locks
	around all calls to the underlying memory allocation subroutines.
	This can help but also might still not be enough (since some
	shell code can still be used even within multi-threaded code).
	But every little bit helps.

	One would think that everything today is multi-thread safe,
	but NO.  There are still some hold-outs, and these hold-outs
	make it bad for everybody!

	Well, there it is.

	Synospsis:

	int lib_initmemalloc(int f)

	Arguments:

	f		switch (0=OFF, 1=ON)

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<dlfcn.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<sigman.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	KSHBUILTIN
#define	KSHBUILTIN	0
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#define	NDEBFNAME	"kshlib.deb"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	TO_LOCKENV	10

#define	RUNMODE_MKSH	(1<<0)
#define	RUNMODE_MMAIN	(1<<1)

#define	KSHLIB		struct kshlib


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_LOCKMEMALLOC
extern int	lockmemalloc_set(int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif


/* external variables */

extern char	**environ ;


/* local structures */

struct kshlib {
	PTM		m ;	/* mutex data */
	PTM		menv ;	/* mutex environment */
	SIGMAN		sm ;
	int		runmode ;
	volatile int	f_init ;
	volatile int	f_initdone ;
	int		f_sigterm ;
	int		f_sigint ;
} ;

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* forward references */

int		lib_initenviron(void) ;
int		lib_callcmd(cchar *,int,cchar **,cchar **,void *) ;

static int	kshlib_init(void) ;
static void	kshlib_fini(void) ;

static void	kshlib_atforkbefore() ;
static void	kshlib_atforkafter() ;
static void	kshlib_sighand(int) ;

int		lib_initmemalloc(int) ;

#if	CF_DEBUGENV && CF_DEBUGN
static int	ndebugenv(const char *,const char **) ;
#endif


/* local variables */

static pthread_mutex_t	lib_envmutex = PTHREAD_MUTEX_INITIALIZER ;

static const char	*defenviron[] = {
	"_PROCSTATE=screwed",
	NULL
} ;

static KSHLIB		kshlib_data ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigigns[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;


/* exported subroutines */


/* ARGSUSED */
void lib_init(int flags,void *contextp)
{

#if	CF_DEBUGN
	{
	    const pid_t	pid = ucgetpid() ;
	    void *p = dlsym(RTLD_DEFAULT,"environ") ;
	    nprintf(NDEBFNAME,"lib_init: ent pid=%u\n",pid) ;
	    nprintf(NDEBFNAME,"lib_init: flags=%16ß (%u)\n",flags,flags) ;
	    if (p != NULL) {
		int	i ;
		const char	***evp = (const char ***) p ;
		const char	**ev ;
		ev = *evp ;
	        nprintf(NDEBFNAME,"lib_init: p=%P\n",p) ;
	        nprintf(NDEBFNAME,"lib_init: main-environ{%P}=%P\n",evp,ev) ;
	        nprintf(NDEBFNAME,"lib_init: lib-environ{%P}=%P\n",
			&environ,environ) ;
	    ndebugenv("lib_init-m",ev) ;
	    }
	    nprintf(NDEBFNAME,"lib_init: lib-environ=%P\n",environ) ;
	    nprintf(NDEBFNAME,"lib_init: contextp=%P\n",contextp) ;
	}
#endif /* CF_DEBUGN */

#if	CF_DEBUGENV && CF_DEBUGN && 0
	if (environ != NULL) {
	    const char	**ev = (const char **) environ ;
	    ndebugenv("lib_init-l",ev) ;
	}
#endif

#if	CF_LOCKMEMALLOC
	{
	    const int	f = KSHBUILTIN ;
	    (void) lib_initmemalloc(f) ;
	}
#endif /* CF_LOCKMEMALLOC */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_init: ret\n") ;
#endif

}
/* end subroutine (lib_init) */


int lib_initenviron()
{
	int	rs = SR_OK ;
#if	CF_DEBUGN
	{
	    const pid_t	pid = ucgetpid() ;
	    nprintf(NDEBFNAME,"lib_initenviron: ent pid=%u\n",pid) ;
	    nprintf(NDEBFNAME,"lib_initenviron: environ=%P\n",environ) ;
	}
#endif
	if (environ == NULL) {
	    if ((rs = ptm_lock(&lib_envmutex)) >= 0) {
	        char ***eppp = dlsym(RTLD_DEFAULT,"environ") ;
	        if ((eppp != NULL) && (eppp != &environ)) environ = *eppp ;
	        if (environ == NULL) environ = (char **) defenviron ;
	        rs = ptm_unlock(&lib_envmutex) ;
	    } /* end if (ptm) */
	} /* end if (environ) */
#if	CF_DEBUGN && 0
	nprintf(NDEBFNAME,"lib_initenviron: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_initenviron) */


/* ARGSUSED */
int lib_kshbegin(void *contextp)
{
	int	rs = SR_OK ;
	if (contextp != NULL) rs = lib_initenviron() ;
	if (rs >= 0) {
	    if ((rs = kshlib_init()) >= 0) {
	        KSHLIB	*klp = &kshlib_data ;
	        void	(*sh)(int) = kshlib_sighand ;
		kip->runmode |= RUNMODE_MKSH ;
	        klp->f_sigterm = 0 ;
	        klp->f_sigint = 0 ;
	        rs = sigman_start(&klp->sm,sigblocks,sigigns,sigints,sh) ;
	    } /* end if (kshlib_init) */
	} /* end if (ok) */
	return rs ;
} 
/* end subroutine (lib_kshbegin) */


int lib_kshend()
{
	KSHLIB		*klp = &kshlib_data ;
	int		rs ;
	kip->runmode &= (~ RUNMODE_MKSH) ;
	rs1 = sigman_finish(&klp->sm) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (lib_kshend) */


int lib_kshlib_runmode()
{
	KSHLIB	*kip = &kshlib_data ;
	return kip->runmode ;
}
/* end subroutine (lib_kshend) */


int lib_sigintr()
{
	KSHLIB	*klp = &kshlib_data ;
	return (klp->f_sigint) ? SR_INTR : SR_OK ;
}
/* end subroutine (lib_sigintr) */


int lib_sigterm()
{
	KSHLIB	*klp = &kshlib_data ;
	return (klp->f_sigterm) ? SR_EXIT : SR_OK ;
}
/* end subroutine (lib_sigterm) */


int lib_sigreset(int sn)
{
	KSHLIB	*klp = &kshlib_data ;
	switch (sn) {
	case SIGTERM:
	    klp->f_sigterm = 0 ;
	    break ;
	case SIGINT:
	    klp->f_sigint = 0 ;
	    break ;
	} /* end switch */
	return SR_OK ;
}
/* end subroutine (lib_sigreset) */


int lib_initmemalloc(int f)
{
	int		rs = SR_OK ;
	const char	*sym = "lockmemalloc_set" ;
	void		*sop = RTLD_SELF ;
	void		*p ;
	if (f) {
	    if ((p = dlsym(sop,sym)) != NULL) {
	        int	(*fun)(int) = (int (*)(int)) p ;
	        rs = (*fun)(TRUE) ;
#if	CF_DEBUGN
	        nprintf(NDEBFNAME,"lib_initmemalloc: LOCKMEMALLOC rs=%d\n",rs) ;
#endif
	    }
	} /* end if (enabled) */
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_initmemalloc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_initmemalloc) */


/* ARGSUSED */
int lib_caller(func,argc,argv,envv,contextp)
int		(*func)(int,const char **,void *) ;
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	int	rs ;
	int	ex = EX_OK ;

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_caller: &environ=%p\n",&environ) ;
	nprintf(NDEBFNAME,"lib_caller: environ=%p\n",environ) ;
	nprintf(NDEBFNAME,"lib_caller: envv=%p\n",envv) ;
#endif

#if	CF_DEBUGENV && CF_DEBUGN
	if (envv != NULL)
	    ndebugenv("lib_caller",envv) ;
#endif

	if ((rs = lib_initenviron()) >= 0) {

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_caller: func()\n") ;
#endif

	    if (func != NULL) {
	        ex = (*func)(argc,argv,contextp) ;
	    } else
		ex = EX_NOPROG ;

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_caller: func() ex=%u\n",ex) ;
#endif

	} /* end if (lib_initenviron) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_caller: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_caller) */


int lib_callfunc(func,argc,argv,envv,contextp)
int		(*func)(int,const char **,const char **,void *) ;
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	int	rs ;
	int	ex = EX_OK ;

	if ((rs = lib_initenviron()) >= 0) {
	    if (func != NULL) {
	        ex = (*func)(argc,argv,envv,contextp) ;
	    } else
		ex = EX_NOPROG ;
	} /* end if (lib_initenviron) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

	return ex ;
}
/* end subroutine (lib_callfunc) */


int lib_callcmd(name,argc,argv,envv,contextp)
const char	name[] ;
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	int	rs ;
	int	ex = EX_OK ;

	if ((rs = lib_initenviron()) >= 0) {
	    if ((name != NULL) && (name[0] != '\0')) {
		char	symname[SYMNAMELEN+1] ;
	        if ((rs = sncpy2(symname,MAXNAMELEN,"p_",name)) >= 0) {
		    void	*sop = RTLD_SELF ;
		    void	*p ;
	            if ((p = dlsym(sop,symname)) != NULL) {
			int (*cf)(int,cchar **,cchar **,void *) ;
			    cf = (int (*)(int,cchar **,cchar **,void *)) p ;
	                    ex = (*cf)(argc,argv,envv,contextp) ;
	            } else
	                ex = EX_UNAVAILABLE ;
	        } else
	            ex = EX_NOPROG ;
	    } else
	        ex = EX_NOPROG ;
	} /* end if (lib_initenviron) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_OSERR ;

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_callcmd: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_callcmd) */


/* ARGSUSED */
int lib_callprog(name,argc,argv,envv,contextp)
const char	name[] ;
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	int	rs ;
	int	rs1 ;
	int	ex = EX_OK ;
	if ((rs = kshlib_init()) >= 0) {
	    if ((rs = lib_kshbegin(NULL)) >= 0) {
		ex = lib_callcmd(name,argc,argv,envv,NULL) ;
		rs1 = lib_kshend() ;
		if (rs >= 0) rs = rs1 ;
	    } else
		ex = EX_SOFTWARE ;
	} /* end if (kshlib_init) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_OSERR ;

#if	CF_DEBUGS
	nprintf(NDEBFNAME,"lib_callprog: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_callprog) */


/* ARGSUSED */
int lib_mainbegin(const char **envv)
{
	return SR_OK ;
}
/* end subroutine (lib_mainbegin) */


int lib_mainend(void)
{
	return SR_OK ;
}
/* end subroutine (lib_mainend) */


#if	CF_PLUGIN
ulong plugin_version(void) {
	return 20131127UL ;
}
/* end subroutine (plugin_version) */
#endif /* CF_PLUGIN */


/* local subroutines */


static int kshlib_init()
{
	struct kshlib	*uip = &kshlib_data ;
	int	rs = 1 ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = kshlib_atforkbefore ;
	        void	(*a)() = kshlib_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(kshlib_fini)) >= 0) {
		        rs = 0 ;
	    	        uip->f_initdone = TRUE ;
		    }
		    if (rs < 0)
		        uc_atforkrelease(b,a,a) ;
	        } /* end if (uc_atfork) */
	        if (rs < 0)
	            uip->f_init = FALSE ;
	    } /* end if (ptm_create) */
	} else {
	    while (! uip->f_initdone) msleep(1) ;
	}
	return rs ;
}
/* end subroutine (kshlib_init) */


static void kshlib_fini()
{
	struct kshlib	*uip = &kshlib_data ;
	if (uip->f_initdone) {
	    {
	        void	(*b)() = kshlib_atforkbefore ;
	        void	(*a)() = kshlib_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct kshlib)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (kshlib_fini) */


static void kshlib_atforkbefore()
{
	KSHLIB	*uip = &kshlib_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (kshlib_atforkbefore) */


static void kshlib_atforkafter()
{
	KSHLIB	*uip = &kshlib_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (kshlib_atforkafter) */


static void kshlib_sighand(int sn)
{
	KSHLIB	*klp = &kshlib_data ;
	switch (sn) {
	case SIGINT:
	    klp->f_sigint = TRUE ;
	    break ;
	case SIGKILL:
	default:
	    klp->f_sigterm = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (kshlib_sighand) */


#if	CF_DEBUGENV && CF_DEBUGN
static int ndebugenv(s,ev)
const char	*s ;
const char	**ev ;
{
	const char	*dfn = NDEBFNAME ;
	const char	*ep ;
	if (s != NULL) {
	    if (ev != NULL) {
	        int	i ;
		const char	*fmt = "%s: e%03u=>%t<\n" ;
	        nprintf(dfn,"%s: env¬\n", s) ;
	        for (i = 0 ; ev[i] != NULL ; i += 1) {
	            ep = ev[i] ;
		    nprintf(dfn,fmt,s,i,ep,strlinelen(ep,-1,50)) ;
	        }
	        nprintf(dfn,"%s: nenv=%u\n", s,i) ;
	    } else
	        nprintf(dfn,"%s: environ=*null*\n",s) ;
	}
	return 0 ;
}
/* end subroutine (ndebugenv) */
#endif /* CF_DEBUGENV */


