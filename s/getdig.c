/* hexdig */

/* create a HEX digit from a value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-14, David A­D­ Morano
	This little object module was first written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We return a digit (anything from base-2 up to base-36) given a value.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */

static cchar	digtab[] = "0123456789ABCDEFghijklmnopqrstuvwxyz" ;


/* exported subroutines */


int getdig(int v)
{
	const int	n = nelem(digtab) ;
	int		dig = -1 ;
	if ((v >= 0) && (v < n)) {
	    dig = digtab[v] ;
	}
	return dig ;
}
/* end subroutine (getdig) */


