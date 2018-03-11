/* sileader */

/* get the ... something ... a "leader" (whatever that is)? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check for some condition.


*******************************************************************************/


#define	SILEADER_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ctdec.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	siskipwhite(cchar *,int) ;
extern int	sibreak(cchar *,int,cchar *) ;
extern int	siskipwhite(cchar *,int) ;
extern int	isdigitlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int sileader(sp,sl)
const char	*sp ;
int		sl ;
{
	int		si = 0 ;

	if (sl > 0) {
	    int	ch ;
	    if ((si = siskipwhite(sp,sl)) > 0) {
	        sp += si ;
	        sl -= si ;
	    }
	    ch = MKCHAR(sp[0]) ;
	    if (isdigitlatin(ch)) {
		int	ci ;
	        if ((ci = sibreak(sp,sl," \t")) > 0) {
	            si += ci ;
	            si += siskipwhite((sp + ci),(sl - ci)) ;
	        } else
	            si = 0 ;
	    } /* end if (is-digit) */
	} /* end if (positive) */

	return si ;
}
/* end subroutine (sileader) */


