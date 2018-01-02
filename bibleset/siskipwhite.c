/* siskipwhite */

/* subroutine to find a character in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the number of characters skipped in the string
	due to being whitespace.

	Synopsis:

	int siskipwhite(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on

	Returns:

	>0	number of characters skipped
	==0	no characters were skipped (no whitespace)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int siskipwhite(cchar *sp,int sl)
{
	int		i ;

	for (i = 0 ; sl-- && sp[i] ; i += 1) {
	    if (! CHAR_ISWHITE(sp[i])) break ;
	} /* end while */

	return i ;
}
/* end subroutine (siskipwhite) */


