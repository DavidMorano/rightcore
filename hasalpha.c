/* hasalpha */

/* does the given string an at least one ALPHA character? */


/* revision history:

	= 1998-10-10, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if a specified string has any:
		ALPHA
	characters.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isalphalatin(int) ;


/* exported subroutines */


int hasalpha(cchar *sp,int sl)
{
	int		f = FALSE ;

	while (sl && *sp) {
	    const int	ch = MKCHAR(*sp) ;
	    f = isalphalatin(ch) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasalpha) */


