/* dispatcher */

/* object to dispatch jobs to threads */
/* last modified %G% version %I% */


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
	dop->nthr = n ;
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
	    if ((rs = ciq_start(&dop->wq)) >= 0) {
	        if ((rs = psem_create(&dop->ws,FALSE,0)) >= 0) {
	            const int	size = sizeof(pthread_t) ;
	            const int	vo = (VECOBJ_OREUSE) ;
	            if ((rs = vecobj_start(&dop->tids,size,10,vo)) >= 0) {
	                pthread_t	tid, *tidp ;
	                workthr		w = (workthr) dispatcher_worker ;
	                int		i ;
/* create threads to handle it */
	                for (i = 0 ; (rs >= 0) && (i < dop->nthr) ; i += 1) {
	                    if ((rs = uptcreate(&tid,NULL,w,dop)) >= 0) {
	                        rs = vecobj_add(&dop->tids,&tid) ;
	                    }
	                } /* end for */
/* all setup: but handle any errors */
	                if (rs < 0) {
	                    dop->f_exit = TRUE ;
	                    for (i = 0 ; i < dop->nthr ; i += 1) {
	                        psem_post(&dop->ws) ;
	                    }
	                    i = 0 ;
	                    while (vecobj_get(&dop->tids,i,&tidp) >= 0) {
	                        if (tidp != NULL) {
	                            uptjoin(*tidp,NULL) ;
	                        }
	                        i += 1 ;
	                    } /* end while */
	                } /* end if (failure) */
	                if (rs < 0)
	                    vecobj_finish(&dop->tids) ;
	            } /* end if (vecobj) */
	            if (rs < 0)
	                psem_destroy(&dop->ws) ;
	        } /* end if (ptm) */
	        if (rs < 0)
	            ciq_finish(&dop->wq) ;
	    } /* end if (ciq) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (dispatcher_start) */


int dispatcher_finish(DISPATCHER *dop,int f_abort)
{
	pthread_t	*tidp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		trs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("dispatcher_finish: ent\n") ;
#endif

	if (dop == NULL) return SR_FAULT ;

	dop->f_done = TRUE ; /* broadcast that we are "done" */
	if (f_abort) dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->nthr ; i += 1) {
	    psem_post(&dop->ws) ;
	}

#if	CF_DEBUGS
	debugprintf("dispatcher_finish: mid3 rs=%d\n",rs) ;
#endif

	for (i = 0 ; vecobj_get(&dop->tids,i,&tidp) >= 0 ; i += 1) {
	    if (tidp != NULL) {
	        rs1 = uptjoin(*tidp,&trs) ;
	        if (rs >= 0) rs = rs1 ;
	        if (rs >= 0) rs = trs ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("dispatcher_finish: mid5 rs=%d\n",rs) ;
#endif

	rs1 = vecobj_finish(&dop->tids) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->ws) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ciq_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("dispatcher_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
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


/* local subroutines */


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


