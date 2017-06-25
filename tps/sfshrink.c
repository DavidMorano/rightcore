/* sfshrink */

/* remove leading and trailing white space */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will identify the non-white-space portion of the buffer
	by ignoring white space at the beginning and at the end of the given
	buffer.  No modifications to the buffer are made.

	Synopsis:

	int sfshrink(sp,sl,rpp)
	const char	*sp;
	int		sl ;
	char		**rpp ;

	Arguments:

	sp	buffer
	sl	buffer length
	rpp	pointer to prointer to resulting string

	Returns:

	+ non-white-space string length (if OK), otherwise 0


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sfshrink(cchar *sp,int sl,cchar **rpp)
{
	if (sl < 0) {
	    while (CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	    }
	    sl = strlen(sp) ;
	} else {
	    while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	        sl -= 1 ;
	    } /* end while */
	    if (sp[0] == '\0') sl = 0 ;
	} /* end if */
	while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1])) {
	    sl -= 1 ;
	}
	if (rpp != NULL) *rpp = sp ;
	return sl ;
}
/* end subroutine (sfshrink) */


