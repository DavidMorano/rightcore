/* cupper */

/* convert characters to upper case */
/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written but adapted from assembly.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*
; string library subroutines

;	This subroutine converts a string of characters to upper case.

*/


#include	<envstandards.h>


/* exported subroutines */


int cupper(src,dst,len)
const char	*src ;
char		*dst ;
int		len ;
{
	int	i ;


	if (len <= 0) {

	    len = 0 ;
	    while (*src != '\0') {

	        *dst++ = 
	            ((*src >= 'a') && (*src <= 'z')) ? (*src++ - 32) : *src++ ;

	        len += 1 ;

	    } /* end for */

	} else {

	    for (i = 0 ; i < len ; i += 1) {

	        *dst++ = 
	            ((*src >= 'a') && (*src <= 'z')) ? (*src++ - 32) : *src++ ;

	    } /* end for */

	} /* end if */

	return len ;
}
/* end subroutine (cupper) */



