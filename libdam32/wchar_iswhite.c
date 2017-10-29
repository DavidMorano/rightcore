/* wchar_iswhite */

/* check for a white-space wide-character */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check for a white-space wide-character.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stddef.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int wchar_iswhite(wchar_t ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 256)) {
	    f = CHAR_ISWHITE(ch) ;
	}
	return f ;
}
/* end subroutine (wchar_iswhite) */


