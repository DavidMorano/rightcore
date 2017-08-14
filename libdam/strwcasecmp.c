/* strwcasecmp */

/* compare (case insentively) the minimum common characters of two strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for a convenience.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine compares two strings, but the second string is allowed
	to be counted instead of NUL terminated.

	Synopsis:

	int strwcasecmp(s1,s2,s2len)
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
#include	<strings.h>		/* |strncasecmp()| */

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	tolc
#define	tolc(ch)	CHAR_TOLC(ch)
#endif


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	nleadstr(cchar *,cchar *,int) ;
extern int	nleadcasestr(cchar *,cchar *,int) ;


/* exported subroutines */


int strwcasecmp(cchar *s1,cchar *s2,int s2len)
{
	int		rc = MKCHAR(s1[0]) ;

	if (s2len < 0) 
	    s2len = strlen(s2) ;

	if (s2len > 0) {
	    rc = (s1[0] - s2[0]) ;
	    if (rc == 0) {
	        if ((rc = strncasecmp(s1,s2,s2len)) == 0) {
	            int m = nleadcasestr(s1,s2,s2len) ;
	            if (m < s2len) {
			int	ch1 = MKCHAR(s1[m]) ;
			int	ch2 = MKCHAR(s2[m]) ;
		        rc = (tolc(ch1) - tolc(ch2)) ;
		    } else {
			int	ch = MKCHAR(s1[m]) ;
		        rc = tolc(ch) ;
		    }
		}
	    }
	}

	return rc ;
}
/* end subroutine (strwcasecmp) */


