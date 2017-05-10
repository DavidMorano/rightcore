/* strwhite */

/* find the next white space character in a string */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FASTER	1		/* cheap attempt as faster */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will scan a string and return the first
	white-space-like character found.  This subroutine is really just a
	short cut for something like:

		char *strpbrk(s," \v\t\r\n")
		const char	s[] ;

	Synopsis:

	char *strwhite(s)
	const char	*s ;

	Arguments:

	s		string to search in

	Returns:

	NULL		if no white space was found
	!= NULL		the pointer to the first white space character


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


char *strwhite(cchar *s)
{

#if	CF_FASTER
	return strpbrk(s," \t\r\n\v") ;
#else
	while ((*s != '\0') && (! CHAR_ISWHITE(*s)))
	    s += 1 ;

	return ((s != '\0') ? s : NULL) ;
#endif

}
/* end subroutine (strwhite) */


