/* uc_timemgr */
/* lang=C++11 */

/* UNIX® time management */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2017-10-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manage time-outs (and registered call-backs).

	Synopsis:

	int uc_timemgr(int cmd,TIMEMGR *val)

	Arguments:

	cmd		command:
			    timemgrcmd_set,
			    timemgrcmd_cancel

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<time.h>
#include	<string.h>
#include	<queue>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<pta.h>
#include	<upt.h>
#include	<vechand.h>		/* vector-handles */
#include	<ciq.h>			/* container-interlocked-queue */
#include	<localmisc.h>

#include	"timemgr.h"


/* local defines */

#define	UCTIMEMGR	struct uctimemgr
#define	UCTIMEMGR_FL	struct uctimemgr_flags
#define	UCTIMEMGR_SCOPE	PTHREAD_SCOPE_SYSTEM

#define	NDF		"uctimemgr.deb"


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

#ifndef	TYPEDEF_TWORKER
#define	TYPEDEF_TWORKER	1
typedef	int (*tworker)(void *) ;
#endif

struct uctimemgr_flags {
	uint		timer:1 ;	/* UNIX-RT timer created */
	uint		working:1 ;
	uint		running_catch:1 ;
	uint		running_disp:1 ;
} ;

struct uctimemgr {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	vechand		ents ;
	ciq		pass ;
	UCTIMEMGR_FL	open, f ;
	pririty_queue	*pqp ;
	pid_t		pid ;
	pthread_t	tid_catch ;
	pthread_t	tid_disp ;
	timer_t		timerid ;
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
	volatile int	f_begin ;
	volatile int	f_running ;
	volatile int	f_cmd ;
	volatile int	f_syncing ;
	volatile int	f_exiting ;
	volatile int	waiters ;
	int		cmd ;
	int		count ;
} ;

enum cmds {
	cmd_exit,
	cmd_sync,
	cmd_overlast
} ;


/* forward references */

extern "C" int	uctimemgr_init() ;
extenr "C" void	uctimemgr_fini() ;

static int	uctimemgr_capbegin(UCTIMEMGR *,int) ;
static int	uctimemgr_capend(UCTIMEMGR *) ;

static int	uctimemgr_workready(UCTIMEMGR *) ;
static int	uctimemgr_workbegin(UCTIMEMGR *) ;
static int	uctimemgr_workend(UCTIMEMGR *) ;

static int	uctimemgr_priqbegin(UCTIMEMGR *) ;
static int	uctimemgr_priqend(UCTIMEMGR *) ;

static int	uctimemgr_timerbegin(UCTIMEMGR *) ;
static int	uctimemgr_timerend(UCTIMEMGR *) ;

static int	uctimemgr_thrsbegin(UCTIMEMGR *) ;
static int	uctimemgr_thrsend(UCTIMEMGR *) ;

static int	uctimemgr_thrcatchbegin(UCTIMEMGR *) ;
static int	uctimemgr_thrcatchend(UCTIMEMGR *) ;
static int	uctimemgr_thrcatchproc(UCTIMEMGR *) ;

static int	uctimemgr_thrdispbegin(UCTIMEMGR *) ;
static int	uctimemgr_thrdispend(UCTIMEMGR *) ;

static int	uctimemgr_sendsync(UCTIMEMGR *) ;
static int	uctimemgr_run(UCTIMEMGR *) ;
static int	uctimemgr_runcheck(UCTIMEMGR *) ;
static int	uctimemgr_runner(UCTIMEMGR *) ;
static int	uctimemgr_worker(UCTIMEMGR *) ;
static int	uctimemgr_worksync(UCTIMEMGR *) ;
static int	uctimemgr_cmdsend(UCTIMEMGR *,int) ;
static int	uctimemgr_cmdrecv(UCTIMEMGR *) ;
static int	uctimemgr_waitdone(UCTIMEMGR *) ;

static void	uctimemgr_atforkbefore() ;
static void	uctimemgr_atforkparent() ;
static void	uctimemgr_atforkchild() ;


/* local variables */

static UCTIMEMGR	uctimemgr_data ; /* zero-initialized */


/* exported subroutines */


int uctimemgr_init()
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = uctimemgr_atforkbefore ;
	    	    void	(*ap)() = uctimemgr_atforkparent ;
	    	    void	(*ac)() = uctimemgr_atforkchild ;
	            if ((rs = uc_atfork(b,ap,ac)) >= 0) {
	                if ((rs = uc_atexit(uctimemgr_fini)) >= 0) {
	    	            uip->f_initdone = TRUE ;
			    uip->pid = getpid() ;
			    f = TRUE ;
		        }
		        if (rs < 0)
		            uc_atforkrelease(b,ap,ac) ;
	            } /* end if (uc_atfork) */
	            if (rs < 0)
	                ptc_destroy(&uip->c) ;
	        } /* end if (ptc_create) */
		if (rs < 0)
		    ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) rs = SR_OK ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimemgr_init) */


void uctimemgr_fini()
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    uctimemgr_waitdone(uip) ;
	    {
	        void	(*b)() = uctimemgr_atforkbefore ;
	        void	(*ap)() = uctimemgr_atforkparent ;
	        void	(*ac)() = uctimemgr_atforkchild ;
	        uc_atforkrelease(b,ap,ac) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UCTIMEMGR)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (uctimemgr_fini) */


int uc_timemgr(int cmd,TIMEMGR *valp)
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	int		rs ;

	if ((rs = uctimemgr_init()) >= 0) {
	    if ((rs = uctimemgr_capbegin(uip,-1)) >= 0) {
		if ((rs = uctimemgr_workready(uip)) >= 0) {
	            switch (cmd) {
	            case timemgrcmd_set:
	                rs = uctimemgr_cmdset(valp) ;
	                break ;
	            case timemgrcmd_cancel:
	                rs = uctimemgr_cmdcancel(valp) ;
	                break ;
		    default:
		        rs = SR_INVALID ;
		        break ;
	            } /* end switch */
		} /* end if (uctimemgr_workready) */
	        rs1 = uctimemgr_capend(uip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (uctimemgr-cap) */
	} /* end if (uctimemgr_init) */

	return rs ;
}
/* end subroutine (uc_timemgr) */


/* local subroutines */


static int uctimemgr_capbegin(UCTIMEMGR *uip,int to)
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
/* end subroutine (uctimemgr_capbegin) */


static int uctimemgr_capend(UCTIMEMGR *uip)
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
/* end subroutine (uctimemgr_capend) */


static int uctimemgr_workready(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_workready) {
	    rs = uctimemgr_workbegin(uip) ;
	}
	return rs ;
}
/* end subroutine (uctimemgr_workready) */


#ifdef	COMMENT
union sigval {
	int	sival_int;	/* integer value */
	void	*sival_ptr;	/* pointer value */
};
struct sigevent {
	int		sigev_notify;	/* notification mode */
	int		sigev_signo;	/* signal number */
	union sigval	sigev_value;	/* signal value */
	void		(*sigev_notify_function)(union sigval);
	pthread_attr_t	*sigev_notify_attributes;
	int		__sigev_pad2;
};
int sigevent_init(SIGEVENT *sep,int notify,int signo,int val)
{
	int		rs = SR_OK ;
	if (sep == NULL) return SR_FAULT ;
	memset(sep,0,sizeof(SIGEVENT)) ;
	sep->sigev_notify = notify ;
	sep->sigev_signo = signo ;
	sep->sigev_value.sigval_int = val ;
	return rs ;
}
/* end subroutine (sigevent_init) */
#endif /* COMMENT */

static int uctimemgr_workbegin(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (! uip->f_workready) {
	    int	vo = 0 ;
	    so |= (VECHAND_OSTATIONARY|VECHAND_OREUSE| VECHAND_OCOMPACT) ;
	    vo |= ( VECHAND_OSWAP| VECHAND_OCONSERVE) ;
	    if ((rs = vechand_start(&uip->ents,0,vo)) >= 0) {
		if ((rs = uctimemgr_priqbegin(uip)) >= 0) {
		    if ((rs = uctimemgr_timerbegin(uip)) >= 0) {
			if ((rs = ciq_start(&uip->pass)) >= 0) {
			    if ((rs = uctimemgr_thrsbegin(uip)) >= 0) {
			        uip->open.working = TRUE ;
			    }
			    if (rs < 0) {
			        ciq_finish(&uip->pass) ;
			    }
			}
			if (rs < 0) {
		    	    uctimemgr_timerend(uip) ;
			}
		    }
		    if (rs < 0) {
			uctimemgr_priqend(uip) ;
		    }
	        }
		if (rs < 0) {
		    vechand_finish(&uip->ents) ;
		}
	    } /* end if (vechand_start) */
	}
	return rs ;
}
/* end subroutine (uctimemgr_workbegin) */


static int uctimemgr_workend(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->open.working) {
	    rs1 = uctimemgr_thrsend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = ciq_finish(&uip->pass) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uctimemgr_timerend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uctimemgr_priqend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = vechand_finish(&uip->ents) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->open.working = FALSE ;
	}
	return rs ;
}
/* end subroutine (uctimemgr_workend) */


static int uctimemgr_priqbegin(UCTIMEMGR *uip)
{
	void		*p ;
	int		rs = SR_OK ;
	p = new(nothrow) priority_queue<TIMEMGR *,vector<TIMEMGR *>,ourcmp> ;
	if (p != NULL) {
	    uip->pqp = (priority_queue<TIMEMGR *>) p ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (uctimemgr_priqbegin) */


static int uctimemgr_priqend(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	delete uip->pqp ;
	uip->pqp = NULL ;
	return rs ;
}
/* end subroutine (uctimemgr_priqend) */


static int uctimemgr_timerbegin(UCTIMEMGR *uip)
{
	const int	cid = CLOCK_REALTIME ;
	timer_t		tid ;
	int		rs ;
	if ((rs = uc_timercreate(cid,NULL,&tid)) >= 0) {
	    uip->timerid = tid ;
	    uip->open.timer = TRUE ;
	}
	return rs ;
}
/* end subroutine (uctimemgr_timerbegin) */


static int uctimemgr_timerend(UCTIMEMGR *uip)
{
	timer_t		tid = uip->timerid ;
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = uc_timerdelete(tid) ;
	if (rs >= 0) rs = rs1 ;
	uip->timerid = 0 ;
	uip->open.timer = FALSE ;
	return rs ;
}
/* end subroutine (uctimemgr_timerend) */


static int uctimemgr_thrsbegin(UCTIMEMGR *uip)
{
	int		rs ;
	if ((rs = uctimemgr_thrcatchbegin(uip)) >= 0) {
	    rs = uctimemgr_thrdispbegin(uip) ;
	    if (rs < 0) {
		uctimemgr_thrcatchend(uip) ;
	    }
	}
	return rs ;
}
/* end subroutine (uctimemgr_thrsbegin) */


static int uctimemgr_thrsend(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	rs1 = uctimemgr_thrcatchend(uip) ;
	if (rs >= 0) rs = rs1 ;
	rs1 = uctimemgr_thrdispend(uip) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (uctimemgr_thrsend) */


static int uctimemgr_thrcatchbegin(UCTIMEMGR *uip)
{
	PTA		a ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&a)) >= 0) {
	    const int	scope = UCTIMEMGR_SCOPE ;
	    if ((rs = pta_setscope(&a,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) uctimemgr_thrcatchproc ;
		if ((rs = uptcreate(&tid,NULL,wt,uip)) >= 0) {
		    uip->f.running_catch = TRUE ;
		    uip->tid_catch = tid ;
		    f = TRUE ;
		} /* end if (pthread-create) */
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimemgr_thrcatchbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimemgr_thrcatchbegin) */


static int uctimemgr_thrcatchend(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->f.running_catch) {
	 	    pthread_t	tid = uip->tid_catch ;
		    int		trs ;
		    if ((rs = uptjoin(tid,&trs)) >= 0) {
		        uip->f.running_catch = FALSE ;
		        rs = trs ;
		    } else if (rs == SR_SRCH) {
		        uip->f.running_catch = FALSE ;
		        rs = SR_OK ;
		    }
	        } /* end if (uctimemgr_sendsync) */
	}
	return rs ;
}
/* end subroutine (uctimemgr_thrcatchend) */


static int uctimemgr_thrcatchproc(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	while ((rs = uctimemgr_sigwait(uip)) > 0) {
	   if (uip->f.thrcatch_exit) break ;


	} /* end while */
	return rs ;
}
/* end subroutine (uctimemgr_thrcatchproc) */


static int uctimemgr_sigwait(UCTIMEMGR *uip)
{
	siginfo_t	si ;
	int		rs ;
	while ((rs = uc_sigwaitinfo()) > 0) {
	   if (uip->f.thrcatch_exit) break ;


	return rs ;
}
/* end subroutine (uctimemgr_sigwait) */


static int uctimemgr_thrdispbegin(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	if (uip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (uctimemgr_thrdispbegin) */


static int uctimemgr_thrdispend(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	if (uip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (uctimemgr_thrdispend) */


static int uctimemgr_sendsync(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (! uip->f_syncing) {
	    SIGBLOCK	b ;
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
	        if ((rs = uctimemgr_init()) >= 0) {
		    if ((rs = uctimemgr_run(uip)) >= 0) {
		        const int	cmd = cmd_sync ;
		        rs = uctimemgr_cmdsend(uip,cmd) ;
		        c = uip->count ;
		    }
	        } /* end if (init) */
	        sigblock_finish(&b) ;
	    } /* end if (sigblock) */
	} /* end if (syncing not in progress) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimemgr_sendsync: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (uctimemgr_sendsync) */


static int uctimemgr_run(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (! uip->f_running) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	        if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    if (! uip->f_running) {
		        rs = uctimemgr_runner(uip) ;
		        f = rs ;
		    } else {
		        rs = uctimemgr_runcheck(uip) ;
		        f = rs ;
		    } /* end if (not running) */
	            rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} else {
	    rs = uctimemgr_runcheck(uip) ;
	    f = rs ;
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimemgr_run: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimemgr_run) */


static int uctimemgr_runcheck(UCTIMEMGR *uip)
{
	const pid_t	pid = getpid() ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (pid != uip->pid) {
		uip->f_running = FALSE ;
		uip->f_exiting = FALSE ;
		uip->pid = pid ;
		rs = uctimemgr_run(uip) ;
		f = rs ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimemgr_runcheck) */


static int uctimemgr_runner(UCTIMEMGR *uip)
{
	PTA		a ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&a)) >= 0) {
	    const int	scope = UCTIMEMGR_SCOPE ;
	    if ((rs = pta_setscope(&a,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) uctimemgr_worker ;
		if ((rs = uptcreate(&tid,NULL,wt,uip)) >= 0) {
		    uip->f_running = TRUE ;
		    uip->tid = tid ;
		    f = TRUE ;
		} /* end if (pthread-create) */
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimemgr_runner: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimemgr_runner) */


/* it always takes a good bit of code to make this part look easy! */
static int uctimemgr_worker(UCTIMEMGR *uip)
{
	int		rs ;

	while ((rs = uctimemgr_cmdrecv(uip)) > 0) {
	    switch (rs) {
	    case cmd_sync:
		rs = uctimemgr_worksync(uip) ;
		break ;
	    } /* end switch */
	    if (rs < 0) break ;
	} /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimemgr_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (uctimemgr_worker) */


static int uctimemgr_worksync(UCTIMEMGR *uip)
{
	int		rs ;
	uip->f_syncing = TRUE ;
	if ((rs = u_sync()) >= 0) {
	    uip->count += 1 ;
	}
	uip->f_syncing = FALSE ;
	return rs ;
}
/* end subroutine (uctimemgr_worksync) */


static int uctimemgr_cmdsend(UCTIMEMGR *uip,int cmd)
{
	int		rs ;
	int		rs1 ;
	int		to = 5 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    if (! uip->f_exiting) {
	        uip->waiters += 1 ;

	        while ((rs >= 0) && uip->f_cmd) {
		    rs = ptc_waiter(&uip->c,&uip->m,to) ;
	        } /* end while */

	        if (rs >= 0) {
	            uip->cmd = cmd ;
	            uip->f_cmd = TRUE ;
		    if (uip->waiters > 1) {
	                rs = ptc_signal(&uip->c) ;
		    }
	        }

	        uip->waiters -= 1 ;
	    } else {
		rs = SR_HANGUP ;
	    }
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */

	return rs ;
}
/* end subroutine (uctimemgr_cmdsend) */


static int uctimemgr_cmdrecv(UCTIMEMGR *uip)
{
	int		rs ;
	int		rs1 ;
	int		to = 1 ;
	int		cmd = 0 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    uip->waiters += 1 ;
	    to = -1 ;

	    while ((rs >= 0) && (! uip->f_cmd)) {
		rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

	    if (rs >= 0) {
	        cmd = uip->cmd ;
	        uip->f_cmd = FALSE ;
		if (uip->waiters > 1) {
	            rs = ptc_signal(&uip->c) ;
		}
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (uctimemgr_cmdrecv) */


static int uctimemgr_waitdone(UCTIMEMGR *uip)
{
	int		rs = SR_OK ;

	if (uip->f_running) {
	    const pid_t	pid = getpid() ;
	    if (pid == uip->pid) {
	        const int	cmd = cmd_exit ;
	        if ((rs = uctimemgr_cmdsend(uip,cmd)) >= 0) {
	 	    pthread_t	tid = uip->tid ;
		    int		trs ;
		    if ((rs = uptjoin(tid,&trs)) >= 0) {
		        uip->f_running = FALSE ;
		        rs = trs ;
		    } else if (rs == SR_SRCH) {
		        uip->f_running = FALSE ;
		        rs = SR_OK ;
		    }
	        } /* end if (uctimemgr_sendsync) */
	    } else {
		uip->f_running = FALSE ;
	    }
	} /* end if (running) */

	return rs ;
}
/* end subroutine (uctimemgr_waitdone) */


static void uctimemgr_atforkbefore()
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uctimemgr_atforkbefore) */


static void uctimemgr_atforkparent()
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uctimemgr_atforkparent) */


static void uctimemgr_atforkchild()
{
	UCTIMEMGR	*uip = &uctimemgr_data ;
	uip->f_running = FALSE ;
	uip->f_exiting = FALSE ;
	uip->pid = getpid() ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uctimemgr_atforkchild) */


