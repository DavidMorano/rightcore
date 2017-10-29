/* hasvarpathprefix */

/* test for var-path prefix? */


/* revision history:

	= 1998-10-10, David A­D­ Morano
	This subroutine was originally written but modeled from assembly
	language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine tests whether the file-name has the var-path prefix on
	it.

	A so-called "var-path" prefix looks like either one of the following:

	%<string>/<comething>/<...>
	/%<string>/<something>/...


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int hasvarpathprefix(cchar *sp,int sl)
{
	const int	ec = '%' ;
	int		f = FALSE ;

	if (sp != NULL) {
	    f = f || (sl && (sp[0] == ec)) ;
	    if (!f) {
		f = TRUE ;
	        f = f && ((sl < 0) || (sl > 1)) ;
		f = f && (sp[0] == '/') && (sp[1] == ec) ;
	    }
	}

	return f ;
}
/* end subroutine (hasvarpathprefix) */


