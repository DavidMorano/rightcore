/* main (two) */
/* lang=C99 */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	What were we doing here?  A custom version of SQRT?


*******************************************************************************/


#include	<envstandards.h>
#include	<stdio.h>
#include	<math.h>
#include	<localmisc.h>


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	double		a = 2.0 ;
	double		b ;

	b = sqrt(a) ;
	printf("sqrt(2)=%2.40f\n",b) ;

	return 0 ;
}
/* end subroutine (main) */


