/* uc_timeout */
/* lang=C++11 */

/* UNIX® time-out management */


#define	CF_DEBUGN	0		/* special debugging */
#define	CF_CHILDTHRS	0		/* start threads in child process */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

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
#include	<ucontext.h>
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
#include	<vecsorthand.h>		/* vector-sorted-handles */
#include	<ciq.h>			/* container-interlocked-queue */
#include	<timespec.h>
#include	<itimerspec.h>
#include	<sigevent.h>
#include	<localmisc.h>

#include	"timeout.h"


/* local defines */

#define	UCTIMEOUT	struct uctimeout
#define	UCTIMEOUT_FL	struct uctimeout_flags
#define	UCTIMEOUT_SCOPE	PTHREAD_SCOPE_SYSTEM

#define	NDF		"uctimeout.deb"


/* typedefs */

#ifndef	TYPEDEF_SIGINFOHAND
#define	TYPEDEF_SIGINFOHAND	1
typedef void (*siginfohand_t)(int,siginfo_t *,void *) ;
#endif

typedef vecsorthand	prique ;


/* external subroutines */

extern "C" int	uc_timeout(int cmd,TIMEOUT *valp) ;

extern "C" int	msleep(int) ;

#if	CF_DEBUGN
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern "C" int	nprintf(cchar *,cchar *,...) ;
#endif

extern "C" cchar	*strsigabbr(int) ;


/* local structures */

#ifndef	TYPEDEF_TWORKER
#define	TYPEDEF_TWORKER		1
typedef	int	(*tworker)(void *) ;
#endif

struct uctimeout_flags {
	uint		timer:1 ;	/* UNIX-RT timer created */
	uint		workready:1 ;
	uint		thrs:1 ;
	uint		wasblocked:1 ;
	uint		running_siger:1 ;
	uint		running_disp:1 ;
} ;

struct uctimeout {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	vechand		ents ;
	CIQ		pass ;
	UCTIMEOUT_FL	f ;
	vecsorthand	*pqp ;
	sigset_t	savemask ;
	pid_t		pid ;
	pthread_t	tid_siger ;
	pthread_t	tid_disp ;
	timer_t		timerid ;
	volatile int	waiters ;	/* n-waiters for general capture */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
	volatile int	f_capture ;	/* capture flag */
	volatile int	f_thrsiger ;	/* thread running (siger) */
	volatile int	f_thrdisp ;	/* thread running (disp) */
	volatile int	f_cmd ;
	volatile int	f_reqexit ;	/* request exit of threads */
	volatile int	f_exitsiger ;	/* thread is exiting */
	volatile int	f_exitdisp ;	/* thread is exiting */
} ;

enum dispcmds {
	dispcmd_exit,
	dispcmd_timeout,
	dispcmd_handle,
	dispcmd_overlast

} ;


/* forward references */

extern "C" int	uctimeout_init() ;
extern "C" void	uctimeout_fini() ;

static int	uctimeout_cmdset(UCTIMEOUT *,TIMEOUT *) ;
static int	uctimeout_cmdcancel(UCTIMEOUT *,TIMEOUT *) ;

static int	uctimeout_enterpri(UCTIMEOUT *,TIMEOUT *) ;
static int	uctimeout_timerset(UCTIMEOUT *,time_t) ;

static int	uctimeout_capbegin(UCTIMEOUT *,int) ;
static int	uctimeout_capend(UCTIMEOUT *) ;

static int	uctimeout_workready(UCTIMEOUT *) ;
static int	uctimeout_workbegin(UCTIMEOUT *) ;
static int	uctimeout_workend(UCTIMEOUT *) ;
static int	uctimeout_workfins(UCTIMEOUT *) ;

static int	uctimeout_priqbegin(UCTIMEOUT *) ;
static int	uctimeout_priqend(UCTIMEOUT *) ;

static int	uctimeout_sigbegin(UCTIMEOUT *) ;
static int	uctimeout_sigend(UCTIMEOUT *) ;

static int	uctimeout_timerbegin(UCTIMEOUT *) ;
static int	uctimeout_timerend(UCTIMEOUT *) ;

static int	uctimeout_thrsbegin(UCTIMEOUT *) ;
static int	uctimeout_thrsend(UCTIMEOUT *) ;

static int	uctimeout_sigerbegin(UCTIMEOUT *) ;
static int	uctimeout_sigerend(UCTIMEOUT *) ;
static int	uctimeout_sigerworker(UCTIMEOUT *) ;

static int	uctimeout_dispbegin(UCTIMEOUT *) ;
static int	uctimeout_dispend(UCTIMEOUT *) ;
static int	uctimeout_dispworker(UCTIMEOUT *) ;
static int	uctimeout_cmdrecv(UCTIMEOUT *) ;

static int	uctimeout_disphandle(UCTIMEOUT *) ;
static int	uctimeout_dispjobdel(UCTIMEOUT *,TIMEOUT *) ;

static int	uctimeout_sigerwait(UCTIMEOUT *) ;
static int	uctimeout_sigerserve(UCTIMEOUT *) ;

static void	uctimeout_atforkbefore() ;
static void	uctimeout_atforkparent() ;
static void	uctimeout_atforkchild() ;

static int	ourcmp(const void *,const void *) ;


/* local variables */

static UCTIMEOUT	uctimeout_data ; /* zero-initialized */


/* exported subroutines */


int uctimeout_init()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_init: ent\n") ;
#endif
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
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_init: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_init) */


void uctimeout_fini()
{
	UCTIMEOUT	*uip = &uctimeout_data ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_fini: ent\n") ;
#endif
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    uctimeout_workend(uip) ;
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
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_fini: ret\n") ;
#endif
}
/* end subroutine (uctimeout_fini) */


int uc_timeout(int cmd,TIMEOUT *valp)
{
	UCTIMEOUT	*uip = &uctimeout_data ;
	int		rs ;
	int		rs1 ;
	int		id = 0 ;

#if	CF_DEBUGN
	nprintf(NDF,"uc_timeout: ent cmd=%u\n",cmd) ;
	nprintf(NDF,"uc_timeout: sigtimeout=%u\n",SIGTIMEOUT) ;
	nprintf(NDF,"uc_timeout: siger=%p\n",uctimeout_sigerworker) ;
	nprintf(NDF,"uc_timeout: disp=%p\n",uctimeout_dispworker) ;
#endif

	if (valp == NULL) return SR_FAULT ;
	if (cmd < 0) return SR_INVALID ;

	if ((rs = uctimeout_init()) >= 0) {
	    if ((rs = uctimeout_capbegin(uip,-1)) >= 0) {
	        if ((rs = uctimeout_workready(uip)) >= 0) {
	            switch (cmd) {
	            case timeoutcmd_set:
	                rs = uctimeout_cmdset(uip,valp) ;
	                id = rs ;
	                break ;
	            case timeoutcmd_cancel:
	                rs = uctimeout_cmdcancel(uip,valp) ;
	                id = rs ;
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

#if	CF_DEBUGN
	nprintf(NDF,"uc_timeout: ret rs=%d id=%u\n",rs,id) ;
#endif

	return (rs >= 0) ? id : rs ;
}
/* end subroutine (uc_timeout) */


/* local subroutines */


static int uctimeout_cmdset(UCTIMEOUT *uip,TIMEOUT *valp)
{
	TIMEOUT		*ep ;
	const int	esize = sizeof(TIMEOUT) ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_cmdset: ent val=%u\n",valp->val) ;
#endif
	if (valp->metp == NULL) return SR_FAULT ;
	if ((rs = uc_libmalloc(esize,&ep)) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
	        vechand		*elp = &uip->ents ;
	        if ((rs = vechand_add(elp,ep)) >= 0) {
	            const int	ei = rs ;
	            *ep = *valp ;
	            {
	                ep->id = ei ;
	                rs = uctimeout_enterpri(uip,ep) ;
	            }
	            if (rs < 0)
	                vechand_del(elp,ei) ;
	        } /* end if (vechand_add) */
	        rs1 = ptm_unlock(&uip->m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ptm) */
	    if (rs < 0)
	        uc_libfree(ep) ;
	} /* end if (m-a) */
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_cmdset: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (uctimeout_cmdset) */


static int uctimeout_cmdcancel(UCTIMEOUT *uip,TIMEOUT *valp)
{
	const int	id = valp->id ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(&uip->m)) >= 0) {
	    TIMEOUT	*ep ;
	    vechand	*elp = &uip->ents ;
	    if ((rs = vechand_get(elp,id,&ep)) >= 0) {
	        const int	ei = rs ;
	        if ((rs = vechand_del(elp,ei)) >= 0) {
	        	prique 		*pqp = uip->pqp ;
	                const int	nrs = SR_NOTFOUND ;
			int		f_free = FALSE ;
	                if ((rs = vecsorthand_delhand(pqp,ep)) >= 0) {
			    f_free = TRUE ;
			} else if (rs == nrs) {
	                    CIQ		*cqp = &uip->pass ;
	                    if ((rs = ciq_remhand(cqp,ep)) >= 0) {
				f_free = TRUE ;
			    } else if (rs == nrs) {
	                        rs = SR_OK ;
	                    }
			}
			if ((rs >= 0) && f_free) {
	            	    rs = uc_libfree(ep) ;
			}
	        } /* end if (vechand_del) */
	    } /* end if (vechand_get) */
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (uctimeout_cmdcancel) */


static int uctimeout_enterpri(UCTIMEOUT *uip,TIMEOUT *ep)
{
	prique		*pqp = uip->pqp ;
	int		rs ;
	int		pi = 0 ;
	if ((rs = vecsorthand_count(pqp)) > 0) {
	    TIMEOUT	*tep ;
	    if ((rs = vecsorthand_get(pqp,0,&tep)) >= 0) {
	        if (ep->val < tep->val) {
	            if ((rs = vecsorthand_add(pqp,ep)) >= 0) {
	                pi = rs ;
	                rs = uctimeout_timerset(uip,ep->val) ;
	                if (rs < 0)
	                    vecsorthand_del(pqp,pi) ;
	            }
	        } else {
	            rs = vecsorthand_add(pqp,ep) ;
	            pi = rs ;
	        }
	    } /* end if (vecsorthand_get) */
	} else {
	    if ((rs = vecsorthand_add(pqp,ep)) >= 0) {
	        pi = rs ;
	        rs = uctimeout_timerset(uip,ep->val) ;
	        if (rs < 0)
	            vecsorthand_del(pqp,pi) ;
	    } /* end if (vecsorthand_add) */
	}
	return (rs >= 0) ? pi : rs ;
}
/* end subroutine (uctimeout_enterpri) */


static int uctimeout_timerset(UCTIMEOUT *uip,time_t val)
{
	TIMESPEC	ts ;
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_timerset: ent val=%u\n",val) ;
#endif
	if ((rs = timespec_load(&ts,val,0)) >= 0) {
	    ITIMERSPEC	it ;
	    if ((rs = itimerspec_load(&it,&ts,NULL)) >= 0) {
	        const timer_t	timerid = uip->timerid ;
	        const int	tf = TIMER_ABSTIME ;
	        rs = uc_timerset(timerid,tf,&it,NULL) ;
	    }
	}
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_timerset: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (uctimeout_timerset) */


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
	if (! uip->f.workready) {
	    rs = uctimeout_workbegin(uip) ;
	}
	if ((rs >= 0) && (! uip->f.thrs)) {
	    rs = uctimeout_thrsbegin(uip) ;
	}
	return rs ;
}
/* end subroutine (uctimeout_workready) */


static int uctimeout_workbegin(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if (! uip->f.workready) {
	    int		vo = 0 ;
	    vo |= (VECHAND_OSTATIONARY|VECHAND_OREUSE| VECHAND_OCOMPACT) ;
	    vo |= ( VECHAND_OSWAP| VECHAND_OCONSERVE) ;
	    if ((rs = vechand_start(&uip->ents,0,vo)) >= 0) {
	        if ((rs = uctimeout_priqbegin(uip)) >= 0) {
	            if ((rs = uctimeout_sigbegin(uip)) >= 0) {
	                if ((rs = uctimeout_timerbegin(uip)) >= 0) {
	                    if ((rs = ciq_start(&uip->pass)) >= 0) {
	                        if ((rs = uctimeout_thrsbegin(uip)) >= 0) {
	                            uip->f.workready = TRUE ;
	                        }
	                        if (rs < 0) {
	                            ciq_finish(&uip->pass) ;
	                        }
	                    }
	                    if (rs < 0) {
	                        uctimeout_timerend(uip) ;
	                    }
	                } /* end if (uctimeout_timerbegin) */
	                if (rs < 0)
	                    uctimeout_sigend(uip) ;
	            } /* end if (uctimeout_sigbegin) */
	            if (rs < 0) {
	                uctimeout_priqend(uip) ;
	            }
	        } /* end if (uctimeout_pribegin) */
	        if (rs < 0) {
	            vechand_finish(&uip->ents) ;
	        }
	    } /* end if (vechand_start) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (uctimeout_workbegin) */


static int uctimeout_workend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_workend: ent\n") ;
#endif
	if (uip->f.workready) {
	    rs1 = uctimeout_thrsend(uip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_workend: mid1 rs=%d\n",rs) ;
#endif
	    rs1 = ciq_finish(&uip->pass) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_workend: mid2 rs=%d\n",rs) ;
#endif
	    rs1 = uctimeout_timerend(uip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_workend: mid3 rs=%d\n",rs) ;
#endif
	    rs1 = uctimeout_sigend(uip) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = uctimeout_priqend(uip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_workend: mid4 rs=%d\n",rs) ;
#endif
	    rs1 = uctimeout_workfins(uip) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_workend: mid5 rs=%d\n",rs) ;
#endif
	    rs1 = vechand_finish(&uip->ents) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->f.workready = FALSE ;
	}
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_workend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (uctimeout_workend) */


static int uctimeout_workfins(UCTIMEOUT *uip)
{
	vechand		*elp = &uip->ents ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	void		*top ;
	for (i = 0 ; vechand_get(elp,i,&top) >= 0 ; i += 1) {
	    if (top != NULL) {
	        rs1 = uc_libfree(top) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (uctimeout_workfins) */


static int uctimeout_priqbegin(UCTIMEOUT *uip)
{
	const int	osize = sizeof(vecsorthand) ;
	int		rs ;
	void		*p ;
	if ((rs = uc_libmalloc(osize,&p)) >= 0) {
	    prique	*pqp = (prique *) p ;
	    uip->pqp = (prique *) p ;
	    rs = vecsorthand_start(pqp,1,ourcmp) ;
	    if (rs < 0) {
	        uc_libfree(uip->pqp) ;
	        uip->pqp = NULL ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (uctimeout_priqbegin) */


static int uctimeout_priqend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	{
	    rs1 = vecsorthand_finish(uip->pqp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	{
	    rs1 = uc_libfree(uip->pqp) ;
	    if (rs >= 0) rs = rs1 ;
	    uip->pqp = NULL ;
	}
	return rs ;
}
/* end subroutine (uctimeout_priqend) */


static int uctimeout_sigbegin(UCTIMEOUT *uip)
{
	sigset_t	ss, oss ;
	const int	scmd = SIG_BLOCK ;
	const int	sig = SIGTIMEOUT ;
	int		rs ;
	uc_sigsetempty(&ss) ;
	uc_sigsetadd(&ss,sig) ;
	if ((rs = pt_sigmask(scmd,&ss,&oss)) >= 0) {
	    if ((rs = uc_sigsetismem(&ss,sig)) > 0) {
	        uip->f.wasblocked = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (uctimeout_sigbegin) */


static int uctimeout_sigend(UCTIMEOUT *uip)
{
	sigset_t	ss ;
	int		rs = SR_OK ;
	if (! uip->f.wasblocked) {
	    const int	scmd = SIG_UNBLOCK ;
	    const int	sig = SIGTIMEOUT ;
	    uc_sigsetempty(&ss) ;
	    uc_sigsetadd(&ss,sig) ;
	    rs = pt_sigmask(scmd,&ss,NULL) ;
	}
	return rs ;
}
/* end subroutine (uctimeout_sigend) */


static int uctimeout_timerbegin(UCTIMEOUT *uip)
{
	SIGEVENT	se ;
	const int	st = SIGEV_SIGNAL ;
	const int	sig = SIGTIMEOUT ;
	const int	val = 0 ; /* we do not (really) care about this */
	int		rs ;
	if ((rs = sigevent_load(&se,st,sig,val)) >= 0) {
	    const int	cid = CLOCK_REALTIME ;
	    timer_t	tid ;
	    if ((rs = uc_timercreate(cid,&se,&tid)) >= 0) {
	        uip->timerid = tid ;
	        uip->f.timer = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (uctimeout_timerbegin) */


static int uctimeout_timerend(UCTIMEOUT *uip)
{
	timer_t		tid = uip->timerid ;
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = uc_timerdestroy(tid) ;
	if (rs >= 0) rs = rs1 ;
	uip->timerid = 0 ;
	uip->f.timer = FALSE ;
	return rs ;
}
/* end subroutine (uctimeout_timerend) */


static int uctimeout_thrsbegin(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if ((! uip->f.thrs) && (! uip->f_reqexit)) {
	    if ((rs = uctimeout_sigerbegin(uip)) >= 0) {
	        if ((rs = uctimeout_dispbegin(uip)) >= 0) {
	            uip->f.thrs = TRUE ;
	        }
	        if (rs < 0) {
	            uctimeout_sigerend(uip) ;
	        }
	    }
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (uctimeout_thrsbegin) */


static int uctimeout_thrsend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (uip->f.thrs) {
	    uip->f.thrs = FALSE ;
	    uip->f_reqexit = TRUE ;
	    rs1 = uctimeout_dispend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uctimeout_sigerend(uip) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (uctimeout_thrsend) */


static int uctimeout_sigerbegin(UCTIMEOUT *uip)
{
	PTA		ta ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerbegin: ent\n") ;
#endif

	if ((rs = pta_create(&ta)) >= 0) {
	    const int	scope = UCTIMEOUT_SCOPE ;
	    if ((rs = pta_setscope(&ta,scope)) >= 0) {
	        pthread_t	tid ;
	        tworker	wt = (tworker) uctimeout_sigerworker ;
	        if ((rs = uptcreate(&tid,&ta,wt,uip)) >= 0) {
	            uip->f.running_siger = TRUE ;
	            uip->tid_siger = tid ;
	            f = TRUE ;
	        } /* end if (pthread-create) */
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&ta) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_sigerbegin) */


static int uctimeout_sigerend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerend: ent\n") ;
#endif
	if (uip->f.running_siger) {
	    pthread_t	tid = uip->tid_siger ;
	    const int	sig = SIGTIMEOUT ;
	    if ((rs = uptkill(tid,sig)) >= 0) {
	        int	trs ;
#if	CF_DEBUGN
	        nprintf(NDF,"uctimeout_sigerend: uptkill() rs=%d\n",rs) ;
#endif
	        uip->f.running_siger = FALSE ;
	        if ((rs = uptjoin(tid,&trs)) >= 0) {
	            rs = trs ;
	        } else if (rs == SR_SRCH) {
	            rs = SR_OK ;
	        }
	    } /* end if (uptkill) */
	}
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (uctimeout_sigerend) */


/* this is an independent thread of execution */
static int uctimeout_sigerworker(UCTIMEOUT *uip)
{
	int		rs ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerworker: ent\n") ;
#endif
	while ((rs = uctimeout_sigerwait(uip)) > 0) {
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_sigerworker: _sigerwait() rs=%d\n",rs) ;
#endif
	    if (uip->f_reqexit) break ;
	    switch (rs) {
	    case 1:
	        rs = uctimeout_sigerserve(uip) ;
	        break ;
	    }
	    if (rs < 0) break ;
	} /* end while */
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerworker: ret rs=%d\n",rs) ;
#endif
	uip->f_exitsiger = TRUE ;
	return rs ;
}
/* end subroutine (uctimeout_sigerworker) */


static int uctimeout_sigerwait(UCTIMEOUT *uip)
{
	TIMESPEC	ts ;
	sigset_t	ss ;
	siginfo_t	si ;
	const int	sig = SIGTIMEOUT ;
	const int	to = 5 ;
	int		rs ;
	int		cmd = 0 ;
	int		f_exit = FALSE ;
	int		f_timedout = FALSE ;
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerwait: ent\n") ;
#endif
	uc_sigsetempty(&ss) ;
	uc_sigsetadd(&ss,sig) ;
	uc_sigsetadd(&ss,SIGALRM) ;
	timespec_load(&ts,to,0) ;
	repeat {
	    rs = uc_sigwaitinfoto(&ss,&si,&ts) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_INTR:
	            break ;
	        case SR_AGAIN:
	            f_timedout = TRUE ;
	            rs = SR_OK ; /* will cause exit from loop */
	            break ;
	        default:
	            f_exit = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;
	if (rs >= 0) {
	    if (! uip->f_reqexit) {
	        if (f_timedout) {
	            cmd = 2 ;
	        } else if (sig == si.si_signo) {
	            cmd = 1 ;
	        }
	    } /* end if (not exiting) */
	} /* end if (ok) */
#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_sigerwait: ret rs=%d cmd=%u\n",rs,cmd) ;
#endif
	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (uctimeout_sigerwait) */


static int uctimeout_sigerserve(UCTIMEOUT *uip)
{
	const int	to = 60 ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    vecsorthand		*pqp = uip->pqp ;
	    const time_t	dt = time(NULL) ;
	    while ((rs = vecsorthand_count(pqp)) > 0) {
	        TIMEOUT		*tep ;
	        if ((rs = vecsorthand_get(pqp,0,&tep)) >= 0) {
	            const int	ei = rs ;
	            if (tep->val > dt) break ;
	            if ((rs = vecsorthand_del(pqp,ei)) >= 0) {
	                if ((rs = ciq_ins(&uip->pass,tep)) >= 0) {
	                    uip->f_cmd = TRUE ;
	                    rs = ptc_signal(&uip->c) ;
	                }
	            }
	        }
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (uctimeout_sigerserve) */


static int uctimeout_dispbegin(UCTIMEOUT *uip)
{
	PTA		ta ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_dispbegin: ent\n") ;
#endif

	if ((rs = pta_create(&ta)) >= 0) {
	    const int	scope = UCTIMEOUT_SCOPE ;
	    if ((rs = pta_setscope(&ta,scope)) >= 0) {
	        pthread_t	tid ;
	        tworker		wt = (tworker) uctimeout_dispworker ;
	        if ((rs = uptcreate(&tid,&ta,wt,uip)) >= 0) {
	            uip->f.running_disp = TRUE ;
	            uip->tid_disp = tid ;
	            f = TRUE ;
	        } /* end if (uptcreate) */
	    } /* end if (pta-setscope) */
	    rs1 = pta_destroy(&ta) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pta) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_dispbegin: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_dispbegin) */


static int uctimeout_dispend(UCTIMEOUT *uip)
{
	int		rs = SR_OK ;
	if (uip->f.running_disp) {
	    pthread_t	tid = uip->tid_disp ;
	    int		trs ;
	    uip->f.running_disp = FALSE ;
	    if ((rs = uptjoin(tid,&trs)) >= 0) {
	        rs = trs ;
	    } else if (rs == SR_SRCH) {
	        rs = SR_OK ;
	    }
	}
	return rs ;
}
/* end subroutine (uctimeout_dispend) */


/* it always takes a good bit of code to make this part look easy! */
static int uctimeout_dispworker(UCTIMEOUT *uip)
{
	int		rs ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_dispworker: ent\n") ;
#endif

	while ((rs = uctimeout_cmdrecv(uip)) > 0) {
#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_dispworker: _cmdrecv() rs=%d\n",rs) ;
#endif
	    switch (rs) {
	    case dispcmd_timeout:
#if	CF_DEBUGN
	        nprintf(NDF,"uctimeout_dispworker: timeout\n") ;
#endif
	        break ;
	    case dispcmd_handle:
	        rs = uctimeout_disphandle(uip) ;
	        break ;
	    } /* end switch */
	    if (rs < 0) break ;
	} /* end while (looping on commands) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_dispworker: ret rs=%d\n",rs) ;
#endif

	uip->f_exitdisp = TRUE ;
	return rs ;
}
/* end subroutine (uctimeout_dispworker) */


static int uctimeout_disphandle(UCTIMEOUT *uip)
{
	TIMEOUT		*tep ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_disphandle: ent\n") ;
#endif

	while ((rs1 = ciq_rem(&uip->pass,&tep)) >= 0) {
	    if ((rs = uctimeout_dispjobdel(uip,tep)) > 0) {
	        timeout_met	met = (timeout_met) tep->metp ;
	        rs = (*met)(tep->objp,tep->tag,tep->arg) ;
	        uc_libfree(tep) ;
	    }
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && (rs1 != SR_EMPTY)) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_disphandle: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uctimeout_disphandle) */


static int uctimeout_dispjobdel(UCTIMEOUT *uip,TIMEOUT *tep)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	if ((rs = ptm_lockto(&uip->m,-1)) >= 0) {
	    if ((rs = vechand_delhand(&uip->ents,tep)) >= 0) {
		f = TRUE ;
	    } else if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
	    }
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uctimeout_dispjobdel) */


static int uctimeout_cmdrecv(UCTIMEOUT *uip)
{
	int		rs ;
	int		rs1 ;
	int		to = 2 ;
	int		cmd = dispcmd_exit ;

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_cmdrecv: ent\n") ;
#endif

	if ((rs = ptm_lockto(&uip->m,-1)) >= 0) {
	    uip->waiters += 1 ;

	    while ((rs >= 0) && (! uip->f_cmd)) {
	        rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

#if	CF_DEBUGN
	    nprintf(NDF,"uctimeout_cmdrecv: mid2 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        uip->f_cmd = FALSE ;
	        if (! uip->f_reqexit) cmd = dispcmd_handle ;
	        if (uip->waiters > 1) {
	            rs = ptc_signal(&uip->c) ;
	        }
	    } else if (rs == SR_TIMEDOUT) {
#if	CF_DEBUGN
	        nprintf(NDF,"uctimeout_cmdrecv: timeout\n") ;
#endif
	        if (! uip->f_reqexit) cmd = dispcmd_timeout ;
	        rs = SR_OK ;
	    } else {
	        cmd = dispcmd_exit ;
#if	CF_DEBUGN
	        nprintf(NDF,"uctimeout_cmdrecv: err rs=%d\n",rs) ;
#endif
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-section) */

#if	CF_DEBUGN
	nprintf(NDF,"uctimeout_cmdrecv: ret rs=%d cmd=%u\n",rs,cmd) ;
#endif

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (uctimeout_cmdrecv) */


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
	uip->f_reqexit = FALSE ;
	uip->f.thrs = FALSE ;
	uip->f_thrsiger = FALSE ;
	uip->f_thrdisp = FALSE ;
	uip->f_exitsiger = FALSE ;
	uip->f_exitdisp = FALSE ;
	uip->pid = getpid() ;
	ptm_unlock(&uip->m) ;
#if	CF_CHILDTHRS /* optional */
	if (uip->f.workready) {
	    uctimeout_thrsbegin(uip) ;
	}
#endif /* CF_CHILDTHRS */
}
/* end subroutine (uctimeout_atforkchild) */


static int ourcmp(const void *a1p,const void *a2p)
{
	TIMEOUT		**e1pp = (TIMEOUT **) a1p ;
	TIMEOUT		**e2pp = (TIMEOUT **) a2p ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            TIMEOUT	*i1p = (TIMEOUT *) *e1pp ;
	            TIMEOUT	*i2p = (TIMEOUT *) *e2pp ;
	            rc = (i1p->val - i2p->val) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (ourcmp) */


