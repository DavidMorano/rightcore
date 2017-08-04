/* hasalluc */

/* is this whole string upper-case? */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Determine if the entire string passed consists of all upper-case
	characters.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int hasalluc(cchar *sp,int sl)
{
	int		f = TRUE ;

	while (sl && *sp) {
	    f = CHAR_ISUC(*sp) ;
	    if (! f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasalluc) */


