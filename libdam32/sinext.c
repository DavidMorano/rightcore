/* sinext */

/* return index to end of next string-field */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find the index to the end of the first string field within the given
	string.

	Synopsis:

	int sinext(sp,sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined

	Returns:

	>=0	index of found substring
	<0	substring not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	ISWHITE(ch)	CHAR_ISWHITE(ch)


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external subroutines */


/* exported subroutines */


int sinext(cchar *sp,int sl)
{
	int		i = 0 ;
	int		f = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

/* skip leading shite-space */

	while ((i < sl) && ISWHITE(sp[i])) i += 1 ;

/* skip leading string-field */

	while ((i < sl) && sp[i] && (! ISWHITE(sp[i]))) i += 1 ;

/* return number */

	return i ;
}
/* end subroutine (sinext) */


