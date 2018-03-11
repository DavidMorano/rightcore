/* isMacro */
/* lang=C99 */

/* determine if a string is a macro-start */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2018-02-14, David A­D­ Morano
	This is a refactored fragment from the existing code family of various
	TROFF pre-processing utilities.

*/

/* Copyright © 2018 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine determines if the given string contains the start of
	a TROFF-type macro.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* forward references */

static int	twochars(char *,cchar *,int) ;


/* external variables */


/* local variables */


/* exported subroutines */


int isMacro(cchar *lp,int ll) {
	int	f = FALSE ;
	if ((ll > 1) && (lp[0] == '.')) {
	    char	tbuf[3] ;
	    if ((f = twochars(tbuf,(lp+1),(ll-1))) > 1) {
	        f = (strcmp(tbuf,"\\\"") != 0) ;
	    }
	}
	return f ;
}
/* end subroutine (isMacro) */


/* local subroutines */


static int twochars(char *tbuf,cchar *sp,int sl) {
	int		c = 0 ;
	while (sl && (c < 2)) {
	    if (! CHAR_ISWHITE(*sp)) {
		tbuf[c++] = *sp ;
	    }
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	tbuf[c] = '\0' ;
	return c ;
}
/* end subroutine (twochars) */


