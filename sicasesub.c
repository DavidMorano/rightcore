/* sicasesub */

/* match a substring within a larger string */


#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if the parameter string (argument 's2') is or
        is not in the buffer specified by the first two arguments. This
        subroutine either returns (-1) or it returns the character position in
        the buffer of where the string starts.

	Synopsis:

	int sicasesub(sp,sl,s2)
	const char	*sp, *s2 ;
	int		sl ;

	Arguments:

	sp	string to be examined
	sl	length of string to be examined
	s2	null terminated substring to search for

	Returns:

	>=0	index of found substring
	<0	substring not found

	Notes:

	Q. Why are we using |nleadcasestr(3dam)| rather than |strncascmp(3c)|?
	A. I do not really know but could it be that |strncasecmp(3c)|
	   messes up somehow on 8-bit characters?  It should not be the case.
	   Maybe someone, somewhere, was broken at one time and I used
	   |nleadcasestr(3dam)| as a fix.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_CHAR
#define		tolc(c)		CHAR_TOLC(c)
#define		touc(c)		CHAR_TOUC(c)
#else /* CF_CHAR */
extern int	tolc(int) ;
extern int	touc(int) ;
#endif /* CF_CHAR */

extern int	nleadcasestr(cchar *,cchar *,int) ;


/* external variables */


/* exported subroutines */


int sicasesub(cchar *sp,int sl,cchar *s2)
{
	const int	s2len = strlen(s2) ;
	int		i = 0 ;
	int		f = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

	if (s2len <= sl) {
	    const int	s2lead = tolc(s2[0]) ;
	    int		m ;
	    for (i = 0 ; i <= (sl-s2len) ; i += 1) {
		f = ((s2len == 0) || (tolc(sp[i]) == s2lead)) ;
		if (f) {
	            m = nleadcasestr((sp+i),s2,s2len) ;
	            f = (m == s2len) ;
		}
	        if (f) break ;
	    } /* end for */
	} /* end if (possible) */

	return (f) ? i : -1 ;
}
/* end subroutine (sicasesub) */


