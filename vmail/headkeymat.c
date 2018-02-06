/* headkeymat */

/* match on header keys */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        Module was originally written. This was written as part of the PCS
        mailer code cleanup!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Given a header key name 'key' such as "Subject", find if it is present
        in the user supplied string given as 'buf'. Return 0 if there is no
        match, else we return the character position of the start of the header
        value string. The match is case independent.

	Synopsis:

	int headkeymat(key,buf,buflen)
	const char	key[] ;
	const char	buf[] ;
	int		buflen ;

	Arguments:

	key		key name to test for
	buf		buffer holding string to test for a key in
	buflen		number of characters in the buffer

	Returns:

	>0		the key was found and the position of the value (not 
			the key) in the user supplied string under test is 
			returned

	==0		the key was not found
	<0		not possible (hopefully)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int headkeymat(cchar *key,cchar *buf,int buflen)
{
	int		bl = buflen ;
	const char	*bp = buf ;
	const char	*kp = key ;

	if (buflen < 0) {

	    while (*kp && (*kp != ':')) {

	        if (CHAR_TOLC(*bp) != CHAR_TOLC(*kp))
	            return 0 ;

	        kp += 1 ;
	        bp += 1 ;

	    } /* end while */

	    while (CHAR_ISWHITE(*bp))
	        bp += 1 ;

	} else {

	    while (*kp && (*kp != ':') && (bl > 0)) {

	        if (CHAR_TOLC(*bp) != CHAR_TOLC(*kp))
	            return 0 ;

	        kp += 1 ;
	        bp += 1 ;
	        bl -= 1 ;

	    } /* end while */

	    while (CHAR_ISWHITE(*bp) && (bl > 0)) {
	        bp += 1 ;
	        bl -= 1 ;
	    }

	} /* end if */

	if (*bp++ != ':')
	    return 0 ;

	if (buflen < 0) {

	    while (CHAR_ISWHITE(*bp))
	        bp += 1 ;

	} else {

	    while (CHAR_ISWHITE(*bp) && (bl > 0)) {
	        bp += 1 ;
	        bl -= 1 ;
	    }

	}

	return (bp - buf) ;
}
/* end subroutine (headkeymat) */


