/* hasallplusminus */

/* check the strong for consisting only of a plus or minus sign */


/* revision history:

	= 1998-10-10, David A­D­ Morano
        This subroutine was originally written but modeled from assembly
        language.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if the specified string consists of only a plus
	or a minus sign character.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* exported subroutines */


int hasallplusminus(cchar *sp,int sl)
{
	int		f = FALSE ;
	if (*sp != '\0') {
	    f = (sp[0] == '+') || (sp[0] == '-') ;
	    f = f && ((sl == 1) || (sp[1] == '\0')) ;
	}
	return f ;
}
/* end subroutine (hasallplusminus) */


