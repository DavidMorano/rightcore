/* fixgecos */

/* fix the GECOS name as it comes out of the password file ('/etc/passwd') */


/* revision history:

	= 1984-01-12, David A­D­ Morano
        This code was rather completely rewritten from something that did the
        same function in the past. The old code did not properly handle the
        GECOS field format variations.

	= 2008-09-03, David A­D­ Morano
        This subroutine should be completely trashed! It is garbage! Not only is
        this function better handled in other subroutines, this subroutine does
        not even work correctly on current data. Hopefully, this subroutine sits
        here but is not actually used in any current code. Like I said
        "hopefully."

*/

/* Copyright © 1984,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

	GECOSNAME gets the GECOS version of a user name from the
	fifth field of the system password file ('/etc/passwd').

*									
*	PARAMETERS:							
*	un	- username string (as already found in the pass-word file)
*	gn	- GECOS name string to be returned

*									
*	RETURNED VALUE:							
		0	- success (found a valid GECOS name)
		-1	- failed (could not find a valid GECOS name)

*									
*	SUBROUTINES CALLED:						
*		Only system routines are called.
*									
*	GLOBAL VARIABLES USED:						
*		None!!  AS IT SHOULD BE!! (new comment by David A.D. Morano)
*									
*									
*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#ifndef	N
#define	N	NULL
#endif


/* exported subroutines */


int fixgecos(gn,fn,len)
const char	gn[] ;
char		fn[] ;
int		len ;
{
	const char	*cp, *cp2 ;

	if (((cp = strchr(gn,'-')) != N) &&
	    ((cp2 = strchr(cp,CH_LPAREN)) != N)) {

	    cp += 1 ;
	    strncpy(fn,cp,MIN(cp2 - cp,len)) ;

	    fn[cp2 - cp] = '\0' ;

	} else 
	    strncpy(fn,gn,len) ;

	return OK ;
}
/* end subroutine (fixgecos) */


