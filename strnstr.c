/* strnstr */

/* find a substring within a larger string */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if the parameter string (argument 's2') is or
        is not a substring specified by the first two arguments. This subroutine
        either returns a pointer to the the begining of the found substring or
        NULL if not found.

	Synopsis:

	int strnstr(sp,sl,ss)
	const char	*sp ;
	int		sl ;
	const char	*ss ;

	Arguments:

	sp		(s1) string to be examined
	sl		(s1) length of string to be examined
	ss		null terminated substring to search for

	Returns:

	-		pointer to found substring
	NULL		substring was not found
	sp		pointer to 's1' if 'ss' is zero length


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* external variables */


/* forward references */

static char	*strnstr_local(const char *,int,const char *) ;


/* local variables */


/* exported subroutines */


char *strnstr(cchar *sp,int sl,cchar *ss)
{
	char		*rp ;
	if (sl >= 0) {
	    rp = strnstr_local(sp,sl,ss) ;
	} else {
	    rp = strstr(sp,ss) ;
	}
	return rp ;
}
/* end subroutine (strnstr) */


/* local subroutines */


char *strnstr_local(cchar *sp,int sl,cchar *s2)
{
	const int	s2len = strlen(s2) ;
	char		*rp = (char *) sp ;

	if (s2len > 0) {
	    if (sl < 0) sl = strlen(sp) ;
	    if (s2len <= sl) {
	        int		i ;
		int		f = FALSE ;
	        for (i = 0 ; i <= (sl-s2len) ; i += 1) {
	            f = ((s2len == 0) || (sp[i] == s2[0])) ;
	            f = f && (strncmp((sp+i),s2,s2len) == 0) ;
		    if (f) break ;
	        } /* end for */
	        rp = (char *) ((f) ? (sp+i) : NULL) ;
	    } else {
	        rp = NULL ;
	    }
	} /* end if (positive) */

	return rp ;
}
/* end subroutine (strnstr_local) */


