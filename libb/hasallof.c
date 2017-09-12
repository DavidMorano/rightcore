/* hasallof */

/* does the given string have all of the given characters */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Determine if the given string has all of the specified characters within
        it.

	Synopsis:
	int hasallof(cchar *sp,int sl,cchar *tstr)

	Arguments:
	sp		source string
	sl		length of source string
	tstr		string of characters to test against

	Returns:
	<0		error
	==0		failed, did not have all characters specified
	>0		yes, the string has all of the characters specified


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<dupstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sichr(const char *,int,int) ;


/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int hasallof(cchar *sp,int sl,cchar *tstr)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = TRUE ;

	if (tstr[0] != '\0') {
	    DUPSTR	sd ;
	    char	*bp ;
	    f = FALSE ;
	    if ((rs = dupstr_start(&sd,tstr,-1,&bp)) >= 0) {
	        int	bl = rs ;
	        int	si ;
	        while (sl && *sp) {
		    if ((si = sichr(bp,bl,sp[0])) >= 0) {
			if (bl-- > 1) {
			    if (si < bl) bp[si] = bp[bl] ;
			}
		        f = (bl == 0) ;
	                if (f) break ;
		    }
	            sp += 1 ;
	            sl -= 1 ;
	        } /* end while */
	        rs1 = dupstr_finish(&sd) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (dupstr) */
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (hasallof) */


