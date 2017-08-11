/* sflast */

/* find the last <n> characters in a given string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find the last <n> characters in the given string.

	Synopsis:

	int sflast(sp,sl,n,rpp)
	cchar		sp[] ;
	int		sl ;
	int		n ;
	cchar		**rpp ;

	Arguments:

	sp		given string to test
	sl		length of string to test
	n		find this (<n>) number of last characters
	rpp		pointer to receive

	Returns:

	>=0		length of found string
	<0		bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* forward references */


/* local variables */


/* exported subroutines */


int sflast(cchar *sp,int sl,int n,cchar **rpp)
{
	if (sl < 0) sl = strlen(sp) ;
	if (sl < 0) {
	    while (CHAR_ISWHITE(*sp)) sp += 1 ;
	    sl = strlen(sp) ;
	} else {
	    while (sl && CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	        sl -= 1 ;
	    }
	} /* end if */
	if (rpp != NULL) *rpp = sp ;
	return sl ;
}
/* end subroutine (sflast) */


