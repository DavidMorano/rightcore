/* hasallunique */
/* lang=C++11 (interface=C) */

/* test whether a string consists of all unique characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2003-08-21, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check if all of the characters in the given string are unique
	(appears only once in the whole string).

	Synopsis:
	int hasallunique(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:
	sp		string to test
	sl		length of string to test

	Returns:
	FALSE		string does not have all digits
	TRUE		string has all digits in it


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern "C" int	hasallunique(cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static void	bool_init(bool *,int) ;


/* local variables */


/* exported subroutines */


int hasallunique(cchar *sp,int sl)
{
	bool		seen[256] ;
	int		ch ;
	int		f = TRUE ;

	bool_init(seen,256) ;

	while (sl-- && *sp) {
	    ch = MKCHAR(*sp) ;
	    f = seen[ch] ;
	    if (! f) break ;
	    seen[ch] = true ;
	    sp += 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasallunique) */


/* local subroutines */


static void bool_init(bool *bp,int bl)
{
	int	i ;
	for (i = 0 ; i < bl ; i += 1) bp[i] = false ;
}
/* end subroutine (bool_init) */


