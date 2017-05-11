/* sfcasesub */

/* match a substring within a larger string */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if the parameter string (argument 's2') is or
        is not in the buffer specified by the first two arguments. This
        subroutine either returns (-1) or it returns the length of the found
        substring.

	Synopsis:

	int sfcasesub(s1,len,s2,rpp)
	const char	*s1, *s2 ;
	int		len ;
	char		**rpp ;

	Arguments:

	s1	string to be examined
	len	length of string to be examined
	s2	null terminated substring to search for
	rpp	result pointer of beginning of found string

	Returns:

	>=0	length of found substring
	<0	the substring was not found in the main string buffer


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	BAD
#define	BAD	-1
#endif


/* external subroutines */

extern int	nleadcasestr(const char *,const char *,int) ;


/* exported subroutines */


int sfcasesub(cchar *sp,int sl,cchar *s2,cchar **rpp)
{
	const int	s2len = strlen(s2) ;
	int		i = 0 ;
	int		f = FALSE ;

	if (s2len > 0) {
	    if (sl < 0) sl = strlen(sp) ;
	    if (s2len <= sl) {
	        const int	s2lead = CHAR_TOLC(s2[0]) ;
	        int		m ;
	        for (i = 0 ; i <= (sl-s2len) ; i += 1) {
	            if (CHAR_TOLC(sp[i]) == s2lead) {
		        m = nleadcasestr((sp+i),s2,s2len) ;
		        f = (m == s2len) ;
			if (f) break ;
	            }
	        } /* end for */
	    } /* end if (possible) */
	} else {
	    f = TRUE ;
	}

	if (rpp != NULL) {
	    *rpp = ((f) ? (sp+i) : NULL) ;
	}

	return (f) ? s2len : -1 ;
}
/* end subroutine (sfcasesub) */


