/* snwcpyrev */

/* copy in reverse the characters from a source to a destiation */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Copy characters to a destiation string in reverse order from a source
	string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* module global variables */


/* local variables */


/* exported subroutines */


int snwcpyrev(char *dbuf,int dlen,cchar *sp,int sl)
{
	if (dbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	if ((dlen < 0) || (dlen >= sl)) {
	    int		i ;
	    for (i = 0 ; i < sl ; i += 1) {
	        dbuf[i] = sp[sl-i-1] ;
	    } /* end for */
	    dbuf[i] = '\0' ;
	} else {
	    sl = SR_OVERFLOW ;
	}
	return sl ;
}
/* end subroutine (snwcpyrev) */


