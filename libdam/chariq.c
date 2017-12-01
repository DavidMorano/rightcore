/* chariq */

/* Character-Interlocked Queue management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages interlocked FIFO-character operations.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"chariq.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int chariq_start(CHARIQ *op,int size)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (size <= 0) size = 10 ;

	if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	    rs = charq_start(&op->q,size) ;
	    if (rs < 0)
		ptm_destroy(&op->m) ;
	}

	return rs ;
}
/* end subroutine (chariq_start) */


int chariq_finish(CHARIQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	rs1 = charq_finish(&op->q) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (chariq_finish) */


int chariq_ins(CHARIQ *op,int ch)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = charq_ins(&op->q,ch) ;
		c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (chariq_ins) */


int chariq_rem(CHARIQ *op,char *chp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = charq_rem(&op->q,chp) ;
		c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (chariq_rem) */


int chariq_size(CHARIQ *op)
{
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = charq_size(&op->q) ;
		rv = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (chariq_size) */


int chariq_count(CHARIQ *op)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = charq_count(&op->q) ;
		c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (chariq_count) */


