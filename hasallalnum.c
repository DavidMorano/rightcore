/* hasallalnum */

/* test whether a string is composed of all Al-Num characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Are all of the characters Alpha-Num?

	Synopsis:

	int hasallalnum(sp,sl)
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
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isdigitlatin(int) ;
extern int	isalnumlatin(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasallalnum(cchar *sp,int sl)
{
	int		ch ;
	int		f = TRUE ;

	while (sl && *sp) {
	    ch = MKCHAR(*sp) ;
	    f = isalnumlatin(ch) ;
	    if (! f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasallalnum) */


