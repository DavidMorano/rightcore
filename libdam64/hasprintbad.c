/* hasprintbad */

/* has bad-case characters? */


/* revision history:

	= 2008-10-10, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Does the given string have characters that are bad to print?

	Synopsis:

	int hasprintbad(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp		string to test
	sl		length of strin to test

	Returns:

	FALSE		string does not have bad stuff in it
	TRUE		string has some bad stuff in it


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* forward references */


/* external subroutines */

extern int	isprintbad(int) ;


/* exported subroutines */


int hasprintbad(cchar *sp,int sl)
{
	int		f = FALSE ;
	while (sl && *sp) {
	    const int	ch = MKCHAR(*sp) ;
	    f = isprintbad(ch) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	return f ;
}
/* end subroutine (hasprintbad) */


