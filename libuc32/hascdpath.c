/* hascdpath */

/* test for var-path prefix? */


/* revision history:

	= 1998-10-10, David A­D­ Morano
        This subroutine was originally written but modeled from assembly
        language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine tests whether the file-name has the var-path prefix on
        it.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int hascdpath(cchar *sp,int sl)
{
	const int	ec = ('¬' & UCHAR_MAX) ;
	int		f = FALSE ;

	if (sl && (sp != NULL)) {
	    int	ch = MKCHAR(sp[0]) ;
	    f = (ch == ec) ;
	}

	return f ;
}
/* end subroutine (hascdpath) */


