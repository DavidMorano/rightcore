/* strnncpy */

/* copy a string and fill destination out with NULs */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy a source string to a destination until either the end of the
	source string is reached (by its end-marker) or the length of the
	source string is exhausted.  Additionally we always zero out to the
	length of the destination string.

	Synopsis:

	char *strnncpy(d,s,slen,size)
	char		*d ;
	const char	*s ;
	int		slen, size ;

	Arguments:

	d	string buffer that receives the copy
	s	the source string that is to be copied
	slen	length of string to copy
	n	the maximum length to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


char *strnncpy(char *d,cchar *sp,int sl,int size)
{
	if (sl >= 0) {
	    const int	ml = MIN(sl,size) ;
	    strncpy(d,sp,ml) ;
	    if (ml < size) {
	        memset((d + ml),0,(size - ml)) ;
	    }
	} else {
	    strncpy(d,sp,size) ;
	}
	return (d + size) ;
}
/* end subroutine (strnncpy) */


