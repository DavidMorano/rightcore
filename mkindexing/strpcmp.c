/* strpcmp */

/* compares the second string against the prefix of the first */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	= String Prefix Compare

	This subroutine compares two strings but only looks at the leading part
	(prefix) of the first string (s1) up to the length of the second string
	(s2).

	Synopsis:

	int strpcmp(s1,s2)
	const char	s1[], s2[] ;

	Arguments:

	s1	one string
	s2	second string

	Returns:

	>0	the first string is bigger than the second
	0	both strings are equal (as compared)
	<0	first string is less than the second


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int strpcmp(cchar *s1,cchar *s2)
{

	while (*s1 && *s2) {
	    if (*s1 != *s2) break ;
	    s1 += 1 ;
	    s2 += 1 ;
	} /* end while */

/* exact match */

	if ((*s1 | *s2) == 0)
	    return 0 ;

/* no match */

	if (*s1 == '\0')
	    return 1 ;

/* prefix match */

	if (*s2 == '\0')
	    return 0 ;

/* no match */

	return (*s1 - *s2) ;
}
/* end subroutine (strpcmp) */


