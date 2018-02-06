/* getcols */

/* calculate number of columns used by a line of characters */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine returns the number of bytes needed to realize the number
        of columns specified.

	Synopsis:

	int getcols(ntab,ccol,ncols,lbuf,llen)
	int		ntab ;
	int		ccol ;
	int		ncols ;
	const char	lbuf[] ;
	int		llen ;

	Arguments:

	ntab		number of columns in a TAB character
	ccol		current column number
	ncols		number of additional columns wanted
	lbuf		line-buffer for given line of characters
	llen		length of line-buffer

	Returns:
	-		number of bytes used for the given number of columns


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


int getcols(ntab,ccol,ncols,lbuf,llen)
int		ntab ;
int		ccol ;
int		ncols ;
const char	lbuf[] ;
int		llen ;
{
	const int	tcol = (ccol + ncols) ;

	int	i = 0 ;

	if (llen < 0)
	    llen = strlen(lbuf) ;

	if (ccol < tcol) {
	    int	cols ;
	    for (i = 0 ; (ccol < tcol) && (i < llen) ; i += 1) {
	        cols = charcols(ntab,ccol,lbuf[i]) ;
	        ccol += cols ;
	    } /* end for */
	} /* end if */

	return i ;
}
/* end subroutine (getcols) */



