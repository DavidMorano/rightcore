/* sifield */

/* subroutine to find the index of a character in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine searchs for a character in the given string and returns
        the index to either the search-character or to the end of the given
        string.

	Synopsis:

	int sifield(sp,sl,sch)
	const char	sp[] ;
	int		sl ;
	int		sch ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined
	sch	character to search for

	Returns:

	>=0	field length


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sifield(sp,sl,sch)
const char	sp[] ;
int		sl ;
int		sch ;
{
	register int	i ;
	register int	ch ;
	register int	f = FALSE ;


	sch &= 0xff ;
	for (i = 0 ; sl-- && sp[i] ; i += 1) {
	    ch = (sp[i] & 0xff) ;
	    f = (ch == sch) ;
	    if (f) break ;
	} /* end for */

	return i ;
}
/* end subroutine (sifield) */



