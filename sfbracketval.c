/* sfbracketval */

/* find the value within brackets */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before, if any.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We find a sub-string within brackets.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfbracketval(cchar *cp,int rs1,cchar **rpp)
{
	cchar		*cp2, *cp3 ;

	if ((cp2 = strnchr(cp,rs1,'<')) != NULL) {
	    cp2 += 1 ;
	    rs1 = (cp + rs1) - cp2 ;
	    if ((cp3 = strnchr(cp2,rs1,'>')) != NULL)
	        rs1 = cp3 - cp2 ;
	    cp = cp2 ;
	} /* end if */

	while ((rs1 > 0) && CHAR_ISWHITE(*cp)) {
	    cp += 1 ;
	    rs1 -= 1 ;
	} /* end while */

	while ((rs1 > 0) && CHAR_ISWHITE(cp[rs1 - 1])) {
	    rs1 -= 1 ;
	}

	if (rpp != NULL)
	    *rpp = cp ;

	return rs1 ;
}
/* end subroutine (sfbracketval) */


