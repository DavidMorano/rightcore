/* vecobj_hsort */

/* vector object list operations (heapsort) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine extends the VECOBJ object to add the capability to get
	the vector-list heap-sorted rather than quick-sorted as would be done
	normally.  There are rare occassions when a heap sort (or some sort
	other than Quick-Sort) is desirable.  Those occassions are generally
	only when a Quick-Sort is known to perform poorly (like when all of the
	data is already sorted).


*******************************************************************************/


#define	VECOBJ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecobj.h"


/* local defines */


/* forward references */


/* local variables */


/* exported subroutines */


/* sort the entries in the list */
int vecobj_hsort(vecobj *op,int (*cmpfunc)()) 
{

	if (op == NULL) return SR_FAULT ;
	if (cmpfunc == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.issorted) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        heapsort(op->va,op->i,cmpfunc) ;
	    }
	}

	return op->c ;
}
/* end subroutine (vecobj_hsort) */


