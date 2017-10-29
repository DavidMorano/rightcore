/* ncolchar */

/* calculate number of columns used by a character */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine calculates the number of columns that a character takes
	up on a terminal with tab stops set.

	Synopsis:

	int charcols(ntab,ccol,ch)
	int	ntab ;
	int	ccol ;
	int	ch ;

	Arguments:

	ntab		maximum number of columns in a TAB character
	ccol		current column number
	ch		the character to calculate columns for

	Returns:

	-		number of columns used up


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	tabcols(int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ncolchar(int ntab,int ccol,int ch)
{
	int		cols = 0 ;
	switch (ch) {
	case CH_SO:
	case CH_SI:
	case CH_SS2:
	case CH_SS3:
	    cols = 0 ;
	    break ;
	case '\t':
	    cols = tabcols(ntab,ccol) ;
	    break ;
	default:
	    if (((ch & 0x7f) >= 0x20) || (ch == 0xff)) {
	        cols = 1 ;
	    }
	    break ;
	} /* end switch */
	return cols ;
}
/* end subroutine (ncolchar) */


int charcols(int ntab,int ccol,int ch)
{
	return ncolchar(ntab,ccol,ch) ;
}
/* end subroutine (charcols) */


