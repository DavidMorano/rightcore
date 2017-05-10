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
	    f = f || ((sl || (sl > 1)) && (sp[0] == '/') && (sp[1] == ec)) ;
	}

	return f ;
}
/* end subroutine (hasvarpathprefix) */


