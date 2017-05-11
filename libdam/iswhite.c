/* iswhite */

/* is the character white-space? */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Extra-note: Note that non-breaking-white-space (NBSP) characters are
        *not* considered to be white-space!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int iswhite(int ch)
{
	int		f = FALSE ;
	if ((ch >= 0) && (ch < 256)) {
	    f = CHAR_ISWHITE(ch) ;
	}
	return f ;
}
/* end subroutine (iswhite) */


