/* isprintlatin */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is sort of like 'isprint(3c)' but allows for ISO Latin-1
        characters also.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isprintlatin(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 0x100)) {
	    f = ((ch & 0x7f) >= 0x20) && (ch != CH_DEL) ;
	    f = f || (ch == CH_TAB) ;
	}
	return f ;
}
/* end subroutine (isprintlatin) */


