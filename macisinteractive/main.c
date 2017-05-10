/* main (isinteractive) */

/* whole program for ISINTERACTIVE */


/* revision history:

	= 1998-11-01, David Morano

	Originally written for RightCore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<exitcodes.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,char **argv,char **envv)
{
	int		ex = 0 ;
	int		f = FALSE ;

	f = (isatty(0) > 0) && (isatty(1) > 0) ;

	ex = (f) ? EX_OK : EX_TEMPFAIL ;

	return ex ;
}
/* end subroutine (main) */


