/* ns_fixgecos */

/* fix the GECOS name as it comes out of the password file ('/etc/passwd') */


/* revision history:

	= 1994-01-12, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

        GECOSNAME gets the GECOS version of a user name from the fifth field of
        the system password file ('/etc/passwd').

*	PARAMETERS:							

*	gn	- GECOS name string to be returned
	fn	- buffer to receice the fixed name
	len	- length of the buffer to receive the fixed name

*	RETURNED VALUE:							

		0	- success (found a valid GECOS name)
		-1	- failed (could not find a valid GECOS name)

*	SUBROUTINES CALLED:						

*		Only system routines are called.
*									
*	GLOBAL VARIABLES USED:						

*		None!!  AS IT SHOULD BE!! (comment added by David A.D. Morano)
*									
*									
*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<pwd.h>

#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#define	N		NULL

#ifndef	CH_LPAREN
#define	CH_LPAREN	0x28
#endif


/* exported subroutines */


int ns_fixgecos(gn,fn,len)
const char	gn[] ;
char		fn[] ;
int		len ;
{
	const char	*cp, *cp2 ;


	if (((cp = strchr(gn,'-')) != N) &&
	    ((cp2 = strchr(cp,CH_LPAREN)) != N)) {

	    cp += 1 ;
	    strwcpy(fn,cp,MIN(cp2 - cp,len)) ;

	} else if ((cp2 = strchr(gn,CH_LPAREN)) != NULL) {

		strwcpy(fn,gn,MIN(len,cp2 - gn)) ;

	} else 
		strwcpy(fn,gn,len) ;

	return OK ;
}
/* end subroutine */


