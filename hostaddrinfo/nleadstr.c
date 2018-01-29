/* nleadstr */

/* match on the leading part of a string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Calculate the number of characters that two string have in common from
        their leading edges. If we get a match at all we return the number of
        characters matched. If we do not get a match, we return a negative
        number. The second given string is allowed to have an optional length
        supplied.

	Synopsis:

	int nleadstr(bs,sp,sl)
	const char	bs[] ;
	const char	*sp ;
	int		sl ;

	Arguments:

	bs		base string to compare against
	sp		test-string to test against the base string
	sl		length of test-string

	Returns:

	>=0		match found and it matched up to this length
	<0		no match


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int nleadstr(cchar *bs,cchar *sp,int sl)
{
	int		i ;

	if (sl < 0) sl = INT_MAX ;

	for (i = 0 ; (i < sl) && bs[i] && sp[i] ; i += 1) {
	    if (bs[i] != sp[i]) break ;
	} /* end for */

	return i ;
}
/* end subroutine (nleadstr) */


