/* matkeystr */

/* match the key part of a string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check that the key of the given string matches EXACTLY some key-string
	in the given array of strings.  If we get a match, we return the array
	index.  If we do not match, we return something "less-than-zero."

	Synopsis:

	int matkeystr(a,sp,sl)
	const char	*a[] ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	a		array of strings to match against
	sp		string to test against array
	sl		length of test string

	Returns:

	>=0		index of match in array
	<0		no match found


	Design note: Whew!  This code is trying to be cleaver!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define	KEYEND(c)	(((c) == '\0') || ((c) == '='))


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	nleadkeystr(const char *,const char *,int) ;
extern int	strnkeycmp(const char *,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;


/* local variables */


/* exported subroutines */


int matkeystr(cchar **a,cchar *sp,int sl)
{
	int		sch = sp[0] ; /* ok: everything promotes the same */
	int		i ;
	int		f = FALSE ;

	if (sl >= 0) {
	    int		m ;
	    for (i = 0 ; a[i] != NULL ; i += 1) {
		m = (sch == a[i][0]) ;
		if (m > 0) {
		    m = nleadkeystr(a[i],sp,sl) ;
		}
		f = (((m == sl) || KEYEND(sp[m])) && KEYEND(a[i][m])) ;
		if (f) break ;
	    } /* end for */
	} else {
	    for (i = 0 ; a[i] != NULL ; i += 1) {
	        if (sch == a[i][0]) {
		    f = (strkeycmp(a[i],sp) == 0) ;
		} else {
		    f = (KEYEND(sp[0]) && KEYEND(a[i][0])) ;
		}
		if (f) break ;
	    } /* end for */
	} /* end if */

	return (f) ? i : -1 ;
}
/* end subroutine (matkeystr) */


