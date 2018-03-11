/* nlinecols */

/* calculate number of columns used by a line of characters */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine calculates the number of columns that a line of
	characters takes up on a terminal, with tab stops set.

	Synopsis:

	int nlinecols(ntab,ccol,lbuf,llen)
	int		ntab ;
	int		ccol ;
	const char	lbuf[] ;
	int		llen ;

	Arguments:

	ntab		number of columns in a TAB character
	ccol		current column number
	lbuf		source string 
	llen		length of source string

	Returns:
	-		column number after line if used up


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

extern int	charcols(int,int,int) ;
extern int	tabcols(int,int) ;
extern int	iceil(int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int nlinecols(int ntab,int ccol,cchar *lbuf,int llen)
{
	int		i ;
	int		cols ;

	if (llen < 0) llen = strlen(lbuf) ;

	for (i = 0 ; i < llen ; i += 1) {
	    ccol += charcols(ntab,ccol,lbuf[i]) ;
	} /* end for */

	return ccol ;
}
/* end subroutine (nlinecols) */


