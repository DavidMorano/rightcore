/* mkrevstr */

/* reverse the characters in a string in place */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Reverse the characters of a string in place.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* module global variables */


/* local variables */


/* exported subroutines */


int mkrevstr(char *bp,int bl)
{
	int		i ;
	if (bp == NULL) return SR_FAULT ;
	if (bl < 0) bl = strlen(bp) ;
	for (i = 0 ; i < (bl/2) ; i += 1) {
	    int	ch = bp[i] ;
	    bp[i] = bp[bl-i-1] ;
	    bp[bl-i-1] = ch ;
	} /* end for */
	return bl ;
}
/* end subroutine (mkrevstr) */


