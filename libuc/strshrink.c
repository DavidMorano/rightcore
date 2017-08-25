/* strshrink */

/* remove leading and trailing white space */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine ehrink returns a string that is the same as the
        originally specified one except with any leading and trailing
        white-space removed.

	Note that this subroutine modifies the given (passed) string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>
#include	<char.h>


/* local defines */


/* exported subroutines */


char *strshrink(char *s)
{
	int		l ;

/* skip leading white space */

	while (CHAR_ISWHITE(*s)) {
	    s += 1 ;
	}

	l = strlen(s) ;

/* skip trailing white space */

	while ((l > 0) && CHAR_ISWHITE(s[l-1])) {
	    l -= 1 ;
	}

	s[l] = '\0' ;
	return s ;
}
/* end subroutine (strshrink) */


