/* hasMeAlone */

/* test whether a string has only certain chacters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We test whether the given string consists only of those characters that
        symbolically represent the "current" user.

	Synopsis:

	int hasMeAlone(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp		string to test
	sl		length of strin to test

	Returns:

	FALSE		assertion fails
	TRUE		assertion succeeds


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasMeAlone(cchar *sp,int sl)
{
	int		f = FALSE ;
	if (sl < 0) strlen(sp) ;
	if (sl == 1) {
	    const int	ch = MKCHAR(*sp) ;
	    switch (ch) {
	    case '+':
	    case '-':
	    case '!':
		f = TRUE ;
		break ;
	    } /* end switch */
	} /* end if */
	return f ;
}
/* end subroutine (hasMeAlone) */


