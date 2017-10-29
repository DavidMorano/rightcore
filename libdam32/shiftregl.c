/* shiftregl */

/* Shift Register of Longs */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revistion history:

	= 2001-07-10, David A­D­ Morano
        This code was originally started (for Levo machine deadlock detection).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object just implements a vanilla shift register (not clocked). It
        should be used in data analysis applications and not as a Levo machine
        hardware component. There is no levo specific aspects to this object
        (like there has to be for regular Levo machine hardware components).


*******************************************************************************/


#define	SHIFTREGL_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>

#include	"shiftregl.h"


/* local defines */


/* external subroutines */


/* exported subroutines */


int shiftregl_start(srp,n)
SHIFTREGL	*srp ;
int		n ;
{
	int		rs ;
	int		size ;

	if (srp == NULL) return SR_FAULT ;

	if (n < 1) n = 1 ;

	memset(srp,0,sizeof(SHIFTREGL)) ;

	size = n * sizeof(ULONG) ;
	if ((rs = uc_malloc(size,&srp->regs)) >= 0) {
	    int	i ;
	    srp->n = n ;
	    for (i = 0 ; i < srp->n ; i += 1) {
	        srp->regs[i] = i ;
	    }
	    srp->magic = SHIFTREGL_MAGIC ;
	}

	return rs ;
}


int shiftregl_finish(srp)
SHIFTREGL	*srp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (srp == NULL) return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC) return SR_NOTOPEN ;

	if (srp->regs != NULL) {
	    rs1 = uc_free(srp->regs) ;
	    if (rs >= 0) rs = rs1 ;
	    srp->regs = NULL ;
	}

	srp->magic = 0 ;
	return rs ;
}


int shiftregl_shiftin(srp,val)
SHIFTREGL	*srp ;
ULONG		val ;
{
	int		i ;

	if (srp == NULL) return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC) return SR_NOTOPEN ;

	srp->regs[srp->i] = val ;
	srp->i = (srp->i + 1) % srp->n ;
	return SR_OK ;
}


int shiftregl_get(srp,i,vp)
SHIFTREGL	*srp ;
int		i ;
ULONG		*vp ;
{

	if (srp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (srp->magic != SHIFTREGL_MAGIC) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

	if (i >= srp->n) return SR_NOTFOUND ;

	*vp = srp->regs[i] ;
	return SR_OK ;
}
/* end subroutine (shiftregl_get) */


