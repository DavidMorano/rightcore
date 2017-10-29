/* haslc */

/* has upper-case characters? */


/* revision history:

	= 1998-10-10, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if a specified string has any:
		lowercase 
	characters.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int haslc(cchar *sp,int sl)
{
	int		f = FALSE ;

	while (sl && *sp) {
	    f = CHAR_ISLC(*sp) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (haslc) */


