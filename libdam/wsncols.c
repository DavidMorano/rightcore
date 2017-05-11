/* wsncols */

/* calculate number the of columns used by a wide-string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine calculates the number of columns that a wide-string
	uses given a horizontal position in a line of columns.

	Synopsis:

	int wsncols(int ntab,int ccol,const wchar_t *wsp,int wsl)

	Arguments:

	ntab		maximum number of columns in a TAB character
	ccol		current column number
	wsp		wide-string pointer
	wsl		wide-string length (in characters)

	Returns:

	-		number of columns used up


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stddef.h>		/* for 'wchar_t' */
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


int wsncols(int ntab,int ccol,const wchar_t *wsp,int wsl)
{
	int		cols = 0 ;
	while (wsl-- && *wsp) {
	    if (*wsp == CH_TAB) {
	        cols += tabcols(ntab,ccol) ;
	    } else {
		cols += 1 ;
	    }
	} /* end while */
	return cols ;
}
/* end subroutine (wsncols) */


int wscols(int ntab,int ccol,const wchar_t *wsp)
{
	return wsncols(ntab,ccol,wsp,-1) ;
}
/* end subroutine (wscols) */


