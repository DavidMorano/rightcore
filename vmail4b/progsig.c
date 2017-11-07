/* progsig */
/* lang=C89 */

/* program signal handling */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_SIGTSTP	1		/* Solaris® bug in |sigpending(2)| */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage process signals.


*******************************************************************************/


#define	PROGSIG_MASTER	1	/* claim excemption from own forwards */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<poll.h>
#include	<string.h>

#include	<vsystem.h>
#include	<upt.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<sighand.h>
#include	<sockaddress.h>
#include	<raqhand.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sesmsg.h"
#include	"msgdata.h"
#include	"progsig.h"


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
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	NDF		"progsig.deb"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	TO_LOCKENV	10

#define	PROGSIG		struct progsig
#define	PROGSIG_FL	struct progsig_flags
#define	PROGSIG_SCOPE	PTHREAD_SCOPE_PROCESS
#define	PROGSIG_SESDN	"/var/tmp/sessions"

#define	STORENOTE	struct storenote


/* local pragmas */


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

struct progsig_flags {
	uint		dummy:1 ;
} ;

struct progsig {
	PTM		m ;		/* mutex data */
	PTM		menv ;		/* mutex environment */
	PTC		c ;		/* condition variable */
	SIGHAND		sm ;
	SOCKADDRESS	servaddr ;	/* server address */
	RAQHAND		mq ;		/* message queue */
	PROGSIG_FL	f ;
	cchar		*reqfname ;
	cchar		**envv ;
	pthread_t	tid ;
	pid_t		pid ;
	volatile int	f_init ;
	volatile int	f_initdone ;
	volatile int	f_running ;
	volatile int	f_capture ;
	volatile int	f_exiting ;
	volatile int	waiters ;
	sig_atomic_t	f_sigquit ;
	sig_atomic_t	f_sigterm ;
	sig_atomic_t	f_sigintr ;
	sig_atomic_t	f_sigwich ;
	sig_atomic_t	f_sigsusp ;
	sig_atomic_t	f_sigchild ;
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

enum cmds {
	cmd_session,
	cmd_overlast
} ;


/* forward references */

int		progsig_initenviron(void *) ;
int		progsig_callcmd(cchar *,int,cchar **,cchar **,void *) ;
int 		progsig_callfunc(subcmd_t,int,cchar **,cchar **,void *) ;

int		progsig_init(void) ;
void		progsig_fini(void) ;

static void	progsig_atforkbefore() ;
static void	progsig_atforkafter() ;
static void	progsig_sighand(int,siginfo_t *,void *) ;

static int	progsig_mainstuff(PROGSIG *) ;

static int	progsig_begin(PROGSIG *) ;
static int	progsig_end(PROGSIG *) ;
static int	progsig_runbegin(PROGSIG *) ;
static int	progsig_runner(PROGSIG *) ;
static int	progsig_runend(PROGSIG *) ;
static int	progsig_entfins(PROGSIG *) ;
static int	progsig_mq(PROGSIG *) ;
static int	progsig_mkreqfname(PROGSIG *,char *,cchar *) ;
static int	progsig_worker(PROGSIG *) ;
static int	progsig_workecho(PROGSIG *,MSGDATA *) ;
static int	progsig_workbiff(PROGSIG *,MSGDATA *) ;
static int	progsig_workbiffer(PROGSIG *,SESMSG_BIFF *) ;
static int	progsig_workgen(PROGSIG *,MSGDATA *) ;
static int	progsig_workgener(PROGSIG *,SESMSG_GEN *) ;
static int	progsig_workdef(PROGSIG *,MSGDATA *) ;
static int	progsig_msgenter(PROGSIG *,STORENOTE *) ;
static int	progsig_reqopen(PROGSIG *) ;
static int	progsig_reqopener(PROGSIG *,cchar *) ;
static int	progsig_reqsend(PROGSIG *,MSGDATA *,int) ;
static int	progsig_reqrecv(PROGSIG *,MSGDATA *) ;
static int	progsig_reqclose(PROGSIG *) ;
static int	progsig_poll(PROGSIG *) ;
static int	progsig_cmdsend(PROGSIG *,int) ;
static int	progsig_capbegin(PROGSIG *,int) ;
static int	progsig_capend(PROGSIG *) ;
static int	progsig_sigbegin(PROGSIG *) ;
static int	progsig_sigend(PROGSIG *) ;

int		progsig_initmemalloc(int) ;

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

static PROGSIG		progsig_data ; /* zero-initialized */

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
	SIGQUIT,
	SIGTERM,
	SIGINT,
	SIGWINCH,
	SIGCHLD,
#if	CF_SIGTSTP /* causes a hang in |sigpending(2)| in Solaris® UNIX® */
	SIGTSTP,
#endif
	0
} ;


/* exported subroutines */


int progsig_init(void)
{
	PROGSIG		*uip = &progsig_data ;
	int		rs = SR_OK ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	            void	(*b)() = progsig_atforkbefore ;
	            void	(*a)() = progsig_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(progsig_fini)) >= 0) {
			    uip->pid = getpid() ;
			    uip->sfd = -1 ;
		            rs = 1 ;
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
/* end subroutine (progsig_init) */


void progsig_fini(void)
{
	struct progsig	*uip = &progsig_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        progsig_runend(uip) ;
		progsig_end(uip) ;
	    }
	    {
	        void	(*b)() = progsig_atforkbefore ;
	        void	(*a)() = progsig_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct progsig)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (progsig_fini) */


int progsig_mainbegin(cchar **envv)
{
	int		rs ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mainbegin: ent\n") ;
#endif

	if ((rs = progsig_init()) >= 0) {
	    PROGSIG	*uip = &progsig_data ;
	    uip->envv = envv ;
	    if ((rs = progsig_sigbegin(uip)) >= 0) {
		rs = progsig_mainstuff(uip) ;
		if (rs < 0)
	    	    progsig_sigend(uip) ;
	    } /* end if (progsig_sigbegin) */
	} /* end if (progsig_init) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mainbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsig_mainbegin) */


int progsig_mainend(void)
{
	PROGSIG		*uip = &progsig_data ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mainend: ent\n") ;
#endif

	if (uip->f_running) {
	    rs1 = progsig_runend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (running) */

	rs1 = progsig_sigend(uip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mainend: ret rs=%d\n",rs) ;
#endif

	uip->envv = NULL ;
	return rs ;
}
/* end subroutine (progsig_mainend) */


int progsig_adm(int cmd)
{
	PROGSIG		*kip = &progsig_data ;
	int		rs = SR_OK ;
	switch (cmd) {
	case cmd_session:
	    rs = progsig_runbegin(kip) ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return rs ;
}
/* end subroutine (progsig_runmode) */


int progsig_runmode(void)
{
	PROGSIG		*kip = &progsig_data ;
	return kip->runmode ;
}
/* end subroutine (progsig_runmode) */


int progsig_serial(void)
{
	PROGSIG		*kip = &progsig_data ;
	int		s = kip->serial ;
	return s ;
}
/* end subroutine (progsig_serial) */


int progsig_sigquit(void)
{
	PROGSIG		*kip = &progsig_data ;
	int		rs = SR_OK ;
	if (kip->f_sigquit) {
	    kip->f_sigquit = 0 ;
	    rs = SR_QUIT ;
	}
	return rs ;
}
/* end subroutine (progsig_sigquit) */


int progsig_sigterm(void)
{
	PROGSIG		*kip = &progsig_data ;
	int		rs = SR_OK ;
	if (kip->f_sigterm) {
	    kip->f_sigterm = 0 ;
	    rs = SR_EXIT ;
	}
	return rs ;
}
/* end subroutine (progsig_sigterm) */


int progsig_sigintr(void)
{
	PROGSIG		*kip = &progsig_data ;
	int		rs = SR_OK ;
	if (kip->f_sigintr) {
	    kip->f_sigintr = 0 ;
	    rs = SR_INTR ;
	}
	return rs ;
}
/* end subroutine (progsig_sigintr) */


int progsig_issig(int sn)
{
	PROGSIG		*kip = &progsig_data ;
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
	case SIGTSTP:
	    f = kip->f_sigsusp ;
	    if (f) kip->f_sigsusp = 0 ;
	    break ;
	case SIGCHLD:
	    f = kip->f_sigchild ;
	    if (f) kip->f_sigchild = 0 ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progsig_issig) */


int progsig_noteread(PROGSIG_NOTE *rp,int ni)
{
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	if (rp == NULL) return SR_FAULT ;
	memset(rp,0,sizeof(PROGSIG_NOTE)) ;
	if (ni < 0) return SR_INVALID ;
	if ((rs = progsig_init()) >= 0) {
	    PROGSIG	*uip = &progsig_data ;
	    if ((rs = progsig_capbegin(uip,-1)) >= 0) {
	        if ((rs = progsig_mq(uip)) >= 0) {
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
		} /* end if (progsig_mq) */
	        rs1 = progsig_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (progsig_init) */
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (progsig_noteread) */


int progsig_notedel(int ni)
{
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	if (ni < 0) return SR_INVALID ;
	if ((rs = progsig_init()) >= 0) {
	    PROGSIG	*uip = &progsig_data ;
	    if ((rs = progsig_capbegin(uip,-1)) >= 0) {
	        if ((rs = progsig_mq(uip)) >= 0) {
		    rs = raqhand_del(&uip->mq,ni) ;
		    rc = rs ;
		} /* end if (progsig_mq) */
	        rs1 = progsig_capend(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (progsig_init) */
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (progsig_notedel) */


/* local subroutines */


static int progsig_mainstuff(PROGSIG *uip)
{
	int		rs = SR_OK ;
	if (uip == NULL) return SR_FAULT ;
	return rs ;
}
/* end subroutine (progsig_mainstuff) */


static int progsig_mq(PROGSIG *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_mq) {
	    rs = progsig_begin(uip) ;
	}
	return rs ;
}
/* end subroutine (progsig_mq) */


static int progsig_begin(PROGSIG *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_mq) {
	    const int	n = PROGSIG_NENTS ;
	    if ((rs = raqhand_start(&uip->mq,n,0)) >= 0) {
	        uip->f_mq = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (progsig_begin) */


static int progsig_end(PROGSIG *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->f_mq) {
	    rs1 = progsig_entfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->f_mq = FALSE ;
	    rs1 = raqhand_finish(&uip->mq) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (progsig_end) */


static int progsig_entfins(PROGSIG *uip)
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
/* end subroutine (progsig_entfins) */


static int progsig_runbegin(PROGSIG *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_runbegin: ent f_running=%u\n",uip->f_running) ;
#endif

	if (! uip->f_running) {
	    if ((rs = progsig_capbegin(uip,-1)) >= 0) {
		if (! uip->f_running) {
		    if ((rs = progsig_reqopen(uip)) >= 0) {
		        rs = progsig_runner(uip) ;
		        f = rs ;
		    } /* end if (progsig_reqopen) */
		} /* end if (not running) */
		rs1 = progsig_capend(uip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (capture) */
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_runbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progsig_runbegin) */


static int progsig_runner(PROGSIG *uip)
{
	PTA		ta ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&ta)) >= 0) {
	    const int	scope = PROGSIG_SCOPE ;
	    if ((rs = pta_setscope(&ta,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) progsig_worker ;
		if ((rs = uptcreate(&tid,&ta,wt,uip)) >= 0) {
		    uip->f_running = TRUE ;
		    uip->tid = tid ;
		    f = TRUE ;
		} /* end if (pthread-create) */
#if	CF_DEBUGN
		nprintf(NDF,"progsig_runner: pt-create rs=%d tid=%u\n",
			rs,tid) ;
#endif
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&ta) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_runner: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progsig_runner) */


static int progsig_runend(PROGSIG *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_runend: ent run=%u\n",uip->f_running) ;
#endif

	if (uip->f_running) {
	    const int	cmd = sesmsgtype_exit ;
	    if ((rs = progsig_cmdsend(uip,cmd)) >= 0) {
	 	pthread_t	tid = uip->tid ;
		int		trs ;
		if ((rs = uptjoin(tid,&trs)) >= 0) {
		    uip->f_running = FALSE ;
		    rs = trs ;
		}
#if	CF_DEBUGN
		nprintf(NDF,"progsig_runend: pt-join rs=%d tid=%u\n",
			rs,tid) ;
#endif
	        rs1 = progsig_reqclose(uip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (progsig_cmdsend) */
	} /* end if (running) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_runend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsig_runend) */


/* it always takes a good bit of code to make this part look easy! */
static int progsig_worker(PROGSIG *uip)
{
	MSGDATA		m ;
	int		rs = SR_OK ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_worker: ent\n") ;
#endif

	    while ((rs = progsig_reqrecv(uip,&m)) > 0) {
#if	CF_DEBUGN
	nprintf(NDF,"progsig_worker: reqrecv mt=%u\n",rs) ;
#endif
	        switch (rs) {
	        case sesmsgtype_echo:
		    rs = progsig_workecho(uip,&m) ;
		    break ;
	        case sesmsgtype_gen:
		    rs = progsig_workgen(uip,&m) ;
		    break ;
	        case sesmsgtype_biff:
		    rs = progsig_workbiff(uip,&m) ;
		    break ;
		default:
		    rs = progsig_workdef(uip,&m) ;
		    break ;
	        } /* end switch */
	        if (rs < 0) break ;
	    } /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (progsig_worker) */


static int progsig_workecho(PROGSIG *uip,MSGDATA *mip)
{
	int		rs ;
	if ((rs = msgdata_conpass(mip,FALSE)) >= 0) {
	    rs = progsig_reqsend(uip,mip,0) ;
	} /* end if (msgdata_conpass) */
	return rs ;
}
/* end subroutine (progsig_workecho) */


static int progsig_workgen(PROGSIG *uip,MSGDATA *mip)
{
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workgen: ent\n") ;
#endif
#if	CF_DEBUGS
	debugprintf("progsig_workgen: ent\n") ;
#endif
#if	CF_DEBUGS && CF_DEBUGHEXB
	debugprinthexblock("progsig_workgen: ",80,mip->mbuf,mip->mlen) ;
#endif
	if ((rs = progsig_capbegin(uip,-1)) >= 0) {
	    if ((rs = progsig_mq(uip)) >= 0) {
		SESMSG_GEN	m2 ;
		if ((rs = sesmsg_gen(&m2,1,mip->mbuf,mip->mlen)) >= 0) {
		    rs = progsig_workgener(uip,&m2) ;
		} /* end if (sesmsg_gen) */
	    } /* end if (progsig_mq) */
	    rs1 = progsig_capend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (capture) */
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workgen: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progsig_workgen) */


static int progsig_workgener(PROGSIG *uip,SESMSG_GEN *mp)
{
	STORENOTE	*ep ;
	const int	esize = sizeof(STORENOTE) ;
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workgener: ent\n") ;
	nprintf(NDF,"progsig_workgener: m=>%t<\n",
		mp->nbuf,strlinelen(mp->nbuf,-1,50)) ;
#endif
	if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	    time_t	st = mp->stime ;
	    const int	mt = mp->msgtype ;
	    const int	nlen = strlen(mp->nbuf) ;
	    cchar	*nbuf = mp->nbuf ;
	    cchar	*un = mp->user ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workgener: m=>%t<\n",
		nbuf,strlinelen(nbuf,nlen,50)) ;
#endif
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nlen)) >= 0) {
		rs = progsig_msgenter(uip,ep) ;
	        if (rs < 0)
		    storenote_finish(ep) ;
	    } /* end if (storenote_start) */
	    if (rs < 0)
		uc_libfree(ep) ;
	} /* end if (m-a) */
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workgener: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progsig_workgener) */


static int progsig_workbiff(PROGSIG *uip,MSGDATA *mip)
{
	int		rs ;
	int		rs1 ;
	if ((rs = progsig_capbegin(uip,-1)) >= 0) {
	    if ((rs = progsig_mq(uip)) >= 0) {
		SESMSG_BIFF	m3 ;
		if ((rs = sesmsg_biff(&m3,1,mip->mbuf,mip->mlen)) >= 0) {
		    rs = progsig_workbiffer(uip,&m3) ;
		} /* end if (sesmsg_biff) */
	    } /* end if (progsig_mq) */
	    rs1 = progsig_capend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (capture) */
	return rs ;
}
/* end subroutine (progsig_workbiff) */


static int progsig_workbiffer(PROGSIG *uip,SESMSG_BIFF *mp)
{
	STORENOTE	*ep ;
	const int	esize = sizeof(STORENOTE) ;
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_workbiffer: m=>%t<\n",
		mp->nbuf,strlinelen(mp->nbuf,-1,50)) ;
#endif
	if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	    time_t	st = mp->stime ;
	    const int	mt = mp->msgtype ;
	    const int	nlen = strlen(mp->nbuf) ;
	    cchar	*un = mp->user ;
	    cchar	*nbuf = mp->nbuf ;
	    if ((rs = storenote_start(ep,mt,st,un,nbuf,nlen)) >= 0) {
		rs = progsig_msgenter(uip,ep) ;
	        if (rs < 0)
		    storenote_finish(ep) ;
	    } /* end if (storenote_start) */
	    if (rs < 0)
		uc_libfree(ep) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (progsig_workbiffer) */


static int progsig_workdef(PROGSIG *uip,MSGDATA *mip)
{
	int		rs ;
	if (mip == NULL) return SR_FAULT ;
	if ((rs = ptm_lock(&uip->m)) >= 0) {
	    uip->cdefs += 1 ;
	    ptm_unlock(&uip->m) ;
	} /* end if (mutex) */
	return rs ;
}
/* end subroutine (progsig_workdef) */


static int progsig_msgenter(PROGSIG *uip,STORENOTE *ep)
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
/* end subroutine (progsig_msgenter) */


static int progsig_reqopen(PROGSIG *uip)
{
	int		rs ;
	cchar		*dname = PROGSIG_SESDN ;

	if ((rs = sdir(dname,(W_OK|X_OK))) >= 0) {
	    pid_t	sid = getsid(0) ;
	    char	sbuf[MAXPATHLEN+1] ;
	    if ((rs = mksdname(sbuf,dname,sid)) >= 0) {
		if (uip->reqfname == NULL) {
		    char	pbuf[MAXPATHLEN+1] ;
	            if ((rs = progsig_mkreqfname(uip,pbuf,sbuf)) >= 0) {
			rs = progsig_reqopener(uip,pbuf) ;
		    } /* end if (progsig_mkreqfname) */
		} /* end if (reqfname) */
	    } /* end if (mksdname) */
	} /* end if (sdir) */

	return rs ;
}
/* end subroutine (progsig_reqopen) */


static int progsig_reqopener(PROGSIG *uip,cchar *pbuf)
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
/* end subroutine (progsig_reqopener) */


static int progsig_reqclose(PROGSIG *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_reqclose: ent sfd=%d\n",uip->sfd) ;
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
	nprintf(NDF,"progsig_reqclose: reqfname{%p}=¿\n",uip->reqfname) ;
	nprintf(NDF,"progsig_reqclose: reqfname=%s\n",uip->reqfname) ;
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
	nprintf(NDF,"progsig_reqclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsig_reqclose) */


static int progsig_reqsend(PROGSIG *uip,MSGDATA *mip,int clen)
{
	const int	fd = uip->sfd ;
	return msgdata_send(mip,fd,clen) ;
}
/* end subroutine (progsig_reqsend) */


static int progsig_reqrecv(PROGSIG *uip,MSGDATA *mip)
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
		    if ((rs = msgdata_recv(mip,fd)) >= 0) {
			f = TRUE ;
	    	        if (rs > 0) {
	        	    rc = MKCHAR(mip->mbuf[0]) ;
	    	        } else
	        	    rc = sesmsgtype_invalid ;
	            } /* end if (msgdata_recv) */
		} else if (re & POLLERR) {
		    rs = SR_IO ;
		}
	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }
	    if (f) break ;
	    if (rs >= 0) {
		rs = progsig_poll(uip) ;
	    }
	    if (rs < 0) break ;
	} /* end while (polling) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (progsig_reqrecv) */


static int progsig_poll(PROGSIG *uip)
{
	int		rs = SR_OK ;

	if (uip == NULL) return SR_FAULT ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_poll: ent\n") ;
#endif

	return rs ;
}
/* end subroutine (progsig_poll) */


static int progsig_cmdsend(PROGSIG *uip,int cmd)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_cmdsend: ent cmd=%u\n",cmd) ;
	nprintf(NDF,"progsig_cmdsend: f_running=%u\n",uip->f_running) ;
#endif
	if (uip->f_running && (uip->reqfname != NULL)) {
	    f = TRUE ;
	    switch (cmd) {
	    case sesmsgtype_exit:
		{
	    	    MSGDATA	m ;
		    if ((rs = msgdata_init(&m,0)) >= 0) {
		        SESMSG_EXIT	m0 ;
			const int	mlen = MSGBUFLEN ;
			const int	sal = uip->servlen ;
			const void	*sap = &uip->servaddr ;
			msgdata_setaddr(&m,sap,sal) ;
			memset(&m0,0,sizeof(SESMSG_EXIT)) ;
		        if ((rs = sesmsg_exit(&m0,0,m.mbuf,mlen)) >= 0) {
			    m.mlen = rs ;
	    	            rs = progsig_reqsend(uip,&m,0) ;
#if	CF_DEBUGN
			    nprintf(NDF,
				"progsig_cmdsend: progsig_reqsend() rs=%d\n",
				rs) ;
#endif
			} /* end if (sesmsg_exit) */
#if	CF_DEBUGN
			nprintf(NDF,
				"progsig_cmdsend: sesmsg_exit-out rs=%d\n",rs) ;
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
	nprintf(NDF,"progsig_cmdsend: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progsig_cmdsend) */


static void progsig_atforkbefore()
{
	PROGSIG		*uip = &progsig_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (progsig_atforkbefore) */


static void progsig_atforkafter()
{
	PROGSIG		*uip = &progsig_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (progsig_atforkafter) */


/* ARGSUSED */
static void kshlib_sighand(int sn,siginfo_t *sip,void *vcp)
{
	PROGSIG		*kip = &progsig_data ;
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
	case SIGTSTP:
	    kip->f_sigsusp = TRUE ;
	    break ;
	case SIGCHLD:
	    kip->f_sigchild = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (progsig_sighand) */


static int progsig_mkreqfname(PROGSIG *uip,char *sbuf,cchar *dname)
{
	const uint	uv = (uint) uip->pid ;
	const int	dlen = DIGBUFLEN ;
	int		rs ;
	char		dbuf[DIGBUFLEN+1] = { 'p' } ;

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mkreqfname: ent pid=%u\n",uv) ;
#endif

	if ((rs = ctdecui((dbuf+1),(dlen-1),uv)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"progsig_mkreqfname: dbuf=%s\n",dbuf) ;
#endif
	    if ((rs = mkpath2(sbuf,dname,dbuf)) >= 0) {
#if	CF_DEBUGN
		nprintf(NDF,"progsig_mkreqfname: mkpath2() rs=%d sbuf=%s\n",
		rs,sbuf) ;
#endif
		if (uip->reqfname == NULL) {
		    cchar	*cp ;
		    if ((rs = mallocstrw(sbuf,rs,&cp)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"progsig_mkreqfname: reqfname{%p}=¿\n",cp) ;
	nprintf(NDF,"progsig_mkreqfname: reqfname{%p}=%s\n",cp,cp) ;
#endif
			uip->reqfname = cp ;
		    }
		}
	    } /* end if (mkpath) */
	} /* end if (ctdecui) */

#if	CF_DEBUGN
	nprintf(NDF,"progsig_mkreqfname: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progsig_mkreqfname) */


static int progsig_capbegin(PROGSIG *uip,int to)
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
/* end subroutine (progsig_capbegin) */


static int progsig_capend(PROGSIG *uip)
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
/* end subroutine (progsig_capend) */


static int progsig_sigbegin(PROGSIG *kip)
{
	int		rs ;
	void		(*sh)(int) = progsig_sighand ;
	kip->f_sigterm = 0 ;
	kip->f_sigintr = 0 ;
	rs = sighand_start(&kip->sm,sigblocks,sigigns,sigints,sh) ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_sigbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progsig_sigbegin) */


static int progsig_sigend(PROGSIG *kip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = sighand_finish(&kip->sm) ;
	if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"progsig_sigend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (progsig_sigend) */


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
	nprintf(NDF,"progsig/mksdname: sid=%d\n",sid) ;
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
	nprintf(NDF,"progsig/mksdname: ret rs=%d\n",rs) ;
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


