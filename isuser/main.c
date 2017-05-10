/* main */

/* whole program for ISUSER */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services (RNS).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program checks if a specified user (given as an argument string)
	is actually present on the current system.

	Synopsis:

	$ isuser <user>


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<exitcodes.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	int		ex = 0 ;

	if (argc >= 1) {
	    struct passwd	*pp ;
	    const char	*un = argv[1] ;
	    if ((un != NULL) && (un[0] != '\0')) {
	        const int	f = ((pp = getpwnam(un)) != NULL) ;
	        ex = (f) ? EX_OK : EX_NOUSER ;
	    } else
	        ex = EX_NOUSER ;
	} else
	    ex = EX_USAGE ;

	return ex ;
}
/* end subroutine (main) */


