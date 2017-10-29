/* sialnum */

/* subroutine to find the index of an alpha-numberic character */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searchs for an alpha-numberic character within a given
	string and returns the index to that character (if it is found).  It
	returns -1 if the character does not exist within the given string.

	Synopsis:

	int sialnum(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on

	Returns:

	>=0	index of search character in the given string
	<0	the search character was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;


/* exported subroutines */


int sialnum(const char *sp,int sl)
{
	int		i ;
	int		ch ;
	int		f = FALSE ;

	for (i = 0 ; sl && sp[i] ; i += 1) {
	    ch = MKCHAR(sp[i]) ;
	    f = isalnumlatin(ch) ;
	    if (f) break ;
	    sl -= 1 ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (sialnum) */


