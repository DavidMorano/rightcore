/* hasallwhite */

/* is this whole string white? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
        This module was originally written but modeled from original assembly
        language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Determine if the entire string passed conssists of all white-space
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


int hasallwhite(cchar *sp,int sl)
{
	int		f = TRUE ;

	while (sl && *sp) {
	    f = CHAR_ISWHITE(*sp) ;
	    if (! f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasallwhite) */


