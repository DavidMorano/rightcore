/* kshlib */
/* lang=C89 */

/* library initialization for KSH built-in command libraries */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGENV	0		/* debug environment */
#define	CF_DEBUGHEXB	0		/* debug w/ |debugprinthexblock()| */
#define	CF_PLUGIN	1		/* define 'plugin_version()' */
#define	CF_LOCKMEMALLOC	1		/* call |lockmemalloc(3uc)| */
#define	CF_KSHRUN	1		/* run background under KSH */
#define	CF_MQ		0		/* need |kshlib_mq()| */
#define	CF_LOCMALSTRW	0		/* use local |mallocstrw()| */


/* revision history:

	= 2001-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

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

	void lib_init(int flags,void *cxp)

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

	int lib_initenviron(void *cxp)

	Arguments:

	cxp		context pointer

	Returns:

	<0		error
	>=0		OK


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

	int lib_caller(func,argc,argv,envv,cxp)
	int		(*func)(int,cchar **,void *) ;
	int		argc ;
	cchar		*argv[] ;
	cchar		*envv[] ;
	void		*cxp ;

	Arguments:

	func		function to call
	argc		ARGC
	argv		ARGV
	envv		ENVV
	cxp		KSH context

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


	Notes:

	= Forked
        We get forked, screwed, turned, rotated, twisted, yanked and pulled --
        and probably a few other things. So we have to be very careful about
        knowing who we are (our PID) and if the state of our address space and
        threads are still valid.  This whole business is a real fork turner!

	= Aligned integer types?

	"aligned |int|s are already atomic"
        Well, yes, on almost every platform except for the old (original) DEC
        Alpha architecture.


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
#include	<lockmemalloc.h>
#include	<upt.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<sighand.h>
#include	<sockaddress.h>
#include	<raqhand.h>
#include	<char.h>
#include	<utmpacc.h>
#include	<tmtime.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sesmsg.h"
#include	"msgdata.h"
#include	"kshlib.h"


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
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

#define	VARKSHLIBRUN	"KSHLIB_RUNOPTS"

#define	NDF		"kshlib.deb"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	TO_LOCKENV	10

#define	KSHLIB		struct kshlib
#define	KSHLIB_FL	struct kshlib_flags
#define	KSHLIB_SCOPE	PTHREAD_SCOPE_PROCESS

#define	STORENOTE	struct storenote
#define	STORENOTE_FL	struct storenote_flags

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy2w(char *,int,cchar *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	siskipwhite(cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	listenusd(cchar *,mode_t,int) ;
extern int	rmsesfiles(cchar *) ;
extern int	isdirempty(cchar *) ;
extern int	msleep(int) ;
extern int	rmeol(cchar *,int) ;
extern int	iseol(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

#if	CF_LOCKMEMALLOC
extern int	lockmemalloc_set(int) ;
#endif

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
static int	nprintpid(cchar *s) ;
static int	nprintid(cchar *) ;
static int	nprintutmp(char *) ;
#endif /* CF_DEBUGN */

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


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

struct kshlib_flags {
	uint		notes:1 ;
	uint		mq:1 ;
	uint		initrun:1 ;	/* at initialization time */
} ;

struct kshlib {
	PTM		m ;		/* mutex data */
	PTM		menv ;		/* mutex environment */
	PTC		c ;		/* condition variable */
	SIGHAND		sm ;
	SOCKADDRESS	servaddr ;	/* server address */
	RAQHAND		mq ;		/* message queue */
	KSHLIB_FL	f, open ;
	cchar		*sesdname ;	/* session directory-name */
	cchar		*reqfname ;	/* request file-name */
	pid_t		sid ;		/* session ID */
	pid_t		pid ;		/* process ID */
	pthread_t	tid ;		/* worker thread */
	time_t		ti_sescheck ;
	volatile int	f_initonce ;	/* aligned |int|s are already atomic */
	volatile int	f_init ;	/* aligned |int|s are already atomic */
	volatile int	f_initdone ;	/* aligned |int|s are already atomic */
	volatile int	f_running ;	/* aligned |int|s are already atomic */
	volatile int	f_capture ;	/* aligned |int|s are already atomic */
	volatile int	f_exiting ;	/* aligned |int|s are already atomic */
	volatile int	f_autorun ;	/* aligned |int|s are already atomic */
	volatile int	waiters ;	/* aligned |int|s are already atomic */
	sig_atomic_t	f_sigquit ;
	sig_atomic_t	f_sigterm ;
	sig_atomic_t	f_sigintr ;
	sig_atomic_t	f_sigwich ;
	sig_atomic_t	f_sigchild ;
	sig_atomic_t	f_sigsusp ;
	int		intpoll ;
	int		intsescheck ;
	int		seshour ;
	int		runmode ;
	int		serial ;
	int		sfd ;
	int		cdefs ;		/* defualt count */
	int		servlen ;	/* serv-addr length */
	int		pollcount ;
} ;

struct storenote_flags {
	uint		displayed:1 ;	/* displayed by KSH itself */
	uint		read:1 ;	/* marked as read by comment */
} ;

struct storenote {
	STORENOTE_FL	f ;
	time_t		stime ;
	cchar		*dbuf ;
	cchar		*user ;
	char		*a ;
	int		type ;
	int		dlen ;
} ;


/* forward references */

int		lib_initenviron(void *) ;
int		lib_callcmd(cchar *,int,cchar **,cchar **,void *) ;
int 		lib_callfunc(subcmd_t,int,cchar **,cchar **,void *) ;

static int	kshlib_init(void) ;
static void	kshlib_fini(void) ;

static void	kshlib_atforkbefore() ;
static void	kshlib_atforkparent() ;
static void	kshlib_atforkchild() ;
static void	kshlib_sighand(int,siginfo_t *,void *) ;

static int	kshlib_begin(KSHLIB *) ;
static int	kshlib_end(KSHLIB *) ;

static int	kshlib_autorun(KSHLIB *,cchar **) ;
static int	kshlib_autorunopt(KSHLIB *,cchar *,int) ;
static int	kshlib_autorunoptnotes(KSHLIB *,cchar *,int,int) ;
static int	kshlib_autorunopter(KSHLIB *) ;

static int	kshlib_runbegin(KSHLIB *) ;
static int	kshlib_runner(KSHLIB *) ;
static int	kshlib_runend(KSHLIB *) ;

static int	kshlib_sid(KSHLIB *) ;
static int	kshlib_sesdname(KSHLIB *) ;
static int	kshlib_reqfname(KSHLIB *) ;
static int	kshlib_worker(KSHLIB *) ;
static int	kshlib_worknoop(KSHLIB *,MSGDATA *) ;
static int	kshlib_workecho(KSHLIB *,MSGDATA *) ;
static int	kshlib_workbiff(KSHLIB *,MSGDATA *) ;
static int	kshlib_workbiffer(KSHLIB *,SESMSG_BIFF *) ;
static int	kshlib_workgen(KSHLIB *,MSGDATA *) ;
static int	kshlib_workgener(KSHLIB *,SESMSG_GEN *) ;
static int	kshlib_workdef(KSHLIB *,MSGDATA *) ;

static int	kshlib_msgenter(KSHLIB *,STORENOTE *) ;
static int	kshlib_reqopen(KSHLIB *) ;
static int	kshlib_reqopener(KSHLIB *) ;
static int	kshlib_reqsend(KSHLIB *,MSGDATA *,int,int) ;
static int	kshlib_reqrecv(KSHLIB *,MSGDATA *) ;
static int	kshlib_reqclose(KSHLIB *) ;
static int	kshlib_poll(KSHLIB *) ;
static int	kshlib_cmdsend(KSHLIB *,int) ;
static int	kshlib_capbegin(KSHLIB *,int) ;
static int	kshlib_capend(KSHLIB *) ;
static int	kshlib_sigbegin(KSHLIB *,const int *) ;
static int	kshlib_sigend(KSHLIB *) ;

static int	kshlib_notesbegin(KSHLIB *) ;
static int	kshlib_notesend(KSHLIB *) ;
static int	kshlib_notesactive(KSHLIB *) ;
static int	kshlib_notescount(KSHLIB *) ;

static int	kshlib_mqbegin(KSHLIB *) ;
static int	kshlib_mqend(KSHLIB *) ;
static int	kshlib_mqfins(KSHLIB *) ;
static int	kshlib_mqactive(KSHLIB *) ;
static int	kshlib_mqcount(KSHLIB *) ;

static int	kshlib_sesend(KSHLIB *) ;

#if	CF_MQ
static int	kshlib_mq(KSHLIB *) ;
#endif

int		lib_initmemalloc(int) ;

static int	storenote_start(STORENOTE *,int,time_t,cchar *,cchar *,int) ;
static int	storenote_finish(STORENOTE *) ;

static int	sdir(cchar *,int) ;
static int	mksdir(cchar *,mode_t) ;
static int	mksdname(char *,cchar *,pid_t) ;

#if	CF_LOCMALSTRW
static int	mallocstrw(cchar *,int,cchar **) ;
#endif /* CF_LOCMALSTRW */


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
	0
} ;

static const int	sigigns[] = {
	SIGHUP,
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGQUIT,
	SIGTERM,
	SIGINT,
	SIGWINCH,
	SIGCHLD,
	SIGTSTP,
	0
} ;

enum runopts {
	runopt_notes,
	runopt_lognotes,
	runopt_overlast
} ;

static cchar	*runopts[] = {
	"notes",
	"lognotes",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
void lib_init(int flags,void *cxp)
{
	KSHLIB		*uip = &kshlib_data ;
	if (! uip->f_initonce) {
	    uip->f_initonce = TRUE ;

#if	CF_DEBUGN
	{
	    const uint	pid = getpid() ;
	    const uint	ppid = getppid() ;
	    nprintf(NDF,"lib_init: ent pid=%u ppid=%u\n",pid,ppid) ;
	}
#endif

#if	CF_DEBUGENV && CF_DEBUGN
	{
	    const pid_t	pid = getpid() ;
	    void 	*p = dlsym(RTLD_DEFAULT,"environ") ;
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
	    nprintf(NDF,"lib_init: cxp=%P\n",cxp) ;
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
	    if ((rs = lib_initenviron(cxp)) >= 0) {
	        if ((rs = kshlib_init()) >= 0) {
	            KSHLIB	*uip = &kshlib_data ;
	            cchar	**envv = (cchar **) environ ;
	            rs = kshlib_autorun(uip,envv) ;
	        } /* end if (init) */
	    } /* end if (lib_initenviron) */
#if	CF_DEBUGN
	    nprintf(NDF,"lib_init: KSHRUN rs=%d\n",rs) ;
#endif
	}
#endif /* CF_KSHRUN */

#if	CF_DEBUGN
	{
	    const uint	pid = getpid() ;
	    nprintf(NDF,"lib_init: ret pid=%u\n",pid) ;
	}
#endif

	} else {
#if	CF_DEBUGN
	    const uint	pid = getpid() ;
	    nprintf(NDF,"lib_init: REPEAT pid=%u\n",pid) ;
#endif
	} /* end if (init-once) */
}
/* end subroutine (lib_init) */


void lib_fini(void)
{
	kshlib_fini() ;
}
/* end subroutine (lib_fini) */


/* is this multi-thread safe or not? */
/* ARGSUSED */
int lib_initenviron(void *cxp)
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


int lib_mainbegin(cchar **envv,const int *catches)
{
	int		rs ;

#if	CF_DEBUGN
	{
	    const uint	pid = getpid() ;
	    nprintf(NDF,"lib_mainbegin: ent pid=%u\n",pid) ;
	}
#endif

	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_sigbegin(uip,catches)) >= 0) {
	        rs = kshlib_autorun(uip,envv) ;
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
	{
	    const uint	pid = getpid() ;
	    nprintf(NDF,"lib_mainend: ent pid=%u\n",pid) ;
	    nprintf(NDF,"lib_mainend: f_running=%u\n",uip->f_running) ;
	}
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


int lib_kshbegin(void *cxp,const int *catches)
{
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_kshbegin: ent\n") ;
	nprintpid("lib_kshbegin") ;
#endif
	if ((rs = lib_initenviron(cxp)) >= 0) {
	    if ((rs = kshlib_init()) >= 0) {
	        KSHLIB	*kip = &kshlib_data ;
	        if ((rs = kshlib_sigbegin(kip,catches)) >= 0) {
	            kip->runmode |= KSHLIB_RMKSH ;
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
	rs1 = sighand_finish(&kip->sm) ;
	if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_kshbegin: sighand_finish() rs=%d\n",rs) ;
#endif
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


int lib_sigreset(int sn)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	switch (sn) {
	case SIGQUIT:
	    kip->f_sigquit = 0 ;
	    break ;
	case SIGTERM:
	    kip->f_sigterm = 0 ;
	    break ;
	case SIGINT:
	    kip->f_sigintr = 0 ;
	    break ;
	case SIGWINCH:
	    kip->f_sigwich = 0 ;
	    break ;
	case SIGCHLD:
	    kip->f_sigchild = 0 ;
	    break ;
	case SIGTSTP:
	    kip->f_sigsusp = 0 ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (lib_sigreset) */


int lib_sigquit(void)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	if (kip->f_sigquit) {
	    kip->f_sigquit = 0 ;
	    rs = SR_QUIT ;
	}
	return rs ;
}
/* end subroutine (lib_sigquit) */


int lib_sigterm(void)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	if (kip->f_sigterm) {
	    kip->f_sigterm = 0 ;
	    rs = SR_EXIT ;
	}
	return rs ;
}
/* end subroutine (lib_sigterm) */


int lib_sigintr(void)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	if (kip->f_sigintr) {
	    kip->f_sigintr = 0 ;
	    rs = SR_INTR ;
	}
	return rs ;
}
/* end subroutine (lib_sigintr) */


int lib_issig(int sn)
{
	KSHLIB		*kip = &kshlib_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	switch (sn) {
	case SIGQUIT:
	    f = kip->f_sigquit ;
	    if (f) kip->f_sigquit = 0 ;
	    break ;
	case SIGTERM:
	    f = kip->f_sigterm ;
	    if (f) kip->f_sigterm = 0 ;
	    break ;
	case SIGINT:
	    f = kip->f_sigintr ;
	    if (f) kip->f_sigintr = 0 ;
	    break ;
	case SIGWINCH:
	    f = kip->f_sigwich ;
	    if (f) kip->f_sigwich = 0 ;
	    break ;
	case SIGCHLD:
	    f = kip->f_sigchild ;
	    if (f) kip->f_sigchild = 0 ;
	    break ;
	case SIGTSTP:
	    f = kip->f_sigsusp ;
	    if (f) kip->f_sigsusp = 0 ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (lib_issig) */


int lib_initmemalloc(int f)
{
	int		rs = SR_OK ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_initmemalloc: ent f=%u\n",f) ;
#endif
	if (f) {
	    cchar	*sym = "lockmemalloc_set" ;
	    void	*sop = RTLD_SELF ;
	    void	*p ;
	    if ((p = dlsym(sop,sym)) != NULL) {
	        int	(*fun)(int) = (int (*)(int)) p ;
	        rs = (*fun)(lockmemallocset_begin) ;
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
	        } else {
	            rs = SR_NOENT ;
		}
	    } /* end if (sncpy) */
	} else {
	    rs = SR_NOENT ;
	}
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


/* ARGSUSED */
int lib_progcall(cchar *name,int argc,cchar **argv,cchar **envv,void *cxp)
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
	        } else {
	            ex = EX_OSERR ;
		}
	    } else {
	        ex = EX_OSERR ;
	    }
	} else {
	    ex = EX_NOPROG ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"lib_progcall: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_progcall) */


int lib_progcalla(const void *func,int argc,cchar **argv,cchar **envv,void *cxp)
{
	subcmd_t	f = (subcmd_t) func ;
	return lib_callfunc(f,argc,argv,envv,cxp) ;
}
/* end subroutine (lib_progcalla) */


/* ARGSUSED */
int lib_caller(const void *fa,int argc,cchar **argv,cchar **envv,void *cxp)
{
	func_caller	func = (func_caller) fa ;
	int		rs ;
	int		ex = EX_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_caller: &environ=%p\n",&environ) ;
	nprintf(NDF,"lib_caller: environ=%p\n",environ) ;
	nprintf(NDF,"lib_caller: envv=%p\n",envv) ;
#endif

#if	CF_DEBUGENV && CF_DEBUGN
	if (envv != NULL)
	    ndebugenv("lib_caller",envv) ;
#endif

	if ((rs = lib_initenviron(cxp)) >= 0) {

#if	CF_DEBUGN
	    nprintf(NDF,"lib_caller: func()\n") ;
#endif

	    if (func != NULL) {
	        ex = (*func)(argc,argv,cxp) ;
	    } else {
	        ex = EX_NOPROG ;
	    }

#if	CF_DEBUGN
	    nprintf(NDF,"lib_caller: func() ex=%u\n",ex) ;
#endif

	} /* end if (lib_initenviron) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_caller: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_caller) */


int lib_callfunc(subcmd_t func,int argc,cchar **argv,cchar **envv,void *cxp)
{
	int		rs ;
	int		ex = EX_OK ;

	if ((rs = lib_initenviron(cxp)) >= 0) {
	    if ((rs = kshlib_init()) >= 0) {
	        if (func != NULL) {
	            ex = (*func)(argc,argv,envv,cxp) ;
	        } else {
	            ex = EX_NOPROG ;
		}
	    } else {
	        ex = EX_OSERR ;
	    }
	} /* end if (lib_initenviron) */
	if ((rs < 0) && (ex == EX_OK)) ex = EX_MUTEX ;

	return ex ;
}
/* end subroutine (lib_callfunc) */


int lib_callcmd(cchar *name,int argc,cchar **argv,cchar **envv,void *cxp)
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
#if	CF_DEBUGN
		nprintf(NDF,"lib_callcmd: srch-sym=%s\n",symname) ;
#endif
	        if ((p = dlsym(sop,symname)) != NULL) {
	            int (*cf)(int,cchar **,cchar **,void *) ;
	            cf = (int (*)(int,cchar **,cchar **,void *)) p ;
#if	CF_DEBUGN
		    nprintf(NDF,"lib_callcmd: call-before\n") ;
#endif
	            ex = (*cf)(argc,argv,envv,cxp) ;
#if	CF_DEBUGN
		    nprintf(NDF,"lib_callcmd: call-after ex=%u\n",ex) ;
#endif
	        } else {
#if	CF_DEBUGN
		    nprintf(NDF,"lib_callcmd: unavailable\n") ;
#endif
	            ex = EX_UNAVAILABLE ;
		}
	    } else {
	        ex = EX_NOPROG ;
	    }
	} else {
	    ex = EX_NOPROG ;
	}
	if ((rs < 0) && (ex == EX_OK)) ex = EX_OSERR ;

#if	CF_DEBUGN
	nprintf(NDF,"lib_callcmd: ret ex=%u (%d)\n",ex,rs) ;
#endif

	return ex ;
}
/* end subroutine (lib_callcmd) */


int lib_noteadm(int cmd,...)
{
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteadm: ent cmd=%u\n",cmd) ;
#endif
	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_begin(uip)) >= 0) {
	        if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	            switch (cmd) {
	            case kshlibcmd_noteoff:
	                rs = kshlib_notesend(uip) ;
	                rv = rs ;
	                break ;
	            case kshlibcmd_noteon:
	                rs = kshlib_notesbegin(uip) ;
	                rv = rs ;
	                break ;
	            case kshlibcmd_notecount:
	                rs = kshlib_notescount(uip) ;
	                rv = rs ;
	                break ;
	            case kshlibcmd_notestate:
	                rs = kshlib_notesactive(uip) ;
			rv = rs ;
	                break ;
	            } /* end switch */
	            rs1 = kshlib_capend(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (capture) */
	    } /* end if (kshlib_begin) */
	} /* end if (kshlib_init) */
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteadm: ret rs=%d rv=%u\n",rs,rv) ;
#endif
	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (lib_noteadm) */


int lib_noteread(KSHLIB_NOTE *rp,int mi)
{
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	if (rp == NULL) return SR_FAULT ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: ent mi=%u\n",mi) ;
#endif
	memset(rp,0,sizeof(KSHLIB_NOTE)) ;
	if (mi < 0) return SR_INVALID ;
	if ((rs = kshlib_init()) >= 0) {
	    KSHLIB	*uip = &kshlib_data ;
	    if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	        if ((rs = kshlib_mqactive(uip)) > 0) {
	            STORENOTE	*ep ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: raqhand_acc() mi=%u\n",mi) ;
#endif
	            if ((rs = raqhand_acc(&uip->mq,mi,&ep)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: raqhand_acc() rs=%d ep{%p}\n",rs,ep) ;
#endif
	                if (ep != NULL) {
	                    rp->stime = ep->stime ;
	                    rp->type = ep->type ;
	                    rp->nlen = ep->dlen ;
	                    strwcpy(rp->user,ep->user,SESMSG_USERLEN) ;
	                    strwcpy(rp->nbuf,ep->dbuf,SESMSG_NBUFLEN) ;
	                    rc = 1 ;
	                } /* end if (non-null) */
	            } else if (rs == SR_NOTFOUND) {
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: raqhand_acc() rs=NOTFOUND\n") ;
#endif
	                rs = SR_OK ;
	            } /* end if (raqhand_acc) */
	        } /* end if (kshlib_mqactive) */
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: _mqactive-out rs=%d\n",rs) ;
#endif
	        rs1 = kshlib_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: _capend() rs=%d\n",rs) ;
#endif
	    } /* end if (capture) */
	} /* end if (kshlib_init) */
#if	CF_DEBUGN
	nprintf(NDF,"lib_noteread: ret rs=%d rc=%u\n",rs,rc) ;
#endif
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
	        if ((rs = kshlib_mqactive(uip)) > 0) {
	            rs = raqhand_del(&uip->mq,ni) ;
	            rc = rs ;
	        } /* end if (kshlib_mqactive) */
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
	int		rs = SR_OK ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_init: ent f_init=%u\n",uip->f_init) ;
#endif
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	            void	(*b)() = kshlib_atforkbefore ;
	            void	(*ap)() = kshlib_atforkparent ;
	            void	(*ac)() = kshlib_atforkchild ;
	            if ((rs = uc_atfork(b,ap,ac)) >= 0) {
	                if ((rs = uc_atexit(kshlib_fini)) >= 0) {
	                    uip->pid = getpid() ;
	                    uip->sfd = -1 ;
	                    rs = 1 ;
	                    uip->f_initdone = TRUE ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_init: done pid=%d\n",uip->pid) ;
#endif
	                }
	                if (rs < 0)
	                    uc_atforkrelease(b,ap,ac) ;
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
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_init: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_init) */


static void kshlib_fini(void)
{
	struct kshlib	*uip = &kshlib_data ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_fini: ent\n") ;
#endif
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        kshlib_runend(uip) ;
	        kshlib_end(uip) ;
	    }
	    {
	        void	(*b)() = kshlib_atforkbefore ;
	        void	(*ap)() = kshlib_atforkparent ;
	        void	(*ac)() = kshlib_atforkchild ;
	        uc_atforkrelease(b,ap,ac) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct kshlib)) ;
	} /* end if (atexit registered) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_fini: ret\n") ;
#endif
}
/* end subroutine (kshlib_fini) */


static int kshlib_begin(KSHLIB *uip)
{
	if (uip == NULL) return SR_FAULT ;
	uip->ti_sescheck = 0 ;
	uip->intpoll = KSHLIB_INTPOLL ;
	uip->intsescheck = KSHLIB_INTSESCHECK ;
	uip->seshour = KSHLIB_SESHOUR ;
	return SR_OK ;
}
/* end subroutine (kshlib_begin) */


static int kshlib_end(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = kshlib_notesend(uip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = kshlib_mqend(uip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = kshlib_sesend(uip) ;
	if (rs >= 0) rs = rs1 ;

	if (uip->sesdname != NULL) {
	    rs1 = uc_libfree(uip->sesdname) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->sesdname = NULL ;
	}

	return rs ;
}
/* end subroutine (kshlib_end) */


static int kshlib_autorun(KSHLIB *uip,cchar **envv)
{
	int		rs = SR_OK ;
	int		c = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorun: ent\n") ;
#endif
	if (! uip->f_autorun) {
	    cchar	*vp ;
	    uip->f_autorun = TRUE ;
	    if ((vp = getourenv(envv,VARKSHLIBRUN)) != NULL) {
		cchar	*tp ;
		while ((tp = strpbrk(vp," ,:")) != NULL) {
		    if ((tp-vp) > 0) {
		        rs = kshlib_autorunopt(uip,vp,(tp-vp)) ;
			c += 1 ;
		    }
		    vp = (tp+1) ;
		    if (rs < 0) break ;
		} /* end while */
		if ((rs >= 0) && (vp[0] != '\0')) {
		    rs = kshlib_autorunopt(uip,vp,-1) ;
		    c += rs ;
		}
	    } /* end if (get-env) */
	} /* end if (needed auto-run check) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorun: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kshlib_autorun) */


static int kshlib_autorunopt(KSHLIB *uip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si ;
	int		oi ;
	int		vl = 0 ;
	int		c = 0 ;
	cchar		*vp = NULL ;
	cchar		*tp ;
	if ((si = siskipwhite(sp,sl)) > 0) {
	    sp += si ;
	    sl -= si ;
	}
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    vl = sfshrink((tp+1),((sp+sl)-(tp+1)),&vp) ;
	    sl = (tp-sp) ;
	    while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;
	}
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorunopt: opt=%t\n",sp,sl) ;
#endif
	if ((oi = matostr(runopts,2,sp,sl)) >= 0) {
	    switch (oi) {
	    case runopt_notes:
		rs = kshlib_autorunoptnotes(uip,vp,vl,FALSE) ;
		c += rs ;
		break ;
	    case runopt_lognotes:
		rs = kshlib_autorunoptnotes(uip,vp,vl,TRUE) ;
		c += rs ;
		break ;
	    } /* end switch */
	} /* end if (match) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kshlib_autorunopt) */


/* ARGSUSED */
static int kshlib_autorunoptnotes(KSHLIB *uip,cchar *vp,int vl,int f)
{
	int		rs = SR_OK ;
	int		rv = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorunoptnotes: ent f=%u\n",f) ;
	nprintutmp("kshlib_autorunoptnotes") ;
#endif
	if (! uip->f.initrun) {
	    int		f_go = TRUE ;
	    if (f) {
		if ((rs = kshlib_sid(uip)) >= 0) {
		    UTMPACC_ENT	ue ;
		    const pid_t	pid = uip->pid ;
		    const int	rsn = SR_NOTFOUND ;
		    const int	ulen = UTMPACC_BUFLEN ;
		    char	ubuf[UTMPACC_BUFLEN+1] ;
		    if (uip->sid == pid) {
		        if ((rs = utmpacc_entsid(&ue,ubuf,ulen,pid)) == rsn) {
		            rs = SR_OK ;
		            f_go = FALSE ;
		        }
		    } else {
		        f_go = FALSE ;
		    }
		} /* end if (kshlib_sid) */
	    }
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorunoptnotes: mid rs=%d f_go=%u\n",rs,f_go) ;
#endif
	    if ((rs >= 0) && f_go) {
	        uip->f.initrun = TRUE ;
		rs = kshlib_autorunopter(uip) ;
		rv = rs ;
#if	CF_DEBUGN
		nprintf(NDF,"kshlib_autorunoptnotes: _autorunopter() rs=%d\n",
			rs) ;
#endif
	    }
	} /* end if (need check) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_autorunoptnotes: ret rs=%d rv=%u\n",rs,rv) ;
#endif
	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (kshlib_autorunoptnotes) */


static int kshlib_autorunopter(KSHLIB *uip)
{
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;
	if ((rs = kshlib_begin(uip)) >= 0) {
	    if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
		if ((rs = kshlib_notesbegin(uip)) >= 0) {
		    rv = TRUE ;
		}
	        rs1 = kshlib_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (kshlib_begin) */
	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (kshlib_autorunopter) */


static int kshlib_runbegin(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runbegin: ent f_running=%u\n",uip->f_running) ;
#endif

	if (! uip->f_running) {
	    if ((rs = kshlib_reqopen(uip)) >= 0) {
		rs = kshlib_runner(uip) ;
		f = rs ;
	    } /* end if (kshlib_reqopen) */
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_runbegin) */


static int kshlib_runner(KSHLIB *uip)
{
	PTA		ta ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_runner: ent\n") ;
#endif

	if ((rs = pta_create(&ta)) >= 0) {
	    const int	scope = KSHLIB_SCOPE ;
	    if ((rs = pta_setscope(&ta,scope)) >= 0) {
	        pthread_t	tid ;
	        tworker		wt = (tworker) kshlib_worker ;
	        if ((rs = uptcreate(&tid,&ta,wt,uip)) >= 0) {
	            uip->f_running = TRUE ;
	            uip->tid = tid ;
	            f = TRUE ;
	        } /* end if (pthread-create) */
#if	CF_DEBUGN
	        nprintf(NDF,"kshlib_runner: pt-create rs=%d tid=%u\n",
	            rs,tid) ;
#endif
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&ta) ;
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
	nprintf(NDF,"kshlib_runend: ent running=%u\n",uip->f_running) ;
#endif

	if (uip->f_running) {
	    const pid_t		pid = getpid() ;
	    if (pid == uip->pid) {
	        const int	cmd = sesmsgtype_exit ;
	        if ((rs = kshlib_cmdsend(uip,cmd)) >= 0) {
	            pthread_t	tid = uip->tid ;
	            int		trs ;
#if	CF_DEBUGN
		    nprintpid("kshlib_runend") ;
		    nprintid("kshlib_runend") ;
	            nprintf(NDF,"kshlib_runend: pt-join tid=%u\n",tid) ;
#endif
	            if ((rs = uptjoin(tid,&trs)) >= 0) {
	                uip->f_running = FALSE ;
	                rs = trs ;
	            } else if (rs == SR_SRCH) { /* should never happen */
#if	CF_DEBUGN
		    nprintpid("kshlib_runend") ;
		    nprintid("kshlib_runend") ;
	            nprintf(NDF,"kshlib_runend: SRCH uptjoin(%u) rs=%d\n",
		        tid,rs) ;
#endif
	                uip->f_running = FALSE ;
		        rs = SR_OK ;
		    }
#if	CF_DEBUGN
		    nprintid("kshlib_runend") ;
	            nprintf(NDF,"kshlib_runend: pt-join tid=%u rs=%d\n",
	                tid,rs) ;
#endif
	            rs1 = kshlib_reqclose(uip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (kshlib_cmdsend) */
	    } else {
		uip->f_running = FALSE ;
		uip->f_exiting = FALSE ;
		uip->pid = pid ;
	    }
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
	MSGDATA		m ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGN
	{
	    const uint	tid = uptself(NULL) ;
	    nprintf(NDF,"kshlib_worker: ent tid=%u\n",tid) ;
	}
#endif

	if ((rs = msgdata_init(&m,0)) >= 0) {
	    while ((rs = kshlib_reqrecv(uip,&m)) > 0) {
	        int	f_exit = FALSE ;
#if	CF_DEBUGN
	        nprintf(NDF,"kshlib_worker: reqrecv mt=%u\n",rs) ;
#endif
	        switch (rs) {
	        case sesmsgtype_exit:
	            f_exit = TRUE ;
	            break ;
	        case sesmsgtype_noop:
	            rs = kshlib_worknoop(uip,&m) ;
	            break ;
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
	        if (f_exit) break ;
	        if (rs < 0) break ;
	    } /* end while (looping on commands) */
	    rs1 = msgdata_fini(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msgdata) */

#if	CF_DEBUGN
	nprintid("kshlib_worker") ;
	nprintf(NDF,"kshlib_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (kshlib_worker) */


static int kshlib_worknoop(KSHLIB *uip,MSGDATA *mip)
{
	int		rs ;
	if ((rs = msgdata_conpass(mip,FALSE)) >= 0) {
	    rs = kshlib_reqsend(uip,mip,-1,0) ;
	} /* end if (msgdata_conpass) */
	return rs ;
}
/* end subroutine (kshlib_worknoop) */


static int kshlib_workecho(KSHLIB *uip,MSGDATA *mip)
{
	int		rs ;
	if ((rs = msgdata_conpass(mip,FALSE)) >= 0) {
	    rs = kshlib_reqsend(uip,mip,-1,0) ;
	} /* end if (msgdata_conpass) */
	return rs ;
}
/* end subroutine (kshlib_workecho) */


static int kshlib_workgen(KSHLIB *uip,MSGDATA *mip)
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
	debugprinthexblock("kshlib_workgen: ",80,mip->mbuf,mip->ml) ;
#endif
	if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	    if ((rs = kshlib_notesactive(uip)) > 0) {
	        SESMSG_GEN	m2 ;
	        if ((rs = sesmsg_gen(&m2,1,mip->mbuf,mip->ml)) >= 0) {
	            rs = kshlib_workgener(uip,&m2) ;
	        } /* end if (sesmsg_gen) */
	    } /* end if (kshlib_notesactive) */
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
	    int		nl = rmeol(mp->nbuf,-1) ;
	    cchar	*nbuf = mp->nbuf ;
	    cchar	*un = mp->user ;
#if	CF_DEBUGN
	    {
		char	tbuf[TIMEBUFLEN+1] ;
	        nprintf(NDF,"kshlib_workgener: m=>%t<\n",
	            nbuf,strlinelen(nbuf,nl,50)) ;
	        timestr_logz(st,tbuf) ;
	        nprintf(NDF,"kshlib_workgener: t=%s\n",tbuf) ;
	    }
#endif /* CF_DEBUGN */
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nl)) >= 0) {
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


static int kshlib_workbiff(KSHLIB *uip,MSGDATA *mip)
{
	int		rs ;
	int		rs1 ;
	if ((rs = kshlib_capbegin(uip,-1)) >= 0) {
	    if ((rs = kshlib_notesactive(uip)) > 0) {
	        SESMSG_BIFF	m3 ;
	        if ((rs = sesmsg_biff(&m3,1,mip->mbuf,mip->ml)) >= 0) {
	            rs = kshlib_workbiffer(uip,&m3) ;
	        } /* end if (sesmsg_biff) */
	    } /* end if (kshlib_notesactive) */
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
	    int		nl = rmeol(mp->nbuf,-1) ;
	    cchar	*un = mp->user ;
	    cchar	*nbuf = mp->nbuf ;
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nl)) >= 0) {
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


static int kshlib_workdef(KSHLIB *uip,MSGDATA *mip)
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
	const int	rso = SR_OVERFLOW ;
	int		rs ;
	if ((rs = raqhand_ins(qlp,ep)) == rso) {
	    void	*dum ;
	    if ((rs = raqhand_rem(qlp,&dum)) >= 0) {
	        rs = raqhand_ins(qlp,ep) ;
	    }
	}
	return rs ;
}
/* end subroutine (kshlib_msgenter) */


static int kshlib_sid(KSHLIB *uip)
{
	if (uip->sid == 0) uip->sid = getsid(0) ;
	return SR_OK ;
}
/* end subroutine (kshlib_sid) */


static int kshlib_sesdname(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		pl = 0 ;
	if (uip->sesdname == NULL) {
	    cchar	*dname = KSHLIB_SESDNAME ;
	    if ((rs = sdir(dname,(W_OK|X_OK))) >= 0) {
		if ((rs = kshlib_sid(uip)) >= 0) {
	           char		pbuf[MAXPATHLEN+1] ;
	           if ((rs = mksdname(pbuf,dname,uip->sid)) >= 0) {
			cchar	*cp ;
			pl = rs ;
	                if ((rs = uc_libmallocstrw(pbuf,pl,&cp)) >= 0) {
			    uip->sesdname = cp ;
			}
	    	    } /* end if (mksdname) */
		} /* end if (kshlib_sid) */
	    } /* end if (sdir) */
	} else {
	    pl = strlen(uip->sesdname) ;
	} /* end if (needed) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (kshlib_sesdname) */


static int kshlib_reqfname(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		pl = 0 ;
	if (uip->reqfname == NULL) {
	    if ((rs = kshlib_sesdname(uip)) >= 0) {
	        const uint	uv = (uint) uip->pid ;
	        const int	dlen = DIGBUFLEN ;
	        char		dbuf[DIGBUFLEN+1] = { 'p' } ;
	        if ((rs = ctdecui((dbuf+1),(dlen-1),uv)) >= 0) {
		    cchar	*sesdname = uip->sesdname ;
		    char	pbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(pbuf,sesdname,dbuf)) >= 0) {
	                cchar	*cp ;
		        pl = rs ;
	                if ((rs = uc_libmallocstrw(pbuf,pl,&cp)) >= 0) {
	                    uip->reqfname = cp ;
	                }
	            } /* end if (mkpath) */
		} /* end if (ctdecui) */
	    } /* end if (kshlib_sesdname) */
	} else {
	    pl = strlen(uip->sesdname) ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqfname: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (kshlib_reqfname) */


static int kshlib_reqopen(KSHLIB *uip)
{
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqopen: ent\n") ;
#endif
	if ((rs = kshlib_reqfname(uip)) >= 0) {
	    rs = kshlib_reqopener(uip) ;
	} /* end if (kshlib_reqfname) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqopen: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_reqopen) */


static int kshlib_reqopener(KSHLIB *uip)
{
	const mode_t	om = 0666 ;
	const int	lo = 0 ; /* listen options */
	int		rs ;
	cchar		*req = uip->reqfname ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqopener: ent req=%s\n",req) ;
#endif
	if ((rs = listenusd(req,om,lo)) >= 0) {
	    const int	fd = rs ;
	    if ((rs = uc_closeonexec(fd,TRUE)) >= 0) {
	        SOCKADDRESS	*sap = &uip->servaddr ;
	        const int	af = AF_UNIX ;
	        if ((rs = sockaddress_start(sap,af,req,0,0)) >= 0) {
	            uip->servlen = rs ;
	            uip->sfd = fd ;
	        }
	    }
	    if (rs < 0)
	        u_close(fd) ;
	} /* end if (listenusd) */
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqopener: ret rs=%d\n",rs) ;
#endif
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
	            u_unlink(uip->reqfname) ;
	        }
	        rs1 = uc_libfree(uip->reqfname) ;
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


static int kshlib_reqsend(KSHLIB *uip,MSGDATA *mip,int dl,int cl)
{
	const int	fd = uip->sfd ;
	return msgdata_send(mip,fd,dl,cl) ;
}
/* end subroutine (kshlib_reqsend) */


static int kshlib_reqrecv(KSHLIB *uip,MSGDATA *mip)
{
	struct pollfd	fds[1] ;
	const int	fd = uip->sfd ;
	const int	mto = (uip->intpoll*POLLINTMULT) ;
	const int	nfds = 1 ;
	int		size ;
	int		rs ;
	int		rc = 0 ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqrecv: ent\n") ;
#endif

	size = (nfds * sizeof(struct pollfd)) ;
	memset(fds,0,size) ;
	fds[0].fd = fd ;
	fds[0].events = (POLLIN | POLLPRI | POLLERR) ;
	fds[0].revents = 0 ;

	while ((rs = u_poll(fds,nfds,mto)) >= 0) {
	    int		f = FALSE ;
	    if (rs > 0) {
	        const int	re = fds[0].revents ;
	        if (re & (POLLIN|POLLPRI)) {
	            if ((rs = msgdata_recv(mip,fd)) >= 0) {
	                f = TRUE ;
	                if (rs > 0) {
	                    rc = MKCHAR(mip->mbuf[0]) ;
	                } else {
	                    rc = sesmsgtype_invalid ;
			}
	            } /* end if (msgdata_recv) */
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

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_reqrecv: ret rs=%d rc=%u\n",rs,rc) ;
#endif

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

	if ((uip->pollcount % 5) == 4) {
	    const time_t	dt = time(NULL) ;
	    const int		intsescheck = uip->intsescheck ;
	    if ((dt - uip->ti_sescheck) >= intsescheck) {
	        TMTIME		m ;
	        uip->ti_sescheck = dt ;
	        if ((rs = tmtime_localtime(&m,dt)) >= 0) {
		    if (m.hour >= uip->seshour) {
		        cchar	*sesdname = KSHLIB_SESDNAME ;
		        rs = rmsesfiles(sesdname) ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_poll: rmsesfiles() rs=%d\n",rs) ;
#endif
		    }
		}
	    }
	}

	uip->pollcount += 1 ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib_poll: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kshlib_poll) */


static int kshlib_cmdsend(KSHLIB *uip,int cmd)
{
	int		rs = SR_OK ;
	int		rs1 ;
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
	            MSGDATA	m ;
	            if ((rs = msgdata_init(&m,0)) >= 0) {
	                SESMSG_EXIT	m0 ;
	                const int	sal = uip->servlen ;
	                const void	*sap = &uip->servaddr ;
	                msgdata_setaddr(&m,sap,sal) ;
	                memset(&m0,0,sizeof(SESMSG_EXIT)) ;
	                if ((rs = sesmsg_exit(&m0,0,m.mbuf,m.mlen)) >= 0) {
	                    rs = kshlib_reqsend(uip,&m,rs,0) ;
#if	CF_DEBUGN
	                    nprintf(NDF,
	                        "kshlib_cmdsend: kshlib_reqsend() rs=%d\n",rs) ;
#endif
	                } /* end if (sesmsg_exit) */
#if	CF_DEBUGN
	                nprintf(NDF,
	                    "kshlib_cmdsend: sesmsg_exit-out rs=%d\n",rs) ;
#endif
	    		rs1 = msgdata_fini(&m) ;
	    		if (rs >= 0) rs = rs1 ;
	            } /* end if (msgdata) */
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


static void kshlib_atforkparent()
{
	KSHLIB		*uip = &kshlib_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (kshlib_atforkafter) */


static void kshlib_atforkchild()
{
	KSHLIB		*uip = &kshlib_data ;
	uip->f_running = FALSE ;
	uip->f_exiting = FALSE ;
	uip->pid = getpid() ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (kshlib_atforkchild) */


/* ARGSUSED */
static void kshlib_sighand(int sn,siginfo_t *sip,void *vcp)
{
	KSHLIB		*kip = &kshlib_data ;
	switch (sn) {
	case SIGQUIT:
	    kip->f_sigquit = TRUE ;
	    break ;
	case SIGTERM:
	    kip->f_sigterm = TRUE ;
	    break ;
	case SIGINT:
	    kip->f_sigintr = TRUE ;
	    break ;
	case SIGWINCH:
	    kip->f_sigwich = TRUE ;
	    break ;
	case SIGCHLD:
	    kip->f_sigchild = TRUE ;
	    break ;
	case SIGTSTP:
	    kip->f_sigsusp = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (kshlib_sighand) */


static int kshlib_capbegin(KSHLIB *uip,int to)
{
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGN && 0
	nprintf(NDF,"kshlib_capbegin: ent to=%d\n",to) ;
#endif
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
#if	CF_DEBUGN && 0
	nprintf(NDF,"kshlib_capbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_capbegin) */


static int kshlib_capend(KSHLIB *uip)
{
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGN && 0
	nprintf(NDF,"kshlib_capend: ent\n") ;
#endif
	if ((rs = ptm_lock(&uip->m)) >= 0) {
	    uip->f_capture = FALSE ;
	    if (uip->waiters > 0) {
	        rs = ptc_signal(&uip->c) ;
	    }
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
#if	CF_DEBUGN && 0
	nprintf(NDF,"kshlib_capend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_capend) */


/* ARGSUSED */
static int kshlib_sigbegin(KSHLIB *kip,const int *catches)
{
	int		rs ;
	sighand_handler	sh = kshlib_sighand ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_sigbegin: ent\n") ;
#endif
	kip->f_sigterm = 0 ;
	kip->f_sigintr = 0 ;
	rs = sighand_start(&kip->sm,sigblocks,sigigns,sigints,sh) ;
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
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_sigend: ent\n") ;
#endif
	rs1 = sighand_finish(&kip->sm) ;
	if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_sigend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_sigend) */


static int kshlib_notesbegin(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		f = TRUE ;
	if (! uip->open.notes) {
	    if ((rs = kshlib_mqbegin(uip)) >= 0) {
	        if ((rs = kshlib_runbegin(uip)) >= 0) {
	    	    uip->open.notes = TRUE ;
		    f = FALSE ;
		}
	    }
	}
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_notesbegin: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_notesbegin) */


static int kshlib_notesend(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	if (uip->open.notes) {
	    rs1 = kshlib_runend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->open.notes = FALSE ;
	    f = TRUE ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kshlib_notesend) */


static int kshlib_notesactive(KSHLIB *uip)
{
	return MKBOOL(uip->open.notes) ;
}
/* end subroutine (kshlib_notesactive) */


static int kshlib_notescount(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (uip->open.notes) {
	    rs = kshlib_mqcount(uip) ;
	}
	return rs ;
}
/* end subroutine (kshlib_notescount) */


static int kshlib_mqbegin(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (! uip->open.mq) {
	    const int	n = KSHLIB_NENTS ;
	    if ((rs = raqhand_start(&uip->mq,n,0)) >= 0) {
	        uip->open.mq = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (kshlib_mqbegin) */


static int kshlib_mqend(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->open.mq) {
	    rs1 = kshlib_mqfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = raqhand_finish(&uip->mq) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->open.mq = FALSE ;
	}
	return rs ;
}
/* end subroutine (kshlib_mqend) */


static int kshlib_mqfins(KSHLIB *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->open.mq) {
	    RAQHAND	*qlp = &uip->mq ;
	    STORENOTE	*ep ;
	    int		i ;
	    for (i = 0 ; raqhand_get(qlp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs1 = storenote_finish(ep) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_libfree(ep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (non-null) */
	    } /* end for */
	} /* end if (open-mq) */
	return rs ;
}
/* end subroutine (kshlib_mqfins) */


static int kshlib_mqcount(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (uip->open.mq) {
	    rs = raqhand_count(&uip->mq) ;
	}
#if	CF_DEBUGN
	nprintf(NDF,"kshlib_mqcount: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (kshlib_mqcount) */


static int kshlib_mqactive(KSHLIB *uip)
{
	int		rs = MKBOOL(uip->open.mq) ;
	return rs ;
}
/* end subroutine (kshlib_mqactive) */


#if	CF_MQ
/* ensure message-queue operations are initialized */
static int kshlib_mq(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (! uip->open.mq) {
	    rs = kshlib_mqbegin(uip) ;
	}
	return rs ;
}
/* end subroutine (kshlib_mq) */
#endif /* CF_MQ */


static int kshlib_sesend(KSHLIB *uip)
{
	int		rs = SR_OK ;
	if (uip->sesdname != NULL) {
	    if ((rs = kshlib_sid(uip)) >= 0) {
		if (uip->sid == uip->pid) {
		    if ((rs = isdirempty(uip->sesdname)) > 0) {
			rs = u_rmdir(uip->sesdname) ;
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (kshlib_sesend) */


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
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
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


#if	CF_LOCMALSTRW
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
#endif /* CF_LOCMALSTRW */


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
	int		rl = 0 ;
	char		dbuf[DIGBUFLEN+1] = { 's' } ;

#if	CF_DEBUGN
	nprintf(NDF,"kshlib/mksdname: sid=%d\n",sid) ;
#endif

	if ((rs = ctdecui((dbuf+1),(dlen-1),uv)) >= 0) {
	    if ((rs = mkpath2(rbuf,dname,dbuf)) >= 0) {
	        const mode_t	dm = 0777 ;
		rl = rs ;
	        if ((rs = mkdirs(rbuf,dm)) >= 0) {
	            rs = uc_minmod(rbuf,dm) ;
	        }
	    } /* end if (mkpath) */
	} /* end if (ctdecui) */

#if	CF_DEBUGN
	nprintf(NDF,"kshlib/mksdname: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mksdname) */


#if	CF_DEBUGENV && CF_DEBUGN
static int ndebugenv(cchar *s,cchar *ev[])
{
	if (s != NULL) {
	    if (ev != NULL) {
	        int	i ;
	        cchar	*dfn = NDF ;
		cchar	*ep ;
	        cchar	*fmt = "%s: e%03u=>%t<\n" ;
	        nprintf(dfn,"%s: env¬\n", s) ;
	        for (i = 0 ; ev[i] != NULL ; i += 1) {
	            ep = ev[i] ;
	            nprintf(dfn,fmt,s,i,ep,strlinelen(ep,-1,50)) ;
	        }
	        nprintf(dfn,"%s: nenv=%u\n", s,i) ;
	    } else {
	        nprintf(dfn,"%s: environ=*null*\n",s) ;
	    }
	}
	return 0 ;
}
/* end subroutine (ndebugenv) */
#endif /* CF_DEBUGENV */


#if	CF_DEBUGN

static int nprintpid(cchar *s)
{
	const uint	id = getpid() ;
	return nprintf(NDF,"%s: ent pid=%u\n",s,id) ;
}

static int nprintid(cchar *s)
{
	const pthread_t	tid = uptself(NULL) ;
	return nprintf(NDF,"%s: tid=%u\n",s,tid) ;
}
/* end subroutine (nprintid) */

static int nprintutmp(char *s)
{
	KSHLIB		*uip = &kshlib_data ;
	int		rs ;
	nprintf(NDF,"%s: UTMPACC test-begin\n",s) ;
	{
		UTMPACC_ENT	ue ;
		const pid_t	sid = getsid(0) ;
		const pid_t	pid = uip->pid ;
		const int	ulen = UTMPACC_BUFLEN ;
		char		ubuf[UTMPACC_BUFLEN+1] ;

	nprintf(NDF,"%s: sid=%d pid=%u\n",s,sid,pid) ;
	rs = utmpacc_entsid(&ue,ubuf,ulen,pid) ;
	nprintf(NDF,"%s: utmpacc() rs=%d\n",s,rs) ;

	}
	nprintf(NDF,"%s: UTMPACC test-end\n",s) ;
	return rs ;
}
/* end subroutine (nprintutmp) */

#endif /* CF_DEBUGN */


