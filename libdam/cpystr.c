/* cpystr */

/* copy a string */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Like 'strcpy' but returns the number of characters copied rather than
        some string pointer!

	Synopsis:

	int cpystr(s1,s2)
	char		*s1 ;
	const char	*s2 ;

	Arguments:

	s1		destination string
	s2		source string

	Returns:

	>=0		number of characters copied


*******************************************************************************/


#include	<envstandards.h>

#include	<localmisc.h>


/* exported subroutines */


int cpystr(d,s)
char		*d ;
const char	*s ;
{
	const char	*first = s ;

	while (*s != '\0')
	    *d++ = *s++ ;

	*d = '\0' ;
	return (s - first) ;
}
/* end subroutine (cpystr) */


