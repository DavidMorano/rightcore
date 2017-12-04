/* headkey */

/* compare against a header key */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-05-01, David A­D­ Morano
        Module was originally written. This was written as part of the PCS
        mailer code cleanup !

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        Given a header key name 'name' such as "Subject", find if it is present
        in the user supplied string given as 'ts'. Return 0 if there is no
        match, else we return the character position of the header value string.
        The match is case independent.

	Synopsis:

	int headkey(key,ts,tslen)
	char	key[] ;
	char	ts[] ;
	int	tslen ;

	Arguments:

	key		key name to test for (must be NUL terminated)
	ts		string to test for a key in
	tslen		length (maximum) of test string

	Returns:

	>0		the key was found and the position of the 
			beginning of the value in the user supplied 
			test string is returned
	<=0		the key was not found


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<char.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int headkey(cchar *key,cchar *ts,int tslen)
{
	char		*kp = key ;
	char		*sp = ts ;

	if (tslen < 0) {

	    while (*kp && (*kp != ':')) {

	        if (CHAR_TOLC(*sp) != CHAR_TOLC(*kp))
	            return 0 ;

	        sp += 1 ;
	        kp += 1 ;

	    } /* end while */

	} else {

	    while (*kp && (*kp != ':') && *sp && (tslen > 0)) {

	        if (CHAR_TOLC(*sp) != CHAR_TOLC(*kp))
	            return 0 ;

	        kp += 1 ;
	        sp += 1 ;
	        tslen -= 1 ;

	    } /* end while */

	} /* end if */

	while ((tslen > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    tslen -= 1 ;
	}

	if (*sp++ != ':')
	    return 0 ;

	tslen -= 1 ;
	while ((tslen > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    tslen -= 1 ;
	}

	return (sp - key) ;
}
/* end subroutine (headkey) */


