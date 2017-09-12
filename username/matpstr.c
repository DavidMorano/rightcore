/* matpstr */

/* match a partial string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check that the given string matches the LEADING part of some string in
	the given array of strings.  All of the strings in the given array of
	strings are checked and the one that matches the most characters (if
	any) is the one whose index is returned.

	If we get a match, we return the array index.  If we do not match, we
	return "less-than-zero".

	Synopsis:

	int matpstr(a,n,sp,sl)
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


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<localmisc.h>


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int matpstr(cchar **a,int n,cchar *sp,int sl)
{
	int		si ;

	if (sl < 0) sl = strlen(sp) ;

	if (n >= 0) {
	    const int	lc = sp[0] ; /* exveryting promotes */
	    int		i ;
	    int		m_max = 0 ;
	    int		m ;
	    si = -1 ;
	    for (i = 0 ; a[i] != NULL ; i += 1) {
		if ((m = (lc == a[i][0])) > 0) {
		    m = nleadstr(a[i],sp,sl) ;
		}
		if (((m >= n) && (m == sl)) || (a[i][m] == '\0')) {
		    if (m > m_max) {
			m_max = m ;
			si = i ;
		    }
		} /* end if */
	    } /* end for */
	} else {
	    si = matstr(a,sp,sl) ;
	}

	return si ;
}
/* end subroutine (matpstr) */


