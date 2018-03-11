/* intiq */

/* Integer-Interlocked Queue management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-11-24, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages interlocked FIFO-integer operations.

	Notes:
	+ thread-safe


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"intiq.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int intiq_start(INTIQ *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	    rs = fifoitem_start(&op->q) ;
	    if (rs < 0)
		ptm_destroy(&op->m) ;
	}

	return rs ;
}
/* end subroutine (intiq_start) */


int intiq_finish(INTIQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	rs1 = fifoitem_finish(&op->q) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (intiq_finish) */


int intiq_ins(INTIQ *op,int ch)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        const int	esize = sizeof(int) ;
	        rs = fifoitem_ins(&op->q,&ch,esize) ;
		c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (intiq_ins) */


int intiq_rem(INTIQ *op,int *chp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
		const int	esize = sizeof(int) ;
	        rs = fifoitem_rem(&op->q,chp,esize) ;
	        c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (intiq_rem) */


int intiq_count(INTIQ *op)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = fifoitem_count(&op->q) ;
		c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (intiq_count) */


