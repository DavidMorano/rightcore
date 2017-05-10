/* strncasesub */

/* find a substring within a larger string (case insensitive) */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if the parameter string (argument 's2') is or
        is not a substring specified by the first two arguments. This subroutine
        either returns a pointer to the the begining of the found substring or
        NULL if not found. The string comparisons are case insensitive.

	Synopsis:

	char *strncasesub(sp,sl,s2)
	const char	sp[] ;
	int		sl ;
	const char	s2[] ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined
	s2	NUL-terminated substring to search for

	Returns:

	-	pointer to found substring
	NULL	substring was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern char	*strnstr(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* local variables */


/* exported subroutines */


char *strncasesub(cchar *sp,int sl,cchar *ss)
{

	return strncasestr(sp,sl,ss) ;
}
/* end subroutine (strncasesub) */


