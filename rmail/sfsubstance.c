/* sfsubstance */

/* find substantive part of string (strip garbage around items) */


/* revision history:

	= 2005-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds the substantive part of the given string.
	Basically, some qutoing is removed from the beginning and end of the
	given string (if present).  This sort of function is useful for
	removing some quoting-like cruft from EMA "from" strings as found in
	mail-message FROM headers.

	Synopsis:

	int sfsubstance(sp,sl,cpp)
	const char	*sp ;
	int		sl ;
	const char	**cpp ;

	Arguments:

	sp	supplied string to test
	sl	length of supplied string to test
	cpp	pointer to store start of found string

	Returns:

	>=0	length of result
	<0	error

	Note: Integer promotion of characters is not a problem since:
	a. we do not use the <ctype.h> functions
	b. we do not try to compare with 8-bit characters


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* forward references */

static int	isnotours(int) ;


/* exported subroutines */


int sfsubstance(cchar *sp,int sl,cchar **cpp)
{
	int		ch ;

	if (sl < 0)
	    sl = strlen(sp) ;

	while (sl > 0) {
	    ch = MKCHAR(sp[0]) ;
	    if (isnotours(ch)) break ;
	    sp += 1 ;
	    sl -= 1 ;
	}

	while (sl > 0) {
	    ch = MKCHAR(sp[sl-1]) ;
	    if (isnotours(ch)) break ;
	    sl -= 1 ;
	}

	if (cpp != NULL) {
	    *cpp = sp ;
	}

	return sl ;
}
/* end subroutine (sfsubstance) */


/* local subroutines */


static int isnotours(int ch)
{
	int	f = FALSE ;
	f = f || CHAR_ISWHITE(ch) ;
	f = f || (ch == CH_DQUOTE) ;
	f = f || (ch == CH_SQUOTE) ;
	return (! f) ;
}
/* end subroutine (isnotours) */


