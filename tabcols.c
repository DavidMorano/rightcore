/* tabcols */

/* calculate tab columns */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This small subroutine calculates the number of columns that would be
	skipped by advancing to the next tab-stop.

	Synopsis:

	int tabcols(tablen,ncol)
	int	tablen ;
	int	ncol ;

	Arguments:

	tablen		number of columns in a tab
	ncol		current column number (counting from zero)

	Returns:

	-		number of columns that would be moved forward


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	iceil(int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int tabcols(int tablen,int ncol)
{
	const int	n = (ncol % tablen) ;
	int		stepcols ;

	stepcols = iceil((n + 1),tablen) - n ;

	return stepcols ;
}
/* end subroutine (tabcols) */


