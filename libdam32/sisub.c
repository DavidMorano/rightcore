/* sisub */

/* match a substring within a larger string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if the parameter string (argument 's2') is
	or is not in the buffer specified by the first two arguments.  This
	subroutine either returns (-1) or it returns the character position in
	the buffer of where the string starts.

	Synopsis:

	int sisub(sp,sl,s2)
	const char	*sp ;
	const char	*s2 ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined
	s2	null terminated substring to search for

	Returns:

	>=0	index of found substring
	<0	substring not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external subroutines */

extern int	nleadstr(cchar *,cchar *,int) ;


/* exported subroutines */


int sisub(cchar *sp,int sl,cchar *s2)
{
	const int	s2len = strlen(s2) ;
	int		i = 0 ;
	int		f = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

	if (s2len <= sl) {
	    int	m ;
	    for (i = 0 ; i <= (sl-s2len) ; i += 1) {
		f = ((s2len == 0) || (sp[i] == s2[0])) ;
		if (f) {
	     	    m = nleadstr((sp+i),s2,s2len) ;
	     	    f = (m == s2len) ;
		}
	        if (f) break ;
	    } /* end for */
	} /* end if (possible) */

	return (f) ? i : -1 ;
}
/* end subroutine (sisub) */


