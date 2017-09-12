/* matcasesubstr */

/* matcasesubstr (match a string) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Check is the given substring is amoung the array of strings given (case
        insensitively).

	Synopsis:

	int matcasesubstr(a,sp,sl)
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

extern int	sisub(cchar *,int,cchar *) ;
extern int	sicasesub(cchar *,int,cchar *) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	nleadcasestr(cchar *,cchar *,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int matcasesubstr(cchar **a,cchar *sp,int sl)
{
	int		i ;
	for (i = 0 ; a[i] != NULL ; i += 1) {
	    if (sicasesub(sp,sl,a[i]) >= 0) break ;
	} /* end for */
	return (a[i] != NULL) ? i : -1 ;
}
/* end subroutine (matcasesubstr) */


