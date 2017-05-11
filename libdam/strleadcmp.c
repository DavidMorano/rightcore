/* strleadcmp */

/* check if string 's2' is a leading substring of string 's1' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns TRUE if str2 (second argument) is an initial
	substring of str1 (first argument).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* exported subroutines */


int strleadcmp(cchar *s1,cchar *s2)
{
	int		f = TRUE ;

	while (*s2) {
	    f = (*s2++ == *s1++) ;
	    if (! f) break ;
	}

	return f ;
}
/* end subroutine (strleadcmp) */


