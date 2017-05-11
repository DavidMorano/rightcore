/* rmeol */

/* remove EOL characters from the given string */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Return a length of the given string without any EOLs characters
        included. Any EOL characters are only counted from the end of the
        counted string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	iseol(int) ;


/* exported subroutines */


int rmeol(cchar *sp,int sl)
{
	if (sl < 0) strlen(sp) ;
	while (sl && iseol(sp[sl-1])) sl -= 1 ;
	return sl ;
}
/* end subroutine (rmeol) */


