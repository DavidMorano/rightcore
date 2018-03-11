/* ipow */

/* return integer-power */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DYNAMIC	1		/* dynamic programming */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine calculates and returns an integer power for a given
        base.

	Synopsis:

	int ipow(b,p)
	int	b, p ;

	Arguments:

	b	base
	p	power

	Returns:

	-	result (b**p)

	Notes: 

	Only defined for positive exponents.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>		/* 'LONG' type */


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	CF_DYNAMIC

int ipow(int b,int n)
{
	int		r = 1 ;
	if (n == 1) {
	    r = b ;
	} else if (n == 2) { /* common case */
	    r = b*b ;
	} else if (n > 2) {
	    int	t = ipow(b,(n/2)) ;
	    if ((n&1) == 0) {
		r = (t*t) ;
	    } else {
		r = b*(t*t) ;
	    }
	}
	return r ;
}
/* end subroutine (ipow) */

#else /* CF_DYNAMIC */

int ipow(int b,int n)
{
	int		r = 1 ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    r *= b ;
	}
	return r ;
}
/* end subroutine (ipow) */

#endif /* CF_DYNAMIC */


