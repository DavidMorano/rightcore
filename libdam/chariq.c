/* chariq */

/* Character-Interlocked Queue management */


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
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<string.h>

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


int chariq_start(op,size)
CHARIQ		*op ;
int		size ;
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


int chariq_finish(op)
CHARIQ		*op ;
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


int chariq_ins(op,ch)
CHARIQ		*op ;
int		ch ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = charq_ins(&op->q,ch) ;

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (chariq_ins) */


int chariq_rem(op,chp)
CHARIQ		*op ;
char		*chp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = charq_rem(&op->q,chp) ;

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (chariq_rem) */


int chariq_size(op)
CHARIQ		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = charq_size(&op->q) ;

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (chariq_size) */


int chariq_count(op)
CHARIQ		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = ptm_lock(&op->m)) >= 0) {

	    rs = charq_count(&op->q) ;

	    ptm_unlock(&op->m) ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (chariq_count) */


