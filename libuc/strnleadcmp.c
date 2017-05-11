/* strnleadcmp */

/* check if string 's2' is a leading substring of string 's1' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns TRUE if 's2' is an initial substring of 's1'.
	But only up to the maximum number of characters are checked.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int strnleadcmp(cchar *s1,cchar *s2,int lr)
{
	int		f = TRUE ;

	if (lr < 0)
	    lr = strlen(s2) ;

	while (lr-- > 0) {
	    f = (*s2++ == *s1++) ;
	    if (! f) break ;
	} /* end while */

	return f ;
}
/* end subroutine (strnleadcmp) */


