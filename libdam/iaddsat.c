/* iaddsat */

/* Integer Add-Saturated */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine provides a saturated add operation.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* exported subroutines */


int iaddsat(int v1,int v2)
{
	int	vr = (v1+v2) ;
	if (v1 >= 0) {
	    if (v2 >= 0) {
		if (vr < 0) vr = INT_MAX ;
	    }
	} else {
	    if (v2 < 0) {
		if (vr >= 0) vr = INT_MIN ;
	    }
	}
	return vr ;
}
/* end subroutine (iaddsat) */


