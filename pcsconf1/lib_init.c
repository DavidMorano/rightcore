/* lib_init */

/* library initialization for KSH built-in command libraries */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGENV	0		/* debug environment */
#define	CF_LIBINIT	0		/* activate 'lib_init()' */
#define	CF_ENVLOAD	0		/* load 'environ' on init */
#define	CF_PLUGIN	1		/* define 'plugin_version()' */
#define	CF_LOCKMEMALLOC	1		/* call |lockmemalloc(3uc)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Audix Database Processor (DBP) work.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutine:

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

	int lib_initenviron() ;

	Arguments:

	*none*

	Returns:

	<0	error
	>=0	OK

	= Notes

	+ lockmemalloc_set

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


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<dlfcn.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	NDEBFNAME	"ndeb"


/* external subroutines */

#if	CF_LOCKMEMALLOC
extern int	lockmemalloc_set(int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif


/* external variables */

extern char	**environ ; /* this is empty (when in our own link-group) */


/* local structures */


/* forward references */

#if	CF_LOCKMEMALLOC
int lib_initmemalloc() ;
#endif

#if	CF_DEBUGENV && CF_DEBUGS
static int debugenv(const char *,const char **) ;
#endif


/* local (BSS) variables */

static pthread_mutex_t	lib_envmutex = PTHREAD_MUTEX_INITIALIZER ;

static const char	*defenviron[] = {
	"_PROCSTATE=screwed",
	NULL
} ;


/* exported subroutines */


#if	CF_LIBINIT

/* ARGSUSED */
void lib_init(int flags,void *contextp)
{
	void	*sop = RTLD_DEFAULT ;

	const char	***eppp ;


#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_init: 1 environ=%p\n",environ) ;
#endif

#if	CF_ENVLOAD || CF_DEBUGS || CF_DEBUGN || CF_DEBUGENV
	eppp = dlsym(sop,"environ") ;
#endif

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_init: eppp=%p\n",eppp) ;
	if (eppp != NULL)
	nprintf(NDEBFNAME,"lib_init: 2 environ=%p\n",*eppp) ;
#endif

#if	CF_ENVLOAD
	environ = (char **) *eppp ;
#endif

#if	CF_DEBUGENV && CF_DEBUGS
	if ((eppp != NULL) && (*eppp != NULL)) {
	    const char	**envv = (const char **) *eppp ;
	    debugenv("lib_init",envv) ;
	}
#endif

#if	CF_LOCKMEMALLOC
	(void) lib_initmemalloc() ;
#endif

}
/* end subroutine (lib_init) */

#else /* CF_LIBINIT */

/* ARGSUSED */
void lib_init(int flags,void *contextp)
{
#if	CF_LOCKMEMALLOC
	(void) lib_initmemalloc() ;
#endif
}
/* end subroutine (lib_init) */

#endif /* CF_LIBINIT */


int lib_initenviron()
{
	int	rs = SR_OK ;
	if (environ == NULL) {
	    if ((rs = ptm_lock(&lib_envmutex)) >= 0) {
	        char ***eppp = dlsym(RTLD_DEFAULT,"environ") ;
	        if ((eppp != NULL) && (eppp != &environ)) environ = *eppp ;
	        if (environ == NULL) environ = (char **) defenviron ;
	        rs = ptm_unlock(&lib_envmutex) ;
	    } /* end if */
	} /* end if (environ) */
#if	CF_DEBUGN && 0
	nprintf(NDEBFNAME,"lib_initenviron: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_initenviron) */


#if	CF_LOCKMEMALLOC
int lib_initmemalloc()
{
	int		rs = SR_OK ;
	const char	*sym = "lockmemalloc_set" ;
	void		*sop = RTLD_SELF ;
	void		*p ;
	if ((p = dlsym(sop,sym)) != NULL) {
	    int	(*fun)(int) = (int (*)(int)) p ; /* b*llsh*t for LINT */
	    rs = (*fun)(TRUE) ;
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_initmemalloc: LOCKMEMALLOC rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"lib_initmemalloc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_initmemalloc) */
#endif /* CF_LOCKMEMALLOC */


#if	CF_PLUGIN

long plugin_version() {
	return 20131127L ;
}
/* end subroutine (plugin_version) */

#endif /* CF_PLUGIN */


/* local subroutines */


#if	CF_DEBUGENV && CF_DEBUGN
static int debugenv(s,ev)
const char	*s ;
const char	**ev ;
{
	const char	*dfn = NDEBFNAME ;
	const char	*ep ;
	if (s != NULL) {
	    int	i ;
	    nprintf(dfn,"%s: env¬\n", s) ;
	    for (i = 0 ; ev[i] != NULL ; i += 1) {
	        ep = ev[i] ;
		nprintf(dfn,"%s: e%03u=>%t<\n", s,i,
			ep,strlinelen(ep,-1,50)) ;
	    }
	    nprintf(dfn,"%s: nenv=%u\n", s,i) ;
	}
	return 0 ;
}
/* end subroutine (debugenv) */
#endif /* CF_DEBUGENV */


