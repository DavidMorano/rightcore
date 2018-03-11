/* csem */

/* Counting-Semaphore (CSEM) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module provides an interlocked (atomic) counting semaphore. It
        uses the underlying POSIX mutex and condition-variables synchronization
        facility implementations.

        One would think that some OS (or even POSIX) would have given us a
        counting semaphore by now, but NO, that is not the case. Admittedly,
        there are no longer as many uses for a counting semaphore per se, now
        that the p-threads semaphore and p-threads condition variables are
        available. But we get by because the unusual decrement amount on the
        semaphore count is usually only one (1), and that happens to be the
        only, but generally sufficient, decrement amount that p-threads
        semaphores allow for.


*******************************************************************************/


#define	CSEM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"csem.h"


/* local defines */


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* forward references */

static int	csem_ptminit(CSEM *,int) ;
static int	csem_ptcinit(CSEM *,int) ;


/* local variables */


/* exported subroutines */


int csem_create(CSEM *psp,int f_shared,int count)
{
	int		rs ;

	if (psp == NULL) return SR_FAULT ;

	if (count < 1) count = 1 ;

	memset(psp,0,sizeof(CSEM)) ;
	psp->count = count ;

	if ((rs = csem_ptminit(psp,f_shared)) >= 0) {
	    if ((rs = csem_ptcinit(psp,f_shared)) >= 0) {
		psp->magic = CSEM_MAGIC ;
	    }
	    if (rs < 0)
		ptm_destroy(&psp->m) ;
	}

	return rs ;
}
/* end subroutine (csem_start) */


int csem_destroy(CSEM *psp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != CSEM_MAGIC) return SR_NOTOPEN ;

	rs1 = ptc_destroy(&psp->c) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&psp->m) ;
	if (rs >= 0) rs = rs1 ;

	psp->magic = 0 ;
	return rs ;
}
/* end subroutine (csem_destroy) */


int csem_decr(CSEM *psp,int c,int to)
{
	struct timespec	ts ;
	int		rs ;
	int		rs1 ;
	int		ocount = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != CSEM_MAGIC) return SR_NOTOPEN ;

	if (c < 0) return SR_INVALID ;

	if (to >= 0) {
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	}

	if ((rs = ptm_lockto(&psp->m,to)) >= 0) {
	    psp->waiters += 1 ;

	    while ((rs >= 0) && (psp->count < c)) {
		if (to >= 0) {
	            rs = ptc_timedwait(&psp->c,&psp->m,&ts) ;
		} else {
	            rs = ptc_wait(&psp->c,&psp->m) ;
		}
	    } /* end while */

	    if (rs >= 0) {
		ocount = psp->count ;
		psp->count -= c ;
	    }

	    psp->waiters -= 1 ;
	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? ocount : rs ;
}
/* end subroutine (csem_decr) */


int csem_incr(CSEM *psp,int c)
{
	int		rs ;
	int		rs1 ;
	int		ocount = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != CSEM_MAGIC) return SR_NOTOPEN ;

	if (c < 0) return SR_INVALID ;

	if ((rs = ptm_lock(&psp->m)) >= 0) {

	    ocount = psp->count ;
	    if (c > 0) {
	        psp->count += c ;
		if (psp->waiters > 0) {
	            rs = ptc_signal(&psp->c) ;
	        }
	    }

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-lock) */

	return (rs >= 0) ? ocount : rs ;
}
/* end subroutine (csem_incr) */


int csem_count(CSEM *psp)
{
	int		rs ;
	int		rs1 ;
	int		ocount = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != CSEM_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&psp->m)) >= 0) {

	    ocount = psp->count ;

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-lock) */

	return (rs >= 0) ? ocount : rs ;
}
/* end subroutine (csem_count) */


int csem_waiters(CSEM *psp)
{
	int		rs ;
	int		rs1 ;
	int		waiters = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != CSEM_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&psp->m)) >= 0) {

	    waiters = psp->waiters ;

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mutex-lock) */

	return (rs >= 0) ? waiters : rs ;
}
/* end subroutine (csem_waiters) */


/* private subroutines */


static int csem_ptminit(CSEM *psp,int f_shared)
{	
	PTMA		a ;
	int		rs ;
	int		rs1 ;
	int		f_ptm = FALSE ;

	if ((rs = ptma_create(&a)) >= 0) {

	    if (f_shared) {
		const int	v = PTHREAD_PROCESS_SHARED ;
		rs = ptma_setpshared(&a,v) ;
	    }

	    if (rs >= 0) {
	        rs = ptm_create(&psp->m,&a) ;
		f_ptm = (rs >= 0) ;
	    }

	    rs1 = ptma_destroy(&a) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_ptm) ptm_destroy(&psp->m) ;
	} /* end if (ptma) */

	return rs ;
}
/* end subroutine (csem_ptminit) */


static int csem_ptcinit(CSEM *psp,int f_shared)
{	
	PTCA		a ;
	int		rs ;
	int		rs1 ;
	int		f_ptc = FALSE ;

	if ((rs = ptca_create(&a)) >= 0) {

	    if (f_shared) {
		const int	v = PTHREAD_PROCESS_SHARED ;
		rs = ptca_setpshared(&a,v) ;
	    }

	    if (rs >= 0) {
	        rs = ptc_create(&psp->c,&a) ;
		f_ptc = (rs >= 0) ;
	    }

	    rs1 = ptca_destroy(&a) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_ptc) ptc_destroy(&psp->c) ;
	} /* end if (ptca) */

	return rs ;
}
/* end subroutine (csem_ptcinit) */


