/* sfskipwhite */

/* remove leading white space */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will identify the non-white-space portion of the buffer
	by ignoring white space at the beginning of the given buffer.  No
	modifications to the buffer are made.

	Synopsis:

	int sfskipwhite(sp,sl,rpp)
	const char	sp[] ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		given string to test
	sl		length of string to test
	rpp		pointer to receive

	Returns:

	>=0		non-white-space string length
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


int sfskipwhite(cchar *sp,int sl,cchar **rpp)
{
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
/* end subroutine (sfskipwhite) */


