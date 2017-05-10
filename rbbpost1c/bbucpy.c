/* bbucpy */

/* copy a string substituting slashes for where ever we find periods */


/* revision history:

	= 1994-05-01, David A­D­ Morano
        This code was written from scratch to support the correct operation of
        multiple directories for each newsgroup.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<localmisc.h>


/* exported subroutines */


int bbucpy(d,s)
char		*d ;
const char	*s ;
{
	char	*dp = d ;


	while (*s) {

	    if (*s == '.') {

	        *dp++ = '/' ;
	        s += 1 ;

	    } else
	        *dp++ = *s++ ;

	} /* end while */

	*dp++ = '\0' ;
	return (dp - s) ;
}
/* end subroutine (bbcpy) */



