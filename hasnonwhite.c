/* hasnonwhite (Has-Non-White) */

/* determine if the given string has non-white content */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written. This is sort of experiemental
	(so far). We are struggling a little bit over its name.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if a given string is empty or not.

	Synopsis:

	int hasnonwhite(cchar *sp,int sl)

	Arguments:

	sp		string pointer
	sl		string length

	Returns:

	1		TRUE (has some non-white content)
	0		FALSE (string is empty)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasnonwhite(cchar *sp,int sl)
{
	int		f = FALSE ;
	if (sp != NULL) {
	    while (sl && *sp) {
	        f = (! CHAR_ISWHITE(*sp)) ;
	        if (f) break ;
	        sp += 1 ;
	        sl -= 1 ;
	    } /* end while */
	}
	return f ;
}
/* end subroutine (hasnonwhite) */


int hascontent(cchar *sp,int sl)
{
	return hasnonwhite(sp,sl) ;
}
/* end subroutine (hascontent) */


int hasStrContent(cchar *sp,int sl)
{
	return hasnonwhite(sp,sl) ;
}
/* end subroutine (hasStrContent) */


int hasNotEmpty(cchar *sp,int sl)
{
	return hasStrContent(sp,sl) ;
}
/* end subroutine (hasNotEmpty) */


