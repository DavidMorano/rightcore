/* sfbreak */

/* subroutine to find a sub-string in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if any of the characters from the parameter
	string (argument 's2') are found in the primary string buffer
	specified.

	Synopsis:

	int sfbreak(s,slen,s2,rpp)
	const char	s[], s2[] ;
	int		slen ;
	const char	**rpp ;

	Arguments:

	s	string to be examined
	slen	length of string to be examined
	s2	null-terminated string of break characters to break on
	rpp	result pointer of beginning of found break=string

	Returns:

	>=0	length of the break-string
	-1	no characters from string 's2' were present in 's'


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int sfbreak(cchar *sp,int sl,cchar *s2,cchar **rpp)
{
	int		j = 0 ;
	int		f = FALSE ;

	if (sl < 0)
	    sl = strlen(sp) ;

	while (sl && *sp) {

	    for (j = 0 ; s2[j] ; j += 1) {
	        f = (*sp == s2[j]) ;
		if (f) break ;
	    }

	    if (f) break ;

	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	if (rpp != NULL) {
	    *rpp = (f) ? sp : NULL ;
	}

	return (f) ? sl : -1 ;
}
/* end subroutine (sfbreak) */


int sfbrk(cchar *sp,int sl,cchar *s2,cchar **rpp)
{
	return sfbreak(sp,sl,s2,rpp) ;
}
/* end subroutine (sfbrk) */


