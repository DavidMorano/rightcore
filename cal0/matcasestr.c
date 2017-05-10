/* matcasestr */

/* match a case insensitive string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	- 1998-12-01, David A­D­ Morano

	Module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Check that the given string matches EXACTLY (case insensitively)
	some string in the given array of strings.  If we get a match,
	we return the array index.  If we do not match, we return
	"less-than-zero".

	Synopsis:

	int matcasestr(a,s,slen)
	const char	*a[] ;
	const char	s[] ;
	int		slen ;

	Arguments:

	a		array of string to match against
	s		string to test against array
	slen		length of test string

	Returns:

	>=0		index of match in array
	<0		no match found


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;


/* external variables */

#if	CF_CHAR
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#else /* CF_CHAR */
extern int	tolc(int) ;
extern int	touc(int) ;
#endif /* CF_CHAR */


/* forward references */


/* local variables */


/* exported subroutines */


int matcasestr(a,s,slen)
const char	*a[] ;
const char	s[] ;
int		slen ;
{
	register int	lc = tolc(s[0]) ;

	int	i ;
	int	m ;


	if (slen >= 0) {

	    for (i = 0 ; a[i] != NULL ; i += 1) {

		m = (lc == tolc(a[i][0])) ;
		if (m > 0)
		    m = nleadcasestr(a[i],s,slen) ;

		    if ((m == slen) && (a[i][m] == '\0'))
	                break ;

	    } /* end for */

	} else {

	    for (i = 0 ; a[i] != NULL ; i += 1) {

		m = (lc == tolc(a[i][0])) ;
		if (m > 0)
	            m = nleadcasestr(a[i],s,-1) ;

		    if ((a[i][m] == '\0') && (s[m] == '\0'))
	                break ;

	    } /* end for */

	} /* end if */

	return (a[i] != NULL) ? i : -1 ;
}
/* end subroutine (matcasestr) */



