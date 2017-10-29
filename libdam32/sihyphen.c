/* sihyphen */

/* find a hyphen (a fake hyphen of two minus characters) in a string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This finds the string index of a fake hyphen in the given string
	('s1').  A fake hyphen is two minus characters in a row.

	Synopsis:

	int sihyphen(sp,sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined

	Returns:

	>=0	index of found substring
	-1	substring not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	strnchr(const char *,int,int) ;


/* exported subroutines */


int sihyphen(const char *sp,int sl)
{
	int		i ;
	int		f = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

	for (i = 0 ; (i < sl) && sp[i] ; i += 1) {
	    f = (sp[i] == '-') && ((i + 1) < sl) && (sp[i + 1] == '-') ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (sihyphen) */


