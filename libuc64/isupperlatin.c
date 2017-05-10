/* isupperlatin */

/* is the specified character something in the Latin character set? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is sort of like 'isupper(3c)' but allows for ISO Latin-1
        (ISO-8859-1) characters also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int isupperlatin(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 0x100)) {
	    f = CHAR_ISUC(ch) ;
	}
	return f ;
}
/* end subroutine (isupperlatin) */


