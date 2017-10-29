/* matostr */

/* matostr (match an "option" string) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check that the given string matches a MINIMUM number of leading
	characters for some string in the given array of strings.  If we get a
	match, we return the array index.  If we do not match, we return
	"less-than-zero".

	Synopsis:

	int matostr(a,n,sp,sl)
	const char	*a[] ;
	int		n ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	a		array of string to match against
	n		minimum number of characters that must match
	sp		string to test against array
	sl		length of test string

	Returns:

	>=0		index of match in array
	<0		no match found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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


int matostr(cchar **a,int n,cchar *sp,int sl)
{
	const int	lc = sp[0] ; /* ok: everything promotes similarly */
	int		i ;
	int		m ;

	if (sl < 0) sl = strlen(sp) ;

	for (i = 0 ; a[i] != NULL ; i += 1) {
	    if ((m = (lc == a[i][0])) > 0) {
	        m = nleadstr(a[i],sp,sl) ;
	    }
	    if ((m == sl) && ((m >= n) || (a[i][m] == '\0'))) {
		break ;
	    }
	} /* end for */

	return (a[i] != NULL) ? i : -1 ;
}
/* end subroutine (matostr) */


