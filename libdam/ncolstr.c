/* ncolstr */

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
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	ncolchar(int,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ncolstr(int ntab,int ccol,const char *sp,int sl)
{
	int		cols = 0 ;
	int		ch ;
	while (sl-- && sp[0]) {
	    ch = MKCHAR(sp[0]) ;
	    cols += ncolchar(ntab,(ccol+cols),ch) ;
	    sp += 1 ;
	} /* end while */
	return cols ;
}
/* end subroutine (ncolstr) */


