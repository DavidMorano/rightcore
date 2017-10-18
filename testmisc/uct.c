/* uc_timeout */
/* lang=C++11 */

/* UNIX® time-out management */


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

	int uc_timeout(int cmd,TIMEOUT *val)

	Arguments:

	cmd		command:
			    timeoutcmd_set,
			    timeoutcmd_cancel

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

#include	"timeout.h"


/* local defines */

#define	UCTIMEOUT	struct uctimeout
#define	UCTIMEOUT_FL	struct uctimeout_flags
#define	UCTIMEOUT_SCOPE	PTHREAD_SCOPE_SYSTEM

#define	NDF		"uctimeout.deb"


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

#ifndef	TYPEDEF_TWORKER
#define	TYPEDEF_TWORKER	1
typedef	int (*tworker)(void *) ;
#endif

struct uctimeout_flags {
	uint		timer:1 ;	/* UNIX-RT timer created */
	uint		working:1 ;
	uint		running_catch:1 ;
	uint		running_disp:1 ;
} ;

struct uctimeout {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	vechand		ents ;
	ciq		pass ;
	UCTIMEOUT_FL	open, f ;
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

extern "C" int	uctimeout_init() ;
extenr "C" void	uctimeout_fini() ;

static int	uctimeout_capbegin(UCTIMEOUT *,int) ;
static int	uctimeout_capend(UCTIMEOUT *) ;

static int	uctimeout_workready(UCTIMEOUT *) ;
static int	uctimeout_workbegin(UCTIMEOUT *) ;
static int	uctimeout_workend(UCTIMEOUT *) ;

static int	uctimeout_priqbegin(UCTIMEOUT *) ;
static int	uctimeout_priqend(UCTIMEOUT *) ;

static int	uctimeout_timerbegin(UCTIMEOUT *) ;
static int	uctimeout_timerend(UCTIMEOUT *) ;

static int	uctimeout_thrsbegin(UCTIMEOUT *) ;
static int	uctimeout_thrsend(UCTIMEOUT *) ;

static int	uctimeout_thrcatchbegin(UCTIMEOUT *) ;
static int	uctimeout_thrcatchend(UCTIMEOUT *) ;
static int	uctimeout_thrcatchproc(UCTIMEOUT *) ;

static int	uctimeout_thrdispbegin(UCTIMEOUT *) ;
static int	uctimeout_thrdispend(UCTIMEOUT *) ;

static int	uctimeout_sendsync(UCTIMEOUT *) ;
static int	uctimeout_run(UCTIMEOUT *) ;
static int	uctimeout_runcheck(UCTIMEOUT *) ;
static int	uctimeout_runner(UCTIMEOUT *) ;
static int	uctimeout_worker(UCTIMEOUT *) ;
static int	uctimeout_worksync(UCTIMEOUT *) ;
static int	uctimeout_cmdsend(UCTIMEOUT *,int) ;
static int	uctimeout_cmdrecv(UCTIMEOUT *) ;
static int	uctimeout_waitdone(UCTIMEOUT *) ;

static void	uctimeout_atforkbefore() ;
static void	uctimeout_atforkparent() ;
static void	uctimeout_atforkchild() ;


/* local variables */

static UCTIMEOUT	uctimeout_data ; /* zero-initialized */


/* exported subroutines */


int uctimeout_init()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = uctimeout_atforkbefore ;
	    	    void	(*ap)() = uctimeout_atforkparent ;
	    	    void	(*ac)() = uctimeout_atforkchild ;
	            if ((rs = uc_atfork(b,ap,ac)) >= 0) {
	                if ((rs = uc_atexit(uctimeout_fini)) >= 0) {
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
/* end subroutine (uctimeout_init) */


void uctimeout_fini()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    uctimeout_waitdone(uip) ;
	    {
	        void	(*b)() = uctimeout_atforkbefore ;
	        void	(*ap)() = uctimeout_atforkparent ;
	        void	(*ac)() = uctimeout_atforkchild ;
	        uc_atforkrelease(b,ap,ac) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UCTIMEOUT)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (uctimeout_fini) */


int uc_timeout(int cmd,TIMEOUT *valp)
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	int		rs ;

	if ((rs = uctimeout_init()) >= 0) {
	    if ((rs = uctimeout_capbegin(uip,-1)) >= 0) {
		if ((rs = uctimeout_workready(uip)) >= 0) {
	            switch (cmd) {
	            case timeoutcmd_set:
	                rs = uctimeout_cmdset(valp) ;
	                break ;
	            case timeoutcmd_cancel:
	                rs = uctimeout_cmdcancel(valp) ;
	                break ;
		    default:
		        rs = SR_INVALID ;
		        break ;
	            } /* end switch */
		} /* end if (uctimeout_workready) */
	        rs1 = uctimeout_capend(uip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (uctimeout-cap) */
	} /* end if (uctimeout_init) */

	return rs ;
}
/* end subroutine (uc_timeout) */


/* local subroutines */


static int uctimeout_capbegin(UCTIMEOUT *uip,int to)
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
/* end subroutine (uctimeout_capbegin) */


static int uctimeout_capend(UCTIMEOUT *uip)
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
/* end subroutine (uctimeout_capend) */


static int uctimeout_workready(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if (! uip->f_workready) {
	    rs = uctimeout_workbegin(uip) ;
	}
	return rs ;
}
/* end subroutine (uctimeout_workready) */


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

static int uctimeout_workbegin(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (! uip->f_workready) {
	    int	vo = 0 ;
	    so |= (VECHAND_OSTATIONARY|VECHAND_OREUSE| VECHAND_OCOMPACT) ;
	    vo |= ( VECHAND_OSWAP| VECHAND_OCONSERVE) ;
	    if ((rs = vechand_start(&uip->ents,0,vo)) >= 0) {
		if ((rs = uctimeout_priqbegin(uip)) >= 0) {
		    if ((rs = uctimeout_timerbegin(uip)) >= 0) {
			if ((rs = ciq_start(&uip->pass)) >= 0) {
			    if ((rs = uctimeout_thrsbegin(uip)) >= 0) {
			        uip->open.working = TRUE ;
			    }
			    if (rs < 0) {
			        ciq_finish(&uip->pass) ;
			    }
			}
			if (rs < 0) {
		    	    uctimeout_timerend(uip) ;
			}
		    }
		    if (rs < 0) {
			uctimeout_priqend(uip) ;
		    }
	        }
		if (rs < 0) {
		    vechand_finish(&uip->ents) ;
		}
	    } /* end if (vechand_start) */
	}
	return rs ;
}
/* end subroutine (uctimeout_workbegin) */


static int uctimeout_workend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->open.working) {
	    rs1 = uctimeout_thrsend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = ciq_finish(&uip->pass) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uctimeout_timerend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uctimeout_priqend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = vechand_finish(&uip->ents) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->open.working = FALSE ;
	}
	return rs ;
}
/* end subroutine (uctimeout_workend) */


static int uctimeout_priqbegin(UCTIMEOUT *uip)
{
	void		*p ;
	int		rs = SR_OK ;
	p = new(nothrow) priority_queue<TIMEOUT *,vector<TIMEOUT *>,ourcmp> ;
	if (p != NULL) {
	    uip->pqp = (priority_queue<TIMEOUT *>) p ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (uctimeout_priqbegin) */


static int uctimeout_priqend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	delete uip->pqp ;
	uip->pqp = NULL ;
	return rs ;
}
/* end subroutine (uctimeout_priqend) */


static int uctimeout_timerbegin(UCTIMEOUT *uip)
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
/* end subroutine (uctimeout_timerbegin) */


static int uctimeout_timerend(UCTIMEOUT *uip)
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
/* end subroutine (uctimeout_timerend) */


static int uctimeout_thrsbegin(UCTIMEOUT *uip)
{
	int		rs ;
	if ((rs = uctimeout_thrcatchbegin(uip)) >= 0) {
	    rs = uctimeout_thrdispbegin(uip) ;
	    if (rs < 0) {
		uctimeout_thrcatchend(uip) ;
	    }
	}
	return rs ;
}
/* end subroutine (uctimeout_thrsbegin) */


static int uctimeout_thrsend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	rs1 = uctimeout_thrcatchend(uip) ;
	if (rs >= 0) rs = rs1 ;
	rs1 = uctimeout_thrdispend(uip) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (uctimeout_thrsend) */


static int uctimeout_thrcatchbegin(UCTIMEOUT *uip)
{
	PTA		a ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&a)) >= 0) {
	    const int	scope = UCTIMEOUT_SCOPE ;
	    if ((rs = pta_setscope(&a,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) uctimeout_thrcatchproc ;
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
	nprintf(NDF,"uctimeout_thrcatchbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_thrcatchbegin) */


static int uctimeout_thrcatchend(UCTIMEOUT *uip)
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
	        } /* end if (uctimeout_sendsync) */
	}
	return rs ;
}
/* end subroutine (uctimeout_thrcatchend) */


static int uctimeout_thrcatchproc(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	while ((rs = uctimeout_sigwait(uip)) > 0) {
	   if (uip->f.thrcatch_exit) break ;


	} /* end while */
	return rs ;
}
/* end subroutine (uctimeout_thrcatchproc) */


static int uctimeout_sigwait(UCTIMEOUT *uip)
{
	siginfo_t	si ;
	int		rs ;
	while ((rs = uc_sigwaitinfo()) > 0) {
	   if (uip->f.thrcatch_exit) break ;


	return rs ;
}
/* end subroutine (uctimeout_sigwait) */


static int uctimeout_thrdispbegin(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if (uip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (uctimeout_thrdispbegin) */


static int uctimeout_thrdispend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if (uip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (uctimeout_thrdispend) */


static int uctimeout_sendsync(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (! uip->f_syncing) {
	    SIGBLOCK	b ;
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
	        if ((rs = uctimeout_init()) >= 0) {
		    if ((rs = uctimeout_run(uip)) >= 0) {
		        const int	cmd = cmd_sync ;
		        rs = uctimeout_cmdsend(uip,cmd) ;
		        c = uip->count ;
		    }
	        } /* end if (init) */
	        sigblock_finish(&b) ;
	    } /* end if (sigblock) */
	} /* end if (syncing not in progress) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sendsync: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (uctimeout_sendsync) */


static int uctimeout_run(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (! uip->f_running) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	        if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    if (! uip->f_running) {
		        rs = uctimeout_runner(uip) ;
		        f = rs ;
		    } else {
		        rs = uctimeout_runcheck(uip) ;
		        f = rs ;
		    } /* end if (not running) */
	            rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} else {
	    rs = uctimeout_runcheck(uip) ;
	    f = rs ;
	} /* end if (not-running) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_run: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_run) */


static int uctimeout_runcheck(UCTIMEOUT *uip)
{
	const pid_t	pid = getpid() ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (pid != uip->pid) {
		uip->f_running = FALSE ;
		uip->f_exiting = FALSE ;
		uip->pid = pid ;
		rs = uctimeout_run(uip) ;
		f = rs ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_runcheck) */


static int uctimeout_runner(UCTIMEOUT *uip)
{
	PTA		a ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = pta_create(&a)) >= 0) {
	    const int	scope = UCTIMEOUT_SCOPE ;
	    if ((rs = pta_setscope(&a,scope)) >= 0) {
		pthread_t	tid ;
		tworker		wt = (tworker) uctimeout_worker ;
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
	nprintf(NDF,"uctimeout_runner: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_runner) */


/* it always takes a good bit of code to make this part look easy! */
static int uctimeout_worker(UCTIMEOUT *uip)
{
	int		rs ;

	while ((rs = uctimeout_cmdrecv(uip)) > 0) {
	    switch (rs) {
	    case cmd_sync:
		rs = uctimeout_worksync(uip) ;
		break ;
	    } /* end switch */
	    if (rs < 0) break ;
	} /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_worker: ret rs=%d\n",rs) ;
#endif

	uip->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (uctimeout_worker) */


static int uctimeout_worksync(UCTIMEOUT *uip)
{
	int		rs ;
	uip->f_syncing = TRUE ;
	if ((rs = u_sync()) >= 0) {
	    uip->count += 1 ;
	}
	uip->f_syncing = FALSE ;
	return rs ;
}
/* end subroutine (uctimeout_worksync) */


static int uctimeout_cmdsend(UCTIMEOUT *uip,int cmd)
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
/* end subroutine (uctimeout_cmdsend) */


static int uctimeout_cmdrecv(UCTIMEOUT *uip)
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
/* end subroutine (uctimeout_cmdrecv) */


static int uctimeout_waitdone(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;

	if (uip->f_running) {
	    const pid_t	pid = getpid() ;
	    if (pid == uip->pid) {
	        const int	cmd = cmd_exit ;
	        if ((rs = uctimeout_cmdsend(uip,cmd)) >= 0) {
	 	    pthread_t	tid = uip->tid ;
		    int		trs ;
		    if ((rs = uptjoin(tid,&trs)) >= 0) {
		        uip->f_running = FALSE ;
		        rs = trs ;
		    } else if (rs == SR_SRCH) {
		        uip->f_running = FALSE ;
		        rs = SR_OK ;
		    }
	        } /* end if (uctimeout_sendsync) */
	    } else {
		uip->f_running = FALSE ;
	    }
	} /* end if (running) */

	return rs ;
}
/* end subroutine (uctimeout_waitdone) */


static void uctimeout_atforkbefore()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uctimeout_atforkbefore) */


static void uctimeout_atforkparent()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uctimeout_atforkparent) */


static void uctimeout_atforkchild()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	uip->f_running = FALSE ;
	uip->f_exiting = FALSE ;
	uip->pid = getpid() ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uctimeout_atforkchild) */


