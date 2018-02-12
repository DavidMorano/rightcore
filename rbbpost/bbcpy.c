/* bbcpy */

/* copy a string substituting periods for where ever we find slashes */


/* revision history:

	= 1994-05-01, David A­D­ Morano
        This code was written from scratch to support the correct operation of
        multiple directories for each newsgroup.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine copies a string that is supposed to be a bulletin board
        newsgroup directory but it substitutes period characters for where ever
        there was a slash character in the newsgroup directory path.

	Synopsis:

	int bbcpy(s2,s1)
	char	*s1, *s2 ;

	Arguments:

	s1	string that contains the newsgroup directory path
	s2	resulting strings with periods instead of slashes

	Returns:

	>=0	length of string copied


*******************************************************************************/


#include	<envstandards.h>

#include	<localmisc.h>


/* exported subroutines */


int bbcpy(s2,s1)
char		*s2 ;
const char	*s1 ;
{
	const char	*cp = s1 ;

	while (*s1) {
	    if (*s1 == '/') {
	        *s2++ = '.' ;
	        s1 += 1 ;
	    } else
	        *s2++ = *s1++ ;
	} /* end while */

	*s2++ = '\0' ;
	return (s2 - cp) ;
}
/* end subroutine (bbcpy) */



