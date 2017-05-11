/* starmat */

/* see if a string matches a "starmat" ('*') expresssion */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-17, David A­D­ Morano
        This subroutine was coded up to handle some PCS wild-card type matching.
        Do I have it correct? Unlike the stupid standard UNIX REGEX stuff, this
        subroutine is entirely reentrant!

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine does a simple wild-card pattern match. The only
        meta-pattern character allowed is the asterisk! Further, we only allow
        one asterisk in the whole pattern expression! Sorry, live with it.

	Synopsis:

	int starmat(se,s)
	const char	se[] ;
	const char	s[] ;

	Arguments:

	se		pattern to match against
	s		string to test for a match

	Returns:

	TRUE		match
	FALSE		no match


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int starmat(cchar *se,cchar *s)
{
	int		sl, sl1, sl2 ;
	int		f ;
	const char	*tp ;

	if ((tp = strchr(se,'*')) != NULL) {

	    f = FALSE ;
	    if (strncmp(s,se,(tp - se)) == 0) {

#if	CF_DEBUGS
		debugprintf("starmat: m1\n") ;
#endif

	        tp += 1 ;
	        sl1 = strlen(s) ;

	        sl2 = strlen(se) ;

	        sl = (se + sl2) - tp ;
	        f = (strncmp((s + sl1 - sl),tp,sl) == 0) ;

#if	CF_DEBUGS
		debugprintf("starmat: f=%u\n",f) ;
#endif

	    } /* end if */

	} else {
	    f = (strcmp(se,s) == 0) ;
	}

	return f ;
}
/* end subroutine (starmat) */


