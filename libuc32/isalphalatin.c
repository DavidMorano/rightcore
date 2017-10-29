/* isalphalatin */

/* is the specified character an Alpha Latin character set? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is sort of like 'isalpha(3c)' but allows for ISO Latin-1
        (ISO-8859-1) characters also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<ctype.h>		/* *required* */
#include	<ascii.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int isalphalatin(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 0x100)) {
	    f = (ch < 0x80) && isalpha(ch) ;
	    f = f || ((ch >= 0xC0) && (ch != 0xD7) && (ch != 0xF7)) ;
	}
	return f ;
}
/* end subroutine (isalphalatin) */


