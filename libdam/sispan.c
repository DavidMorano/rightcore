/* sispan */

/* subroutine to find a character in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the number of characters skipped in the string
	due to belonging to the specified character class.

	Synopsis:

	int sispan(sp,sl,class)
	const char	sp[] ;
	int		sl ;
	const char	class[] ;

	Arguments:

	sp	string to be examined
	sl	length of string of break character to break on
	class	string of characters defining a character class to skip

	Returns:

	>0	number of character skipped
	==0	no characters were skipped (no whitespace)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* external subroutines */


/* forward references */

static int	isclass(const char *,int) ;


/* exported subroutines */


int sispan(cchar *sp,int sl,cchar *class)
{
	int	i ;

	for (i = 0 ; sl-- && sp[i] ; i += 1) {
	    if (! isclass(class,sp[i])) break ;
	} /* end for */

	return i ;
}
/* end subroutine (sispan) */


/* local subroutines */


static int isclass(const char *class,int ch)
{
	int		i ;
	int		f = FALSE ;

	ch &= 0xff ;
	for (i = 0 ; class[i] ; i += 1) {
	    f = (ch == MKCHAR(class[i])) ;
	    if (f) break ;
	}

	return f ;
}
/* end subroutine (isclass) */


