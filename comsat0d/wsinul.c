/* wsinul */

/* get length of a wide-string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Calculate the length of a wide-string.

	Synopsis:

	int wsinul(const wchar_t *wsp)

	Arguments:

	wsp	the source string that is to be copied

	Returns:

	-	the character pointer to the end of the destination


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stddef.h>		/* for 'wchar_t' */
#include	<localmisc.h>


/* exported subroutines */


int wsinul(const wchar_t *wsp)
{
	int	i = 0 ;
	while (wsp[i]) i += 1 ;
	return i ;
}
/* end subroutine (wsinul) */


