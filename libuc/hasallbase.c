/* hasallbase */

/* test whether a string is composed of all characters of a given base */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	We test if a counted strin contains all of the proper digits consistent
	with the numeric base supplied.

	Synopsis:

	int hasallbase(sp,sl,base)
	const char	*sp ;
	int		sl ;
	int		base ;

	Arguments:

	sp		string to test
	sl		length of strin to test
	base		base to check against

	Returns:

	FALSE		string does not have all digits
	TRUE		string has all digits in it


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasallbase(cchar *sp,int sl,int b)
{
	int		ch ;
	int		v ;
	int		f = FALSE ;

	while (sl && *sp) {
	    ch = MKCHAR(*sp) ;
	    v = CHAR_TOVAL(ch) ;
	    f = (v < b) ;
	    f = f || ((ch == '-') || CHAR_ISWHITE(ch) || (ch == CH_NBSP)) ;
	    if (! f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasallbase) */


