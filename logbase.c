/* logbase */

/* calculate the log of a number given the base also */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2003-03-04, David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find the log-base-(b) of a number (v).

	Synopsis:

	double logbase(double b,double v)

	Arguments:

	b		base
	v		number to take the log-base-(b) of

	Returns:

	-		result


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<math.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


double logbase(double b,double v)
{
	const double	n = log(v) ;
	const double	d = log(b) ;
	return (n/d) ;
}
/* end subroutine (logbase) */


