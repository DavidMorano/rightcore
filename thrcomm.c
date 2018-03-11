/* thrcomm */

/* Thread-Communication (THRCOMM) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides some minimal communication between a controller
	thread and a separate parallel thread.


*******************************************************************************/


#define	THRCOMM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"thrcomm.h"


/* local defines */

#define	TO_NOSPC	5
#define	TO_MFILE	5
#define	TO_INTR		5


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* forward references */

static int	thrcomm_ptminit(THRCOMM *,int) ;
static int	thrcomm_ptcinit(THRCOMM *,int) ;


/* local variables */


/* exported subroutines */


int thrcomm_start(THRCOMM *psp,int f_shared)
{
	int		rs ;

	if (psp != NULL) {
	    memset(psp,0,sizeof(THRCOMM)) ;
	    if ((rs = thrcomm_ptminit(psp,f_shared)) >= 0) {
	        if ((rs = thrcomm_ptcinit(psp,f_shared)) >= 0) {
		    psp->magic = THRCOMM_MAGIC ;
		}
	        if (rs < 0)
		    ptm_destroy(&psp->m) ;
	    } /* end if (PTM created) */
	} else {
	    rs = SR_FAULT ;
	}

	return rs ;
}
/* end subroutine (thrcomm_start) */


int thrcomm_finish(THRCOMM *psp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != THRCOMM_MAGIC) return SR_NOTOPEN ;

	rs1 = ptc_destroy(&psp->c) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&psp->m) ;
	if (rs >= 0) rs = rs1 ;

	psp->magic = 0 ;
	return rs ;
}
/* end subroutine (thrcomm_finish) */


int thrcomm_cmdsend(THRCOMM *psp,int cmd,int to)
{
	struct timespec	ts ;
	int		rs ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != THRCOMM_MAGIC) return SR_NOTOPEN ;

	if (to >= 0) {
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	}

	if ((rs = ptm_lockto(&psp->m,to)) >= 0) {
	    psp->f_cmd = TRUE ;

	    while ((rs >= 0) && (psp->cmd != 0) && (! psp->f_exiting)) {
		if (to >= 0) {
	            rs = ptc_timedwait(&psp->c,&psp->m,&ts) ;
		} else {
	            rs = ptc_wait(&psp->c,&psp->m) ;
		}
	    } /* end while */

	    if (rs >= 0) {
	        if (! psp->f_exiting) {
	            psp->cmd = cmd ;
	            psp->rrs = SR_INPROGRESS ;
                    rs = ptc_broadcast(&psp->c) ;
		} else
		    cmd = 0 ;
	    }

	    psp->f_cmd = FALSE ;
	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (thrcomm_cmdsend) */


int thrcomm_cmdrecv(THRCOMM *psp,int to)
{
	struct timespec	ts ;
	int		rs ;
	int		rs1 ;
	int		cmd = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != THRCOMM_MAGIC) return SR_NOTOPEN ;

	if (to >= 0) {
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	}

	if ((rs = ptm_lockto(&psp->m,to)) >= 0) {

	    while ((rs >= 0) && (psp->cmd == 0)) {
		if (to >= 0) {
	            rs = ptc_timedwait(&psp->c,&psp->m,&ts) ;
		} else {
	            rs = ptc_wait(&psp->c,&psp->m) ;
		}
	    } /* end while */

	    if (rs >= 0) {
		cmd = psp->cmd ;
		psp->cmd = 0 ;
	    }

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (thrcomm_cmdrecv) */


int thrcomm_rspsend(THRCOMM *psp,int rrs,int to)
{
	struct timespec	ts ;
	int		rs ;
	int		rs1 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != THRCOMM_MAGIC) return SR_NOTOPEN ;

	if (to >= 0) {
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	}

	if ((rs = ptm_lockto(&psp->m,to)) >= 0) {

	    while ((rs >= 0) && (psp->rrs != SR_INPROGRESS)) {
		if (to >= 0) {
	            rs = ptc_timedwait(&psp->c,&psp->m,&ts) ;
		} else {
	            rs = ptc_wait(&psp->c,&psp->m) ;
		}
	    } /* end while */

	    if (rs >= 0) {
		psp->rrs = rrs ;
                rs = ptc_broadcast(&psp->c) ;
	    }

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? rrs : rs ;
}
/* end subroutine (thrcomm_rspsend) */


int thrcomm_rsprecv(THRCOMM *psp,int to)
{
	struct timespec	ts ;
	int		rs ;
	int		rs1 ;
	int		rrs = 0 ;

	if (psp == NULL) return SR_FAULT ;

	if (psp->magic != THRCOMM_MAGIC) return SR_NOTOPEN ;

	if (to >= 0) {
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	}

	if ((rs = ptm_lockto(&psp->m,to)) >= 0) {

	    while ((rs >= 0) && (psp->rrs == SR_INPROGRESS)) {
		if (to >= 0) {
	            rs = ptc_timedwait(&psp->c,&psp->m,&ts) ;
		} else {
	            rs = ptc_wait(&psp->c,&psp->m) ;
		}
	    } /* end while */

	    if (rs >= 0) {
		rrs = psp->rrs ;
		psp->rrs = SR_INPROGRESS ;
	    }

	    rs1 = ptm_unlock(&psp->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? rrs : rs ;
}
/* end subroutine (thrcomm_rsprecv) */


int thrcomm_exiting(THRCOMM *psp)
{
	int		rs = SR_OK ;
	psp->f_exiting = TRUE ;
	psp->cmd = 0 ;
	if (psp->f_cmd) {
    	    rs = ptc_broadcast(&psp->c) ;
	}
	return rs ;
}
/* end subroutine (thrcomm_exiting) */


/* private subroutines */


static int thrcomm_ptminit(THRCOMM *psp,int f_shared)
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
/* end subroutine (thrcomm_ptminit) */


static int thrcomm_ptcinit(THRCOMM *psp,int f_shared)
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
/* end subroutine (thrcomm_ptcinit) */


