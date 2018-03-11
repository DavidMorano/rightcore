/* hasleadcolon */

/* test whether a string has a leading (before a slash) colon character */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Does the given string have a colon character (':') occur before
	a slash character ('/')?

	Synopsis:

	int hasleadcolon(sp,sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp		pointer to string
	sl		length of string

	Returns:

	0		no
	1		yes


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasleadcolon(cchar *sp,int sl)
{
	int		f = FALSE ;

	while (sl && *sp) {
	    f = (*sp == ':') ;
	    if (f) break ;
	    if (*sp == '/') break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (hasleadcolon) */


