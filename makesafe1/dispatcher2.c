/* dispatcher */

/* object to dispatch jobs to threads */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	The object is a generalized version of what used to be pseudo-random
	code in other programs (that did multi-thread dispatching).

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object is used as a helper to manage jobs that need to be
        dispatched to parrallel theads.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dispatcher.h"
#include	"upt.h"


/* local defines */


/* typedefs */

typedef int (*workthr)(void *) ;
typedef int (*callthr)(void *,void *) ;


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnprocessors(const char **,int) ;
extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int dispatcher_starter(DISPATCHER *) ;
static int dispatcher_worker(DISPATCHER *) ;


/* local variables */


/* exported subroutines */


int dispatcher_start(DISPATCHER *dop,int n,void *callsub,void *callarg)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("dispatcher_start: ent\n") ;
#endif

	if (dop == NULL) return SR_FAULT ;
	if (callsub == NULL) return SR_FAULT ;

	memset(dop,0,sizeof(DISPATCHER)) ;
	dop->callsub = callsub ;
	dop->callarg = callarg ;

/* set our local concurrency */

	if (n < 0) {
	    if ((rs = uc_nprocessors(0)) >= 0) {
	        n = rs ;
	    } else if (rs == SR_NOSYS) {
	        n = 1 ;
	    }
	} else if (n == 0) {
	    if ((n = uptgetconcurrency()) < 1) {
	        n = 1 ;
	    }
	}

	if (rs >= 0) {
	dop->nthr = n ;
	if ((rs = fsi_start(&dop->wq)) >= 0) {
	    if ((rs = psem_create(&dop->wq_sem,FALSE,0)) >= 0) {
	        if ((rs = ptm_create(&dop->m,NULL)) >= 0) {
	            if ((rs = ptc_create(&dop->cond,NULL)) >= 0) {
	                    const int	size = (dop->nthr * sizeof(DISP_THR)) ;
	                void		*p ;
	                if ((rs = uc_malloc(size,&p)) >= 0) {
	                    dop->threads = p ;
			    memset(p,0,size) ;
	                    rs = disp_starter(dop) ;
	                    if (rs < 0) {
	                        uc_free(dop->threads) ;
	                        dop->threads = NULL ;
	                    }
	                } /* end if (m-a) */
	                if (rs < 0)
	                    ptc_destroy(&dop->cond) ;
	            } /* end if (ptc_create) */
	            if (rs < 0)
	                ptm_destroy(&dop->m) ;
	        } /* end if (ptm_create) */
	        if (rs < 0)
	            psem_destroy(&dop->wq_sem) ;
	    } /* end if (psem_create) */
	    if (rs < 0)
	        fsi_finish(&dop->wq) ;
	} /* end if (fsi) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (dispatcher_start) */


static int dispatcher_finish(DISP *dop,int f_abort)
{
	PROGINFO	*pip = dop->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ; /* lint */

#if	CF_DEBUGS
	    debugprintf("dispatcher_finish: ent f_abort=%u\n",f_abort) ;
#endif

	dop->f_done = TRUE ;		/* exit when no more work */
	if (f_abort) dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->nthr ; i += 1) {
	    rs1 = psem_post(&dop->wq_sem) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	    debugprintf("dispatcher_finish: mid1 rs=%d\n",rs) ;
#endif

	if (dop->threads != NULL) {
	    DISP_THR	*dtp ;
	    pthread_t	tid ;
	    int		trs ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        if (dtp->f_active) {
	            dtp->f_active = FALSE ;
	            tid = dtp->tid ;
	            rs1 = uptjoin(tid,&trs) ;
#if	CF_DEBUGS
	    	debugprintf("dispatcher_finish: "
			"i=%u uptjoin() tid=%u rs=%d trs=%d\n",i,tid,rs,trs) ;
#endif
	            if (rs >= 0) rs = rs1 ;
	            if (rs >= 0) rs = trs ;
	            if (rs > 0) c += trs ;
	        } /* end if (active) */
	    } /* end for */
	    rs1 = uc_free(dop->threads) ;
	    if (rs >= 0) rs = rs1 ;
	    dop->threads = NULL ;
	} /* end if (threads) */

#if	CF_DEBUGS
	    debugprintf("dispatcher_finish: mid2 rs=%d\n",rs) ;
#endif

	rs1 = ptc_destroy(&dop->cond) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&dop->m) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->wq_sem) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fsi_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	    debugprintf("dispatcher_finish: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dispatcher_finish) */


int dispatcher_add(DISPATCHER *dop,void *wop)
{
	int		rs ;

	if (dop == NULL) return SR_FAULT ;
	if (wop == NULL) return SR_FAULT ;

	if ((rs = ciq_ins(&dop->wq,wop)) >= 0) {
	    rs = psem_post(&dop->ws) ;
	}

	return rs ;
}
/* end subroutine (dispatcher_add) */


/* |main| calls this to intermittently wait for worker completion */
int disp_waiting(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    while ((rs = disp_notready(dop,&c)) > 0) {
	        rs = ptc_wait(&dop->cond,mp) ;
	        if (rs < 0) break ;
	    } /* end while */
	    dop->f_wakeup = FALSE ;
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
#if	CF_DEBUGS
	debugprintf("disp_waiting: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_waiting) */


/* private subroutines */


static int dispatcher_starter(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		i ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	    debugprintf("main/dispatcher_starter: ent nthr=%u\n",dop->nthr) ;
#endif

	for (i = 0 ; (rs >= 0) && (i < dop->nthr) ; i += 1) {
	    uptsub_t	fn = (uptsub_t) dispatcher_worker ;
	    if ((rs = uptcreate(&tid,NULL,fn,dop)) >= 0) {
	        dop->threads[i].tid = tid ;
	        dop->threads[i].f_active = TRUE ;
	    }
#if	CF_DEBUGS
	    debugprintf("dispatcher_starter: i=%u uptcreate() rs=%d tid=%u\n",
		i,rs,tid) ;
#endif
	} /* end for */
	    if (rs >= 0) {
		rs = dispatcher_readyset(dop) ;
	    }

	if (rs < 0) {
	    int		n = i ;
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < n ; i += 1) {
	        psem_post(&dop->wq_sem) ;
	    }
	    for (i = 0 ; i < n ; i += 1) {
	        tid = dop->threads[i].tid ;
	        uptjoin(tid,NULL) ;
	        dop->threads[i].f_active = FALSE ;
	    }
	} /* end if (failure) */

#if	CF_DEBUGS
	    debugprintf("dispatcher_starter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dispatcher_starter) */


/* of course this runs in parallel threads */
static int dispatcher_worker(DISPATCHER *dop)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	{
	    pthread_t	tid = pthead_self() ;
	    debugprintf("dispatcher_worker: ent tid=%u\n",tid) ;
	}
#endif

	while ((rs >= 0) && (! dop->f_exit)) {

	    if ((rs = psem_wait(&dop->ws)) >= 0) {
	        void	*wop ;
	        if ((rs = ciq_rem(&dop->wq,&wop)) >= 0) {
	            callthr	sub = (callthr) dop->callsub ;
	            rs = (*sub)(dop->callarg,wop) ;
	            if (rs > 0) c += 1 ;
	        } else if (rs == SR_NOTFOUND) {
	            rs = SR_OK ;
	            if (dop->f_done) break ;
	        }
	    } /* end if (psem_wait) */

	} /* end while (server loop) */

#if	CF_DEBUGS
	{
	    pthread_t	tid = pthead_self() ;
	    debugprintf("dispatcher_worker: tid=%u ret rs=%d c=%u\n",
	        tid,rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dispatcher_worker) */


/* worker thread calls this to register a task completion */
static int disp_taskdone(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    dop->tasks += 1 ;
	    if (! dop->f_wakeup) {
	        dop->f_wakeup = TRUE ;
	        rs = ptc_signal(&dop->cond) ;
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_taskdone) */


static int disp_exiting(DISP *dop)
{
	DISP_THR	*dtp ;
	int		rs ;
	int		i = 0 ;
	if ((rs = disp_getourthr(dop,&dtp)) >= 0) {
	    i = rs ;
	    dtp->f_exiting = TRUE ;
	    rs = ptc_signal(&dop->cond) ;
	} /* end if (disp_getourthr) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (disp_exiting) */


static int disp_allexiting(DISP *dop)
{
	DISP_THR	*threads = dop->threads ;
	int		rs = SR_OK ;
	int		i ;
	int		f = TRUE ;
	for (i = 0 ; i < dop->nthr ; i += 1) {
	    f = f && threads[i].f_exiting ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (disp_allexiting) */


/* helper function for |disp_waiting()| above */
static int disp_notready(DISP *dop,int *cp)
{
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("disp_notready: ent f_wakeup=%u\n",dop->f_wakeup) ;
#endif
	*cp = 0 ;
	if ((rs = fsi_count(&dop->wq)) > 0) {
	    *cp = rs ;
#if	CF_DEBUGS
	    debugprintf("disp_notready: c=%u\n",rs) ;
#endif
	    if (! dop->f_wakeup) {
	        if ((rs = disp_allexiting(dop)) == 0) {
#if	CF_DEBUGS
	            debugprintf("disp_notready: not-allexiting\n") ;
#endif
		    f = TRUE ;
		}
	    }
	}
#if	CF_DEBUGS
	debugprintf("disp_notready: ret rs=%d f=%u c=%u\n",
		rs,f,*cp) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (disp_notready) */


static int disp_getourthr(DISP *dop,DISP_THR **rpp)
{
	int		rs ;
	int		i = 0 ;
	if ((rs = disp_readywait(dop)) >= 0) {
	    DISP_THR	*dtp ;
	    pthread_t	tid = pthread_self() ;
	    int		f = FALSE ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        f = uptequal(dtp->tid,tid) ;
	        if (f) break ;
	    } /* end for */
	    if (f) {
	        if (rpp != NULL) *rpp = dtp ;
	    } else {
	        rs = SR_BUGCHECK ;
	    }
	} /* end if (disp_readywait) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (disp_getourthr) */


/* main-thread calls this to indicate sub-threads can read completed object */
static int disp_readyset(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    {
	        dop->f_ready = TRUE ;
	        rs = ptc_broadcast(&dop->cond) ; /* 0-bit semaphore */
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readyset) */


/* sub-threads call this to wait until object is ready */
static int disp_readywait(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    while ((! dop->f_ready) && (! dop->f_exit)) {
	        rs = ptc_wait(&dop->cond,mp) ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readywait) */

