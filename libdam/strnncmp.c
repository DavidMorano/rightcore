/* strnncmp */

/* compare the minimum common characters of two strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
        Originally written for a convenience so that I do not have to have this
        same function everywhere. How useful is this? I do not know.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine compares two strings but only looks at the minimum
	number of characters that each string has in common as determined by
	the length of each string.

	Synopsis:

	int strnncmp(s1,n1,s2,n2)
	const char	s1[], s2[] ;
	int		n1, n2 ;

	Arguments:

	s1	one string
	n1	length of first string
	s2	second string
	n2	length of second string

	Returns:

	>0	the first string is bigger than the second
	0	both strings are equal (as compared)
	<0	first string is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int strnncmp(cchar *s1,int n1,cchar *s2,int n2)
{
	int		rc ;
	int		n ;

	if (n1 < 0) 
	    n1 = strlen(s1) ;

	if (n2 < 0) 
	    n2 = strlen(s2) ;

	n = MIN(n1,n2) ;
	if ((rc = strncmp(s1,s2,n)) == 0) {
	    rc = (n1 - n2) ;
	}

	return rc ;
}
/* end subroutine (strnncmp) */


