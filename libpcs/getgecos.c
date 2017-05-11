/* getgecos */

/* get the (fixed up) GECOS name out of the password file ('/etc/passwd') */


/* revision history:

	= 1994-01-12, David A­D­ Morano
	This subroutine was originally written.

	= 2008-09-03, David A­D­ Morano
        This subroutine should be removed. Hopefully it is not used in any
        current code!

*/

/* Copyright © 1994,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

	GECOSNAME gets the GECOS version of a user name from the
	fifth field of the system password file ('/etc/passwd').

*									
*	Arguments:

*	un	- username string (as already found in the pass word file)
*	gn	- a buffer supplied to hold the GECOS name string to be returned
	len	length of supplied buffer

*	RETURNED VALUE:							

		OK	- success (found a valid GECOS name)
		BAD	- failed (could not find a valid GECOS name)

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
#define	BUFLEN		200


/* external subroutines */


/* exported subroutines */


int getgecos(un,gn,len)
const char	un[] ;
char		gn[] ;
int		len ;
{
	struct passwd	pe, *pp ;

	int	rs ;

	const char	*cp, *cp2 ;

	char	buf[BUFLEN + 1] ;


#if	defined(SYSHAS_GETPWXXR) && SYSHAS_GETPWXXR
	if ((rs = - getpwnam_r(un,&pe,buf,BUFLEN,&pp)) < 0)
		pp = NULL ;
#else
	pp = (struct passwd *) getpwnam(un) ;
#endif

	if ((pp == NULL) || (pp->pw_gecos[0] == '\0')) 
	    return BAD ;

	if (((cp = strchr(pp->pw_gecos,'-')) != N) &&
	    ((cp2 = strchr(cp,CH_LPAREN)) != N)) {

	    cp += 1 ;
	    strwcpy(gn,cp,MIN(len,cp2 - cp)) ;

	} else if ((cp2 = strchr(pp->pw_gecos,CH_LPAREN)) != NULL) {

		strwcpy(gn,pp->pw_gecos,MIN(len,cp2 - pp->pw_gecos)) ;

	} else 
		strwcpy(gn,pp->pw_gecos,len) ;

	return OK ;
}
/* end subroutine (getgecos) */



