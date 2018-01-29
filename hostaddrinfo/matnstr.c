/* matnstr */

/* match a counted string */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check that the given string exactly matches the leading part of some
	string in the given array of strings.  If we get a match, we return the
	array index.  If we do not match, we return "less-than-zero".

	Synopsis:

	int matnstr(a,sp,sl)
	const char	*a[] ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	a		array of string to match against
	sp		string to test against array
	sl		length of test string

	Returns:

	>=0		index of match in array
	<0		no match found


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int matnstr(cchar **a,cchar *sp,int sl)
{
	int		lc = sp[0] ; /* ok: everything promotes the same */
	int		i ;

	if (sl < 0) sl = strlen(sp) ;

	for (i = 0 ; a[i] != NULL ; i += 1) {
	    if ((lc == a[i][0]) && (strncmp(a[i],sp,sl) == 0)) break ;
	} /* end for */

	return (a[i] != NULL) ? i : -1 ;
}
/* end subroutine (matnstr) */


