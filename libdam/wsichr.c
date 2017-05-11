/* wsichr */

/* search for a character in a wide-string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Search for a character in a wide-string, returning an index of the found
        location, if found, and -1 otherwise.

	Synopsis:

	int wsichr(const wchar_t *sp,int sl,int sch)

	Arguments:

	sp	the source wide-string that is to be copied
	sl	length of wide-string
	sch	the search character (as an integer)

	Returns:

	>=0	the character pointer to the end of the destination
	<0	character was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stddef.h>		/* for 'wchar_t' */
#include	<localmisc.h>


/* external subroutines */

extern int	wsinul(const wchar_t *) ;


/* exported subroutines */


int wsichr(const wchar_t *wsp,int wsl,int sch)
{
	int		i ;
	int		f = FALSE ;
	if (wsl < 0) wsl = wsinul(wsp) ;
	for (i = 0 ; wsl-- && wsp[i] ; i += 1) {
	    f = (wsp[i] == sch) ;
	    if (f) break ;
	} /* end for */
	return (f) ? i : -1 ;
}
/* end subroutine (wsichr) */


