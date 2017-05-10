/* haswhite */

/* has white-space characters? */


/* revision history:

	= 1998-10-10, David A­D­ Morano
        This subroutine was originally written but modeled from assembly
        language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if a specified string has any:
		whitespace
	characters.

	Extra-note: Note that non-breaking-white-space (NBSP) characters are
	*not* considered to be white-space!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int haswhite(cchar *sp,int sl)
{
	int		f = FALSE ;

	while (sl && *sp) {
	    f = CHAR_ISWHITE(*sp) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (haswhite) */


