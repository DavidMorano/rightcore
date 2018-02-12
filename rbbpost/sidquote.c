/* sidquote */

/* find the index of the character past a double-quoted string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searches for the end of a double-quoted string.

	Synopsis:

	int sidquote(sp,sl)
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

#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sidquote(const char *sp,int sl)
{
	int		i ;
	int		ch ;
	int		f_skip = FALSE ;
	int		f_done = FALSE ;

	for (i = 0 ; sl && sp[i] && (! f_done) ; i += 1) {
	    ch = MKCHAR(sp[i]) ;
	    switch (ch) {
	    case '\\':
		f_skip = (! f_skip) ;
		break ;
	    case CH_DQUOTE:
		if (! f_skip) f_done = TRUE ;
/* FALLTHROUGH */
	    default:
		f_skip = FALSE ;
		break ;
	    } /* end switch */
	    sl -= 1 ;
	} /* end for */

	return i ;
}
/* end subroutine (sidquote) */


