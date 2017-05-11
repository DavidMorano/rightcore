/* testlog */

/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#include	<math.h>
#include	<stdio.h>
#include	<errno.h>

int main()
{
	long double	x, y ;

	x = pow(2.0,127.0) + 1.0 ;

	y = log10(x) ;
	if (errno != ERANGE) {
	    printf("y=%Lf\n",y) ;
	} else {
	    printf("y=RANGE\n") ;
	}
	return 0 ;
}
/* end subroutine (main) */


