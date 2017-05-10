/* strlcpy */

/* buffer-size-conscious string operation */


/* revision history:

	= 1998-08-01, David A­D­ Morano

	This subroutine was written because I want to use these
	new (experimental ?) subroutines on platforms other than
	Solaris !!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is a knock off of the 'strlcpy()' that first
	appeared in the Solaris UNIX system from Sun Microsystems.


******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>


/* exported subroutines */


size_t strlcpy(dst,src,maxlen)
char		dst[] ;
const char	src[] ;
size_t		maxlen ;
{
	int	i ;

	for (i = 0 ; (i < (maxlen - 1)) && *src ; i += 1)
	    dst[i] = *src++ ;

	dst[i] = '\0' ;
	return (*src == '\0') ? i : (i+strlen(src)) ;
}
/* end subroutine (strlcpy) */


