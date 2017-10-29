/* sibreak */

/* subroutine to find a character in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if any of the characters from the parameter
	string (argument 's2') are found in the primary string buffer specified
	('s').  This subroutine is almost identical to the 'strcspn(3c)'
	subroutine except that an additional argument ('rpp') can be specified
	with this subroutine.

	Synopsis:

	int sibreak(sp,sl,s2)
	const char	sp[], s2[] ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on
	s2	NULL-terminated substring to search for

	Returns:

	>=0	position (in 's') of a character from 's2'
	<0	no characters from string 's2' were present in 's'


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sibreak(const char *sp,int sl,const char *s2)
{
	int		i, j ;
	int		f = FALSE ;

	for (i = 0 ; sl && sp[i] ; i += 1) {
	    for (j = 0 ; s2[j] ; j += 1) {
	        f = (sp[i] == s2[j]) ;
		if (f) break ;
	    }
	    if (f) break ;
	    sl -= 1 ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (sibreak) */


