/* cpywstr */

/* copy a counted string */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written but modeled after assembly.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is like 'strwcpy()' but it returns the count of the number
	of characters copied rather than a pointer to the character
	beyond the last character copied.

	Synopsis:

	int cpywstr(d,s,n)
	char		*d ;
	const char	*s ;
	int		n ;

	Arguments:

	d		destination string
	s		source string
	n		maximum number of characters to copy

	Returns:

	>=0		number of characters copied


*******************************************************************************/


#include	<envstandards.h>

#include	<localmisc.h>


/* exported subroutines */


int cpywstr(d,s,n)
char		*d ;
const char	*s ;
int		n ;
{
	const char	*first = s ;


	if (n >= 0) {
	    const char	*last = s + n ;

	    while ((*s != '\0') && (s < last))
	        *d++ = *s++ ;

	} else {

	    while (*s != '\0')
	        *d++ = *s++ ;

	} /* end if */

	*d = '\0' ;
	return (s - first) ;
}
/* end subroutine (cpywstr) */



