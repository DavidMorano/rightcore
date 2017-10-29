/* kshlib */

/* library initialization for KSH built-in command libraries */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGENV	0		/* debug environment */
#define	CF_DEBUGHEXB	0		/* debug w/ |debugprinthexblock()| */
#define	CF_PLUGIN	1		/* define 'plugin_version()' */
#define	CF_LOCKMEMALLOC	1		/* call |lockmemalloc(3uc)| */
#define	CF_KSHRUN	1		/* run background under KSH */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	KSH built-in library support.

	------------------------------------------------------------------------
	Name:

	lib_init

	Description:

        This subroutine is called by KSH when it loads this shared library. We
        use this call to initialize some things that are partuclar to when
        executing from within KSH. One of these is to set the underlying memory
        management facility to implement a MUTEX lock around its operations.
        This helps guard against a failure if the KSH-native version of the
        normal memory management subroutines are somehow linked in (loaded)
        rather than the standard default UNIX® system subroutines.

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
        builtin-command (CMD) link-group to the same environment of our caller
        (usually the SHELL itself). Depending on how this module is loaded, it
        may be in the link-group of its parent (not at all unusual) or it may be
        in its own link-group. In theory it could even be on its own link-map,
        but that is not at all a typical situation so we ignore that for our
        purposes. If this modeule is loaded into its own link-group, some means
        has to be provided to set the 'environ' variable (above) to the
        envionment of the calling parent (at least set to something). When the
        builtin commands are called by the SHELL, they (the builtin commands)
        are called directly from it. But the CMD link-group has its own copy of
        the 'environ' variable which would not have yet been set at all by
        anybody (any subroutine anywhere). So when CMDs are called by the SHELL,
        the CMD subroutine itself calls 'lib_initenviron()' in order to set the
        CMD link-group copy of the 'environ' variable to the same as what exists
        wihtin the SHELL itself.

        When CMDs are not called by the SHELL, but rather by some other means,
        some other way to set the 'environ' variable has to be established.
        Possible other ways are:

        1. this subroutine is not linked in, so there is *no* separate copy of
        'environ' in the first place (completely typical in regular programs)

        2. by the caller instead calling the CMD subroutine though an
        intermediate subroutine (like named 'lib_caller()') and which gets its
        internal 'environ' copy set with that subroutine before the CMD
        subroutine is called in turn.

	Synopsis:

	int lib_initenviron(void *contextp)

	Arguments:

	contextp	context pointer

	Returns:

	<0	error
	>=0	OK


	------------------------------------------------------------------------
	Name:

	lib_caller

	Description:

	What in the world does this subroutine do?

        This subroutine lets us call a command (CMD) function, otherwise known
        as a command "builtin" (from the SHELL language on the subject) while
        giving it an arbitrary environment determined by the caller. In the
        infinite (short-sighted) wisdom of the creators of the builtin command
        interface, it was neglected to provide the capability to pass an
        arbitrary environment (like what is possible -- but not often used) with
        regular UNIX® process calls (using 'exec(2)' and friends). Without this
        subroutine, and having to call the command function directory, there is
        no way to pass or to create a unique environment for the function since
        it is forced to simply inherit the environment of the caller.

	Synopsis:

	int lib_caller(func,argc,argv,envv,contextp)
	int		(*func)(int,cchar **,void *) ;
	int		argc ;
	cchar		*argv[] ;
	cchar		*envv[] ;
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

        When CF_LOCKMEMALLOC is set (non-zero) above, the LOCKMEMALLOC facility
        is (possibly) made available for use. Actual use depends on whether the
        module (LOCKMEMALLOC) is available somewhere in the current link-map. If
        it is indeed available, we turn it on (with the proper command). We do
        this (turn it ON) because the KSH program does not provide mutex locks
        around its memory allocation subroutines (which emulate |malloc(3c)| and
        friends). Of course KSH does not use any of the standard system
        subroutines because, well, that would be way too easy wouldn't it? The
        KSH program thinks that it is better than everyone else and so it uses
        its own memory-allocation facility. One problem: it did not protect its
        own facility with mutex locks. It did not do this because the shell is
        single threaded throughout. But this causes problems (like program
        crashes) when some dynmically loaded code splits into a multi-threaded
        mode. Yes, bang, a mess of the underlying memory-allocation system and
        the expected program crash as a result. The use of the LOCKMEMALLOC
        facility places mutex locks around all calls to the underlying memory
        allocation subroutines. This can help but also might still not be enough
        (since some shell code can still be used even within multi-threaded
        code). But every little bit helps.

        One would think that everything today is multi-thread safe, but NO.
        There are still some hold-outs, and these hold-outs make it bad for
        everybody!

	Well, there it is.

	Synospsis:

	int lib_initmemalloc(int f)

	Arguments:

	f		switch (0=OFF, 1=ON)

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#define	KSHLIB_MASTER	1	/* claim excemption from own forwards */


#include	<envstandards.h>

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<dlfcn.h>
#include	<poll.h>
#include	<string.h>

#include	<vsystem.h>
#include	<upt.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<sigman.h>
#include	<sockaddress.h>
#include	<raqhand.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sesmsg.h"
#include	"msginfo.h"
#include	"kshlib.h"


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000		/* poll-time multiplier */
#endif

#ifndef	MSGHDR
#define	MSGHDR		srtuct msghdr
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold int128_t in decimal */
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#define	KSHLIB_MEMALLOC		1
#define	KSHLIB_WHERE		"embedded"
#define	KSHLIB_SYMSEARCH	RTLD_SELF
#else
#define	KSHLIB_MEMALLOC		1
#define	KSHLIB_WHERE		"standalone"
#define	KSHLIB_SYMSEARCH	RTLD_SELF
#endif

#define	VARKSHLIBRUN	"KSHLIB_RUN"

#define	NDF		"kshlib.deb"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	TO_LOCKENV	10

#define	KSHLIB		struct kshlib
#define	KSHLIB_SCOPE	PTHREAD_SCOPE_PROCESS
#define	KSHLIB_SESDNAME	"/var/tmp/sessions"

#define	STORENOTE	struct storenote


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy2w(char *,int,cchar *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	listenusd(cchar *,mode_t,int) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

#if	CF_LOCKMEMALLOC
extern int	lockmemalloc_set(int) ;
#endif

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

typedef int (*subcmd_t)(int,cchar **,cchar **,void *) ;

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef cchar	cchar ;
#endif

#ifndef	TYPEDEF_TWORKER
#define	TYPEDEF_TWORKER	1
typedef	int (*tworker)(void *) ;
#endif

typedef	int (*cmdsub_t)(int,cchar **,cchar **,void *) ;
typedef	int (*func_caller)(int,cchar **,void *) ;

struct kshlib {
	PTM		m ;		/* mutex data */
	PTM		menv ;		/* mutex environment */
	PTC		c ;		/* condition variable */
	SIGMAN		sm ;
	cchar		*reqfname ;
	SOCKADDRESS	servaddr ;	/* server address */
	RAQHAND		mq ;		/* message queue */
	pthread_t	tid ;
	pid_t		pid ;
	volatile int	f_init ;
	volatile int	f_initdone ;
	volatile int	f_running ;
	volatile int	f_capture ;
	volatile int	f_exiting ;
	volatile int	waiters ;
	int		f_sigterm ;
	int		f_sigintr ;
	int		f_mq ;
	int		runmode ;
	int		serial ;
	int		sfd ;
	int		cdefs ;		/* defualt count */
	int		servlen ;	/* serv-addr length */
} ;

struct storenote {
	time_t		stime ;
	cchar		*dbuf ;
	cchar		*user ;
	char		*a ;
	int		type ;
	int		dlen ;
} ;


/* forward references */

int	lib_initenviron(void *) ;
int	lib_callcmd(cchar *,int,cchar **,cchar **,void *) ;
int 	lib_callfunc(subcmd_t,int,cchar **,cchar **,void *) ;

static int	kshlib_init(void) ;
static void	kshlib_fini(void) ;

static void	kshlib_atforkbefore() ;
static void	kshlib_atforkafter() ;
static void	kshlib_sighand(int) ;

static int	kshlib_begin(KSHLIB *) ;
static int	kshlib_end(KSHLIB *) ;
static int	kshlib_workcheck(KSHLIB *,cchar **) ;
static int	kshlib_runbegin(KSHLIB *) ;
static int	kshlib_runner(KSHLIB *) ;
static int	kshlib_runend(KSHLIB *) ;
static int	kshlib_entfins(KSHLIB *) ;
static int	kshlib_mq(KSHLIB *) ;
static int	kshlib_mkreqfname(KSHLIB *,char *,cchar *) ;
static int	kshlib_worker(KSHLIB *) ;
static int	kshlib_workecho(KSHLIB *,MSGINFO *) ;
static int	kshlib_workbiff(KSHLIB *,MSGINFO *) ;
static int	kshlib_workbiffer(KSHLIB *,SESMSG_BIFF *) ;
static int	kshlib_workgen(KSHLIB *,MSGINFO *) ;
static int	kshlib_workgener(KSHLIB *,SESMSG_GEN *) ;
static int	kshlib_workdef(KSHLIB *,MSGINFO *) ;
static int	kshlib_msgenter(KSHLIB *,STORENOTE *) ;
static int	kshlib_reqopen(KSHLIB *) ;
static int	kshlib_reqopener(KSHLIB *,cchar *) ;
static int	kshlib_reqsend(KSHLIB *,MSGINFO *,int) ;
static int	kshlib_reqrecv(KSHLIB *,MSGINFO *) ;
static int	kshlib_reqclose(KSHLIB *) ;
static int	kshlib_poll(KSHLIB *) ;
static int	kshlib_cmdsend(KSHLIB *,int) ;
static int	kshlib_capbegin(KSHLIB *,int) ;
static int	kshlib_capend(KSHLIB *) ;
static int	kshlib_sigbegin(KSHLIB *) ;
static int	kshlib_sigend(KSHLIB *) ;

int		lib_initmemalloc(int) ;

static int	storenote_start(STORENOTE *,int,time_t,cchar *,cchar *,int) ;
static int	storenote_finish(STORENOTE *) ;

static int	mallocstrw(cchar *,int,cchar **) ;
static int	sdir(cchar *,int) ;
static int	mksdir(cchar *,mode_t) ;
static int	mksdname(char *,cchar *,pid_t) ;

#if	CF_DEBUGENV && CF_DEBUGN
static int	ndebugenv(cchar *,cchar **) ;
#endif


/* local variables */

static cchar	*defenviron[] = {
	"_PROCSTATE=screwed",
	NULL
} ;

static KSHLIB		kshlib_data ; /* zero-initialized */

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
	0
} ;


/* exported subroutines */


/* ARGSUSED */
void lib_init(int flags,void *contextp)
{

#if	CF_DEBUGENV && CF_DEBUGN
	{
	    const pid_t	pid = getpid() ;
	    void *p = dlsym(RTLD_DEFAULT,"environ") ;
	    nprintf(NDF,"lib_init: ent pid=%u\n",pid) ;
	    nprintf(NDF,"lib_init: flags=%16ß (%u)\n",flags,flags) ;
	    if (p != NULL) {
		cchar	***evp = (cchar ***) p ;
		cchar	**ev ;
		ev = *evp ;
	        nprintf(NDF,"lib_init: p=%P\n",p) ;
	        nprintf(NDF,"lib_init: main-environ{%P}=%P\n",evp,ev) ;
	        nprintf(NDF,"lib_init: lib-environ{%P}=%P\n",
			&environ,environ) ;
	        ndebugenv("lib_init-m",ev) ;
	    }
	    nprintf(NDF,"lib_init: lib-environ=%P\n",environ) ;
	    nprintf(NDF,"lib_init: contextp=%P\n",contextp) ;
	}
#endif /* CF_DEBUGN */

#if	CF_DEBUGENV && CF_DEBUGN && 0
	if (environ != NULL) {
	    cchar	**ev = (cchar **) environ ;
	    ndebugenv("lib_init-l",ev) ;
	}
#endif

#if	CF_LOCKMEMALLOC
	{
	    const int	f = KSHLIB_MEMALLOC ;
	    (void) lib_initmemalloc(f) ;
	}
#endif /* CF_LOCKMEMALLOC */

#if	CF_KSHRUN
	{
	    int	rs ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_init: KSHRUN\n") ;
#endif
	    if ((rs = lib_initenviron(contextp)) >= 0) {
	        if ((rs = kshlib_init()) >= 0) {
		    KSHLIB	*uip = &kshlib_data ;
		    cchar	**envv = (cchar **) environ ;
		    rs = kshlib_workcheck(uip,envv) ;
	        } /* end if (init) */
	    } /* end if (lib_initenviron) */
#if	CF_DEBUGN
	nprintf(NDF,"lib_init: KSHRUN rs=%d\n",rs) ;
#endif
	}
#endif /* CF_KSHRUN*/

#if	CF_DEBUGN
	nprintf(NDF,"lib_init: ret\n") ;
#endif

}
/* end subroutine (lib_init) */


void lib_fini(void)
{
	kshlib_fini() ;
}
/* end subroutine (lib_fini) */


/* is this multi-thread safe or not? */
int lib_initenviron(void *contextp)
{
	int		rs = SR_OK ;
	if (environ == NULL) {
	        char ***eppp = dlsym(RTLD_DEFAULT,"environ") ;
	        if ((eppp != NULL) && (eppp != &environ)) environ = *eppp ;
	        if (environ == NULL) environ = (char **) defenviron ;
	} /* end if (environ) */
	return rs ;
}
/* end subroutine (lib_initenviron) */


int lib_mainbegin(cchar **envv)
{
	int		rs = SR_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_mainbegin: ent\n") ;
#endif

	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_sigbegin(uip)) >= 0) {
		rs = kshlib_workcheck(uip,envv) ;
		if (rs < 0)
	    	    kshlib_sigend(uip) ;
	    } /* end if (kshlib_sigbegin) */
	} /* end if (kshlib_init) */

#if	CF_DEBUGN
	nprintf(NDF,"lib_mainbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (lib_mainbegin) */


int lib_mainend(void)
{
	KSHLIB		*uip = &kshlib_data ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_mainend: ent\n") ;
#endif

	if (uip->f_running) {
	    rs1 = kshlib_runend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (running) */

	rs1 = kshlib_sigend(uip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_mainend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (lib_mainend) */


/* ARGSUSED */
int lib_kshbegin(void *contextp)
{
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_kshbegin: ent\n") ;
#endif
	if ((rs = lib_initenviron(contextp)) >= 0) {
	    if ((rs = kshlib_init()) >= 0) {
		KSHLIB	*kip = &kshlib_data ;
		if ((rs = kshlib_sigbegin(kip)) >= 0) {
		    cchar	**envv = (cchar **) environ ;
		    if ((rs = kshlib_workcheck(kip,envv)) >= 0) {
			kip->runmode |= KSHLIB_RMKSH ;
		    }
		    if (rs < 0) {
			kip->runmode &= (~ KSHLIB_RMKSH) ;
			kshlib_sigend(kip) ;
		    }
		} /* end if (kshlib_sigbegin) */
	    } /* end if (kshlib_init) */
	} /* end if (lib_initenviron) */
	return rs ;
} 
/* end subroutine (lib_kshbegin) */


int lib_kshend(void)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	int		rs1 ;
	kip->serial += 1 ;
	kip->runmode &= (~ KSHLIB_RMKSH) ;
	rs1 = sigman_finish(&kip->sm) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (lib_kshend) */


int lib_runmode(void)
{
	KSHLIB		*kip = &kshlib_data ;
	return kip->runmode ;
}
/* end subroutine (lib_runmode) */


int lib_serial(void)
{
	KSHLIB		*kip = &kshlib_data ;
	int		s = kip->serial ;
	return s ;
}
/* end subroutine (lib_serial) */


int lib_sigintr(void)
{
	KSHLIB		*kip = &kshlib_data ;
	return (kip->f_sigintr) ? SR_INTR : SR_OK ;
}
/* end subroutine (lib_sigintr) */


int lib_sigterm(void)
{
	KSHLIB		*kip = &kshlib_data ;
	return (kip->f_sigterm) ? SR_EXIT : SR_OK ;
}
/* end subroutine (lib_sigterm) */


int lib_sigreset(int sn)
{
	KSHLIB		*kip = &kshlib_data ;
	switch (sn) {
	case SIGTERM:
	    kip->f_sigterm = 0 ;
	    break ;
	case SIGINT:
	    kip->f_sigintr = 0 ;
	    break ;
	} /* end switch */
	return SR_OK ;
}
/* end subroutine (lib_sigreset) */


int lib_initmemalloc(int f)
{
	int		rs = SR_OK ;
	cchar		*sym = "lockmemalloc_set" ;
	void		*sop = RTLD_SELF ;
	void		*p ;
	if (f) {
	    if ((p = dlsym(sop,sym)) != NULL) {
	        int	(*fun)(int) = (int (*)(int)) p ;
	        rs = (*fun)(TRUE) ;
#if	CF_DEBUGN
	        nprintf(NDF,"lib_initmemalloc: LOCKMEMALLOC rs=%d\n",rs) ;
#endif
	    }
	} /* end if (enabled) */
#if	CF_DEBUGN
	nprintf(NDF,"lib_initmemalloc: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_initmemalloc) */


int lib_progaddr(cchar *name,void *app)
{
	int		rs = SR_OK ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_progaddr: ent name=>%s<\n",name) ;
#endif
	if ((name != NULL) && (name[0] != '\0')) {
	    const int	symlen = SYMNAMELEN ;
	    char	symbuf[SYMNAMELEN+1] ;
	    if ((rs = sncpy2(symbuf,symlen,"p_",name)) >= 0) {
		void	*sop = KSHLIB_SYMSEARCH ;
		void	*p ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_progaddr: sym=%s\n",symbuf) ;
#endif
	        if ((p = dlsym(sop,symbuf)) != NULL) {
		    if (app != NULL) {
			caddr_t	*sub = (caddr_t *) app ;
	   	        *sub = (caddr_t) p ;
		    }
		} else
		    rs = SR_NOENT ;
	    } /* end if (sncpy) */
	} else
	    rs = SR_NOENT ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_progaddr: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (lib_progaddr) */


int lib_proghave(cchar *name)
{
	return lib_progaddr(name,NULL) ;
}
/* end subroutine (lib_proghave) */


int lib_progcall(name,argc,argv,envv,contextp)
cchar		name[] ;
int		argc ;
cchar		*argv[] ;
cchar		*envv[] ;
void		*contextp ;
{
	cmdsub_t	addr ;
	int		rs ;
	int		ex = EX_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_progcall: ent name=%s\n",name) ;
	nprintf(NDF,"lib_progcall: from=%s\n",KSHLIB_WHERE) ;
#endif

	if ((rs = lib_progaddr(name,&addr)) >= 0) {
	    if ((rs = lib_initenviron(NULL)) >= 0) {
	        if ((rs = kshlib_init()) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"lib_progcall: call()\n") ;
#endif
	            ex = (*addr)(argc,argv,envv,NULL) ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_progcall: call() ex=%u\n",ex) ;
#endif
	        } else
	            ex = EX_OSERR ;
	    } else
	       ex = EX_OSERR ;
	} else
	    ex = EX_NOPROG ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_progcall: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_progcall) */


int lib_progcalla(func,argc,argv,envv,contextp)
const void	*func ;
int		argc ;
cchar		*argv[] ;
cchar		*envv[] ;
void		*contextp ;
{
	subcmd_t	f = (subcmd_t) func ;
	return lib_callfunc(f,argc,argv,envv,contextp) ;
}
/* end subroutine (lib_progcalla) */


/* ARGSUSED */
int lib_caller(fa,argc,argv,envv,contextp)
const void	*fa ;
int		argc ;
cchar		*argv[] ;
cchar		*envv[] ;
void		*contextp ;
{
	func_caller	func = (func_caller) fa ;
	int		rs ;
	int		ex = EX_OK ;

#if	CF_DEBUGS
	nprintf(NDF,"lib_caller: &environ=%p\n",&environ) ;
	nprintf(NDF,"lib_caller: environ=%p\n",environ) ;
	nprintf(NDF,"lib_caller: envv=%p\n",envv) ;
#endif

#if	CF_DEBUGENV && CF_DEBUGN
	if (envv != NULL)
	    ndebugenv("lib_caller",envv) ;
#endif

	if ((rs = lib_initenviron(contextp)) >= 0) {

#if	CF_DEBUGS
	nprintf(NDF,"lib_caller: func()\n") ;
#endif

	    if (func != NULL) {
	        ex = (*func)(argc,argv,contextp) ;
	    } else
		ex = EX_NOPROG ;

#if	CF_DEBUGS
	nprintf(NDF,"lib_caller: func() ex=%u\n",ex) ;
#endif

	} /* end if (lib_initenviron) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

#if	CF_DEBUGS
	nprintf(NDF,"lib_caller: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_caller) */


int lib_callfunc(func,argc,argv,envv,contextp)
subcmd_t	func ;
int		argc ;
cchar		*argv[] ;
cchar		*envv[] ;
void		*contextp ;
{
	int		rs ;
	int		ex = EX_OK ;

	if ((rs = lib_initenviron(contextp)) >= 0) {
	    if ((rs = kshlib_init()) >= 0) {
	        if (func != NULL) {
	            ex = (*func)(argc,argv,envv,contextp) ;
	        } else
		    ex = EX_NOPROG ;
	    } else
	        ex = EX_OSERR ;
	} /* end if (lib_initenviron) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

	return ex ;
}
/* end subroutine (lib_callfunc) */


int lib_callcmd(name,argc,argv,envv,contextp)
cchar		name[] ;
int		argc ;
cchar		*argv[] ;
cchar		*envv[] ;
void		*contextp ;
{
	int		rs = SR_OK ;
	int		ex = EX_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_callcmd: ent name=%s\n",name) ;
	nprintf(NDF,"lib_callcmd: from=%s\n",KSHLIB_WHERE) ;
#endif

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
	if ((rs < 0) && (ex == EX_OK)) ex = EX_OSERR ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_callcmd: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_callcmd) */


int lib_noteread(KSHLIB_NOTE *rp,int ni)
{
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	if (rp == NULL) return SR_FAULT ;
	memset(rp,0,sizeof(KSHLIB_NOTE)) ;
	if (ni < 0) return SR_INVALID ;
	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	        if ((rs = kshlib_mq(uip)) >= 0) {
		    STORENOTE	*ep ;
		    if ((rs = raqhand_acc(&uip->mq,ni,&ep)) >= 0) {
			if (ep != NULL) {
			    rp->stime = ep->stime ;
			    rp->type = ep->type ;
			    rp->dlen = ep->dlen ;
			    strwcpy(rp->dbuf,ep->dbuf,SESMSG_NBUFLEN) ;
			    strwcpy(rp->user,ep->user,SESMSG_USERLEN) ;
			    rc = 1 ;
			} /* end if (non-null) */
		    } /* end if (raqhand_acc) */
		} /* end if (kshlib_mq) */
	        rs1 = kshlib_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (kshlib_init) */
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (lib_noteread) */


int lib_notedel(int ni)
{
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	if (ni < 0) return SR_INVALID ;
	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	        if ((rs = kshlib_mq(uip)) >= 0) {
		    rs = raqhand_del(&uip->mq,ni) ;
		    rc = rs ;
		} /* end if (kshlib_mq) */
	        rs1 = kshlib_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (kshlib_init) */
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (lib_notedel) */


#if	CF_PLUGIN
ulong plugin_version(void) {
	return 20131127UL ;
}
/* end subroutine (plugin_version) */
#endif /* CF_PLUGIN */


/* local subroutines */


static int kshlib_init(void)
{
	KSHLIB		*uip = &kshlib_data ;
	int		rs = 1 ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	            void	(*b)() = kshlib_atforkbefore ;
	            void	(*a)() = kshlib_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(kshlib_fini)) >= 0) {
			    uip->pid = getpid() ;
			    uip->sfd = -1 ;
		            rs = 0 ;
	    	            uip->f_initdone = TRUE ;
		        }
		        if (rs < 0)
		            uc_atforkrelease(b,a,a) ;
	            } /* end if (uc_atfork) */
	            if (rs < 0)
	                ptc_destroy(&uip->c) ;
	        } /* end if (ptc_create) */
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while (! uip->f_initdone) msleep(1) ;
	}
	return rs ;
}
/* end subroutine (kshlib_init) */


static void kshlib_fini(void)
{
	struct kshlib	*uip = &kshlib_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        kshlib_runend(uip) ;
		kshlib_end(uip) ;
	    }
	    {
	        void	(*b)() = kshlib_atforkbefore ;
	        void	(*a)() = kshlib_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct kshlib)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (kshlib_fini) */


static int kshlib_mq(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_mq) {
	    rs = kshlib_begin(uip) ;
	}
	return rs ;
}
/* end subroutine (kshlib_mq) */


static int kshlib_begin(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_mq) {
	    const int	n = KSHLIB_NENTS ;
	    if ((rs = raqhand_start(&uip->mq,n,0)) >= 0) {
	        uip->f_mq = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (kshlib_begin) */


static int kshlib_end(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->f_mq) {
	    rs1 = kshlib_entfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->f_mq = FALSE ;
	    rs1 = raqhand_finish(&uip->mq) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (kshlib_end) */


static int kshlib_entfins(KSHLIB *uip)
{
	RAQHAND		*qlp = &uip->mq ;
	STORENOTE	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; raqhand_get(qlp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
		rs1 = storenote_finish(ep) ;
		if (rs >= 0) rs = rs1 ;
		rs1 = uc_libfree(ep) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (non-null) */
	} /* end for */
	return rs ;
}
/* end subroutine (kshlib_entfins) */


static int kshlib_workcheck(KSHLIB *kip,cchar **envv)
{
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*vp = getourenv(envv,VARKSHLIBRUN) ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workcheck: ent v=%s\n",vp) ;
#endif
	        if (vp != NULL) {
	    	    int	v ;
	    	    if ((rs1 = cfdeci(vp,-1,&v)) >= 0) {
			switch (v) {
			case 1:
		            rs = kshlib_runbegin(kip) ;
		            break ;
		        } /* end switch */
	            } /* end if (value) */
	        } /* end if (check run-state) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workcheck: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_workcheck) */


static int kshlib_runbegin(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runbegin: ent f_running=%u\n",uip->f_running) ;
#endif

	if (! uip->f_running) {
	    if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
		if (! uip->f_running) {
		    if ((rs = kshlib_reqopen(uip)) >= 0) {
		        rs = kshlib_runner(uip) ;
		        f = rs ;
		    } /* end if (kshlib_reqopen) */
		} /* end if (not running) */
		rs1 = kshlib_capend(uip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_runbegin) */


static int kshlib_runner(KSHLIB *uip)
{
	PTA		a ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&a)) >= 0) {
	    const int	scope = KSHLIB_SCOPE ;
	    if ((rs = pta_setscope(&a,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) kshlib_worker ;
		if ((rs = uptcreate(&tid,NULL,wt,uip)) >= 0) {
		    uip->f_running = TRUE ;
		    uip->tid = tid ;
		    f = TRUE ;
		} /* end if (pthread-create) */
#if	CF_DEBUGN
		nprintf(NDF,"kshlib_runner: pt-create rs=%d tid=%u\n",
			rs,tid) ;
#endif
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runner: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_runner) */


static int kshlib_runend(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runend: ent run=%u\n",uip->f_running) ;
#endif

	if (uip->f_running) {
	    const int	cmd = sesmsgtype_exit ;
	    if ((rs = kshlib_cmdsend(uip,cmd)) >= 0) {
	 	pthread_t	tid = uip->tid ;
		int		trs ;
		if ((rs = uptjoin(tid,&trs)) >= 0) {
		    uip->f_running = FALSE ;
		    rs = trs ;
		}
#if	CF_DEBUGN
		nprintf(NDF,"kshlib_runend: pt-join rs=%d tid=%u\n",
			rs,tid) ;
#endif
	        rs1 = kshlib_reqclose(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (kshlib_cmdsend) */
	} /* end if (running) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kshlib_runend) */


/* it always takes a good bit of code to make this part look easy! */
static int kshlib_worker(KSHLIB *uip)
{
	MSGINFO		m ;
	int		rs = SR_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_worker: ent\n") ;
#endif

	    while ((rs = kshlib_reqrecv(uip,&m)) > 0) {
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_worker: reqrecv mt=%u\n",rs) ;
#endif
	        switch (rs) {
	        case sesmsgtype_echo:
		    rs = kshlib_workecho(uip,&m) ;
		    break ;
	        case sesmsgtype_gen:
		    rs = kshlib_workgen(uip,&m) ;
		    break ;
	        case sesmsgtype_biff:
		    rs = kshlib_workbiff(uip,&m) ;
		    break ;
		default:
		    rs = kshlib_workdef(uip,&m) ;
		    break ;
	        } /* end switch */
	        if (rs < 0) break ;
	    } /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (kshlib_worker) */


static int kshlib_workecho(KSHLIB *uip,MSGINFO *mip)
{
	int		rs ;
	if ((rs = msginfo_conpass(mip,FALSE)) >= 0) {
	    rs = kshlib_reqsend(uip,mip,0) ;
	} /* end if (msginfo_conpass) */
	return rs ;
}
/* end subroutine (kshlib_workecho) */


static int kshlib_workgen(KSHLIB *uip,MSGINFO *mip)
{
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workgen: ent\n") ;
#endif
#if	CF_DEBUGS
	debugprintf("kshlib_workgen: ent\n") ;
#endif
#if	CF_DEBUGS && CF_DEBUGHEXB
	debugprinthexblock("kshlib_workgen: ",80,mip->mbuf,mip->mlen) ;
#endif
	if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	    if ((rs = kshlib_mq(uip)) >= 0) {
		SESMSG_GEN	m2 ;
		if ((rs = sesmsg_gen(&m2,1,mip->mbuf,mip->mlen)) >= 0) {
		    rs = kshlib_workgener(uip,&m2) ;
		} /* end if (sesmsg_gen) */
	    } /* end if (kshlib_mq) */
	    rs1 = kshlib_capend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (capture) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workgen: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_workgen) */


static int kshlib_workgener(KSHLIB *uip,SESMSG_GEN *mp)
{
	STORENOTE	*ep ;
	const int	esize = sizeof(STORENOTE) ;
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workgener: ent\n") ;
	nprintf(NDF,"kshlib_workgener: m=>%t<\n",
		mp->nbuf,strlinelen(mp->nbuf,-1,50)) ;
#endif
	if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	    time_t	st = mp->stime ;
	    const int	mt = mp->msgtype ;
	    const int	nlen = strlen(mp->nbuf) ;
	    cchar	*nbuf = mp->nbuf ;
	    cchar	*un = mp->user ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workgener: m=>%t<\n",
		nbuf,strlinelen(nbuf,nlen,50)) ;
#endif
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nlen)) >= 0) {
		rs = kshlib_msgenter(uip,ep) ;
	        if (rs < 0)
		    storenote_finish(ep) ;
	    } /* end if (storenote_start) */
	    if (rs < 0)
		uc_libfree(ep) ;
	} /* end if (m-a) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workgener: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_workgener) */


static int kshlib_workbiff(KSHLIB *uip,MSGINFO *mip)
{
	int		rs ;
	int		rs1 ;
	if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	    if ((rs = kshlib_mq(uip)) >= 0) {
		SESMSG_BIFF	m3 ;
		if ((rs = sesmsg_biff(&m3,1,mip->mbuf,mip->mlen)) >= 0) {
		    rs = kshlib_workbiffer(uip,&m3) ;
		} /* end if (sesmsg_biff) */
	    } /* end if (kshlib_mq) */
	    rs1 = kshlib_capend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (capture) */
	return rs ;
}
/* end subroutine (kshlib_workbiff) */


static int kshlib_workbiffer(KSHLIB *uip,SESMSG_BIFF *mp)
{
	STORENOTE	*ep ;
	const int	esize = sizeof(STORENOTE) ;
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_workbiffer: m=>%t<\n",
		mp->nbuf,strlinelen(mp->nbuf,-1,50)) ;
#endif
	if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	    time_t	st = mp->stime ;
	    const int	mt = mp->msgtype ;
	    const int	nlen = strlen(mp->nbuf) ;
	    cchar	*un = mp->user ;
	    cchar	*nbuf = mp->nbuf ;
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nlen)) >= 0) {
		rs = kshlib_msgenter(uip,ep) ;
	        if (rs < 0)
		    storenote_finish(ep) ;
	    } /* end if (storenote_start) */
	    if (rs < 0)
		uc_libfree(ep) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (kshlib_workbiffer) */


static int kshlib_workdef(KSHLIB *uip,MSGINFO *mip)
{
	int		rs ;
	if (mip == NULL) return SR_FAULT ;
	if ((rs = ptm_lock(&uip->m)) >= 0) {
	    uip->cdefs += 1 ;
	    ptm_unlock(&uip->m) ;
	} /* end if (mutex) */
	return rs ;
}
/* end subroutine (kshlib_workdef) */


static int kshlib_msgenter(KSHLIB *uip,STORENOTE *ep)
{
	RAQHAND		*qlp = &uip->mq ;
	const int	ors = SR_OVERFLOW ;
	int		rs ;
	if ((rs = raqhand_ins(qlp,ep)) == ors) {
	    void	*dum ;
	    if ((rs = raqhand_rem(qlp,&dum)) >= 0) {
		rs = raqhand_ins(qlp,ep) ;
	    }
	}
	return rs ;
}
/* end subroutine (kshlib_msgenter) */


static int kshlib_reqopen(KSHLIB *uip)
{
	int		rs ;
	cchar		*dname = KSHLIB_SESDNAME ;

	if ((rs = sdir(dname,(W_OK|X_OK))) >= 0) {
	    pid_t	sid = getsid(0) ;
	    char	sbuf[MAXPATHLEN+1] ;
	    if ((rs = mksdname(sbuf,dname,sid)) >= 0) {
		if (uip->reqfname == NULL) {
		    char	pbuf[MAXPATHLEN+1] ;
	            if ((rs = kshlib_mkreqfname(uip,pbuf,sbuf)) >= 0) {
			rs = kshlib_reqopener(uip,pbuf) ;
		    } /* end if (kshlib_mkreqfname) */
		} /* end if (reqfname) */
	    } /* end if (mksdname) */
	} /* end if (sdir) */

	return rs ;
}
/* end subroutine (kshlib_reqopen) */


static int kshlib_reqopener(KSHLIB *uip,cchar *pbuf)
{
	const mode_t	om = 0666 ;
	const int	lo = 0 ;
	int		rs ;
	if ((rs = listenusd(pbuf,om,lo)) >= 0) {
	    int	fd = rs ;
	    if ((rs = uc_closeonexec(fd,TRUE)) >= 0) {
		SOCKADDRESS	*sap = &uip->servaddr ;
		const int	af = AF_UNIX ;
		cchar		*rf = pbuf ;
		if ((rs = sockaddress_start(sap,af,rf,0,0)) >= 0) {
		    uip->servlen = rs ;
		    uip->sfd = fd ;
		}
	    }
	    if (rs < 0)
		u_close(fd) ;
	} /* end if (listenusd) */
	return rs ;
}
/* end subroutine (kshlib_reqopener) */


static int kshlib_reqclose(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqclose: ent sfd=%d\n",uip->sfd) ;
#endif

	if (uip->sfd >= 0) {
	    rs1 = u_close(uip->sfd) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->sfd = -1 ;
	    {
		SOCKADDRESS	*sap = &uip->servaddr ;
	        rs1 = sockaddress_finish(sap) ;
		if (rs >= 0) rs = rs1 ;
	    }
	    if (uip->reqfname != NULL) {
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqclose: reqfname{%p}=¿\n",uip->reqfname) ;
	nprintf(NDF,"kshlib_reqclose: reqfname=%s\n",uip->reqfname) ;
#endif
		if (uip->reqfname[0] != '\0') {
		    uc_unlink(uip->reqfname) ;
		}
		rs1 = uc_free(uip->reqfname) ;
		if (rs >= 0) rs = rs1 ;
		uip->reqfname = NULL ;
	    } /* end if (reqfname) */
	} /* end if (server-open) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kshlib_reqclose) */


static int kshlib_reqsend(KSHLIB *uip,MSGINFO *mip,int clen)
{
	const int	fd = uip->sfd ;
	return msginfo_sendmsg(mip,fd,clen) ;
}
/* end subroutine (kshlib_reqsend) */


static int kshlib_reqrecv(KSHLIB *uip,MSGINFO *mip)
{
	struct pollfd	fds[1] ;
	const int	fd = uip->sfd ;
	const int	mto = (5*POLLINTMULT) ;
	const int	nfds = 1 ;
	int		size ;
	int		rs ;
	int		rc = 0 ;

	size = (nfds * sizeof(struct pollfd)) ;
	memset(fds,0,size) ;
	fds[0].fd = fd ;
	fds[0].events = (POLLIN | POLLPRI | POLLERR) ;
	fds[0].revents = 0 ;

	while ((rs = u_poll(fds,nfds,mto)) >= 0) {
	    int	f = FALSE ;
	    if (rs > 0) {
		const int	re = fds[0].revents ;
		if (re & (POLLIN|POLLPRI)) {
		    if ((rs = msginfo_recvmsg(mip,fd)) >= 0) {
			f = TRUE ;
	    	        if (rs > 0) {
	        	    rc = MKCHAR(mip->mbuf[0]) ;
	    	        } else
	        	    rc = sesmsgtype_invalid ;
	            } /* end if (msginfo_recvmsg) */
		} else if (re & POLLERR) {
		    rs = SR_IO ;
		}
	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }
	    if (f) break ;
	    if (rs >= 0) {
		rs = kshlib_poll(uip) ;
	    }
	    if (rs < 0) break ;
	} /* end while (polling) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (kshlib_reqrecv) */


static int kshlib_poll(KSHLIB *uip)
{
	int		rs = SR_OK ;

	if (uip == NULL) return SR_FAULT ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_poll: ent\n") ;
#endif

	return rs ;
}
/* end subroutine (kshlib_poll) */


static int kshlib_cmdsend(KSHLIB *uip,int cmd)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_cmdsend: ent cmd=%u\n",cmd) ;
	nprintf(NDF,"kshlib_cmdsend: f_running=%u\n",uip->f_running) ;
#endif
	if (uip->f_running && (uip->reqfname != NULL)) {
	    f = TRUE ;
	    switch (cmd) {
	    case sesmsgtype_exit:
		{
	    	    MSGINFO	m ;
		    if ((rs = msginfo_init(&m)) >= 0) {
		        SESMSG_EXIT	m0 ;
			const int	mlen = MSGBUFLEN ;
			const int	sal = uip->servlen ;
			const void	*sap = &uip->servaddr ;
			msginfo_setaddr(&m,sap,sal) ;
			memset(&m0,0,sizeof(SESMSG_EXIT)) ;
		        if ((rs = sesmsg_exit(&m0,0,m.mbuf,mlen)) >= 0) {
			    m.mlen = rs ;
	    	            rs = kshlib_reqsend(uip,&m,0) ;
#if	CF_DEBUGN
			    nprintf(NDF,
				"kshlib_cmdsend: kshlib_reqsend() rs=%d\n",rs) ;
#endif
			} /* end if (sesmsg_exit) */
#if	CF_DEBUGN
			nprintf(NDF,
				"kshlib_cmdsend: sesmsg_exit-out rs=%d\n",rs) ;
#endif
		    } /* end if (init) */
		}
		break ;
	    default:
		rs = SR_INVALID ;
		break ;
	    } /* end switch */
	} /* end if (running) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_cmdsend: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_cmdsend) */


static void kshlib_atforkbefore()
{
	KSHLIB		*uip = &kshlib_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (kshlib_atforkbefore) */


static void kshlib_atforkafter()
{
	KSHLIB		*uip = &kshlib_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (kshlib_atforkafter) */


static void kshlib_sighand(int sn)
{
	KSHLIB		*kip = &kshlib_data ;
	switch (sn) {
	case SIGINT:
	    kip->f_sigintr = TRUE ;
	    break ;
	case SIGKILL:
	default:
	    kip->f_sigterm = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (kshlib_sighand) */


static int kshlib_mkreqfname(KSHLIB *uip,char *sbuf,cchar *dname)
{
	const uint	uv = (uint) uip->pid ;
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	char		dbuf[DIGBUFLEN+1] = { 'p' } ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_mkreqfname: ent pid=%u\n",uv) ;
#endif

	if ((rs = ctdecui((dbuf+1),(dlen-1),uv)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_mkreqfname: dbuf=%s\n",dbuf) ;
#endif
	    if ((rs = mkpath2(sbuf,dname,dbuf)) >= 0) {
#if	CF_DEBUGN
		nprintf(NDF,"kshlib_mkreqfname: mkpath2() rs=%d sbuf=%s\n",
		rs,sbuf) ;
#endif
		if (uip->reqfname == NULL) {
		    cchar	*cp ;
		    if ((rs = mallocstrw(sbuf,rs,&cp)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_mkreqfname: reqfname{%p}=¿\n",cp) ;
	nprintf(NDF,"kshlib_mkreqfname: reqfname{%p}=%s\n",cp,cp) ;
#endif
			uip->reqfname = cp ;
		    }
		}
	    } /* end if (mkpath) */
	} /* end if (ctdecui) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_mkreqfname: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_mkreqfname) */


static int kshlib_capbegin(KSHLIB *uip,int to)
{
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    uip->waiters += 1 ;

	    while ((rs >= 0) && uip->f_capture) { /* busy */
	        rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

	    if (rs >= 0) {
	        uip->f_capture = TRUE ;
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (kshlib_capbegin) */


static int kshlib_capend(KSHLIB *uip)
{
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lock(&uip->m)) >= 0) {

	    uip->f_capture = FALSE ;
	    if (uip->waiters > 0) {
	        rs = ptc_signal(&uip->c) ;
	    }

	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (kshlib_capend) */


static int kshlib_sigbegin(KSHLIB *kip)
{
	int		rs ;
	void		(*sh)(int) = kshlib_sighand ;
	kip->f_sigterm = 0 ;
	kip->f_sigintr = 0 ;
	rs = sigman_start(&kip->sm,sigblocks,sigigns,sigints,sh) ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_sigbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_sigbegin) */


static int kshlib_sigend(KSHLIB *kip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = sigman_finish(&kip->sm) ;
	if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_sigend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_sigend) */


static int storenote_start(ep,mt,st,un,mdp,mdl)
STORENOTE	*ep ;
int		mt ;
time_t		st ;
cchar		*un ;
cchar		*mdp ;
int		mdl ;
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	if (un == NULL) return SR_FAULT ;
	if (mdp == NULL) return SR_FAULT ;
	ep->stime = st ;
	ep->type = mt ;
	if (mdl < 0) mdl = strlen(mdp) ;
	size += (mdl+1) ;
	size += (strlen(un)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    ep->a = bp ;
	    ep->user = bp ;
	    bp = (strwcpy(bp,un,-1)+1) ;
	    ep->dbuf = bp ;
	    bp = (strwcpy(bp,mdp,mdl)+1) ;
	    ep->dlen = mdl ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (storenote_start) */


static int storenote_finish(STORENOTE *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (ep->a != NULL) {
	    rs1 = uc_libfree(ep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->a = NULL ;
	    ep->user = NULL ;
	    ep->dbuf = NULL ;
	    ep->dlen = 0 ;
	}
	ep->stime = 0 ;
	ep->type = 0 ;
	return rs ;
}
/* end subroutine (storenote_finish) */


static int mallocstrw(cchar *sp,int sl,cchar **rpp)
{
	int		rs ;
	char		*bp ;
	if (rpp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	if ((rs = uc_libmalloc((sl+1),&bp)) >= 0) {
	    *rpp = bp ;
	    strwcpy(bp,sp,sl) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (mallocstrw) */


static int sdir(cchar *dname,int am)
{
	struct ustat	sb ;
	const mode_t	dm = 0777 ;
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = uc_stat(dname,&sb)) == nrs) {
	    f = TRUE ;
	    rs = mksdir(dname,dm) ;
	} else {
	    rs = perm(dname,-1,-1,NULL,am) ;
	} /* end if (stat) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sdir) */


static int mksdir(cchar *dname,mode_t dm)
{
	int		rs ;
	if ((rs = mkdirs(dname,dm)) >= 0) {
	    rs = uc_minmod(dname,dm) ;
	}
	return rs ;
}
/* end if (mksdir) */


static int mksdname(char *rbuf,cchar *dname,pid_t sid)
{
	const uint	uv = (uint) sid ;
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	char		dbuf[DIGBUFLEN+1] = { 's' } ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib/mksdname: sid=%d\n",sid) ;
#endif

	if ((rs = ctdecui((dbuf+1),(dlen-1),uv)) >= 0) {
	    if ((rs = mkpath2(rbuf,dname,dbuf)) >= 0) {
		const mode_t	dm = 0777 ;
		if ((rs = mkdirs(rbuf,dm)) >= 0) {
		    rs = uc_minmod(rbuf,dm) ;
		}
	    } /* end if (mkpath) */
	} /* end if (ctdecui) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib/mksdname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mksdname) */


#if	CF_DEBUGENV && CF_DEBUGN
static int ndebugenv(cchar *s,cchar *ev[])
{
	cchar		*dfn = NDF ;
	cchar		*ep ;
	if (s != NULL) {
	    if (ev != NULL) {
	        int	i ;
		cchar	*fmt = "%s: e%03u=>%t<\n" ;
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


