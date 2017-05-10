/* sichr */

/* subroutine to find the index of a character in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searchs for a character within a given string and
	returns the index to that character (if it is found).  It returns
	-1 if the character does not exist within the given string.

	Synopsis:

	int sichr(sp,sl,sch)
	const char	sp[] ;
	int		sl ;
	int		sch ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on
	sch	character to search for

	Returns:

	>=0	index of search character in the given string
	<0	the search character was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sichr(sp,sl,sch)
const char	sp[] ;
int		sl ;
int		sch ;
{
	register int	i ;
	register int	ch ;
	register int	f = FALSE ;


	sch &= 0xff ;
	for (i = 0 ; sl && sp[i] ; i += 1) {
	    ch = (sp[i] & 0xff) ;
	    f = (ch == sch) ;
	    if (f) break ;
	    sl -= 1 ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (sichr) */



