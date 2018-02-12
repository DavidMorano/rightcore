/* bbucpy */

/* copy a string substituting slashes for where ever we find periods */


/* revision history:

	= 1994-05-01, David A­D­ Morano
        This code was written from scratch to support the correct operation of
        multiple directories for each newsgroup.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<localmisc.h>


/* exported subroutines */


int bbucpy(char *d,cchar *s)
{
	char		*dp = d ;

	while (*s) {
	    if (*s == '.') {
	        *dp++ = '/' ;
	        s += 1 ;
	    } else {
	        *dp++ = *s++ ;
	    }
	} /* end while */

	*dp++ = '\0' ;
	return (dp - s) ;
}
/* end subroutine (bbcpy) */


