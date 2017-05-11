/* strwcmp */

/* compare the minimum common characters of two strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for a convenience.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine compares two strings, but the second string is allowed
	to be counted instead of NUL terminated.

	Synopsis:

	int strwcmp(s1,s2,s2len)
	const char	s1[], s2[] ;
	int		s2len ;

	Arguments:

	s1	one string
	s2	second string
	s2len	length of second string

	Returns:

	>0	the first string is bigger than the second
	0	both strings are equal (as compared)
	<0	first string is less than the second


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;


/* exported subroutines */


int strwcmp(cchar *s1,cchar *s2,int s2len)
{
	int		rc = MKCHAR(s1[0]) ;

	if (s2len < 0) 
	    s2len = strlen(s2) ;

	if (s2len > 0) {
	    rc = (s1[0] - s2[0]) ;
	    if (rc == 0) {
	        if ((rc = strncmp(s1,s2,s2len)) == 0) {
	            int m = nleadstr(s1,s2,s2len) ;
	            if (m < s2len) {
		        rc = (s1[m] - s2[m]) ;
		    } else {
		        rc = MKCHAR(s1[m]) ;
		    }
		}
	    }
	}

	return rc ;
}
/* end subroutine (strwcmp) */


