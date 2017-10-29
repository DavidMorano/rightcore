/* matstr */

/* matstr (match a string) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	Module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check that the given string matches EXACTLY some string in the given
	array of strings.  This is not a prefix match.  If we get a match, we
	return the array index.  If we do not match, we return
	"less-than-zero".

	Synopsis:

	int matstr(a,sp,sl)
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


int matstr(cchar **a,cchar *sp,int sl)
{
	register int	lc = sp[0] ; /* ok: everything promotes similarly */
	register int	i ;

	if (sl >= 0) {
	    register int	m ;

	    for (i = 0 ; a[i] != NULL ; i += 1) {
		m = ((sl > 0) && (lc == a[i][0])) ;
		if (m > 0) m = nleadstr(a[i],sp,sl) ;
		if ((m == sl) && (a[i][m] == '\0')) break ;
	    } /* end for */

	} else {

	    for (i = 0 ; a[i] != NULL ; i += 1) {
		if (lc == a[i][0]) {
	            if (strcmp(a[i],sp) == 0) break ;
		} /* end if */
	    } /* end for */

	} /* end if */

	return (a[i] != NULL) ? i : -1 ;
}
/* end subroutine (matstr) */


