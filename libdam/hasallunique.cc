/* hasallunique */
/* lang=C++11 */

/* test whether a string consists of all unique characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check is all of the characters in the given string are Alpha
	characters.

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

extern char	*strwcpy(char *,const char *,int) ;


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


