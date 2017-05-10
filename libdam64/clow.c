/* clow */

/* ; string library subroutines */
/* last modified %G% version %I% */


/*
;	This file contains many of the string manipulation subroutines
;	used in other modules.
*/



#include	"localmisc.h"




/* routine to convert a counted string to lower case */
char *clow(len,src,dst)
int	len ;
char	*src, *dst ;
{
	register int	i ;

	for (i = 0 ; i < len ; i += 1) {

	    *dst++ = 
	       ((*src >= 'A') && (*src <= 'Z')) ? (*src++ + 32) : *src++ ;

	}

	return dst ;
}
/* end subroutine (clow) */


