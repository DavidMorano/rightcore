/* main (isinteractive) */

/* whole program for ISINTERACTIVE */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<exitcodes.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	ex = 0 ;
	int	f = TRUE ;


	f = f && (isatty(0) > 0) ;

	f = f && (isatty(1) > 0) ;

	ex = (f) ? EX_OK : EX_TEMPFAIL ;

	return ex ;
}
/* end subroutine (main) */



