/* main (testfibo) */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program calculates and prints out the first few Finonacci numbers.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>


/* local defines */


#define	NN	20


/* external subroutines */

extern longlong_t	fibonacci(unsigned int) ;


/* exported subroutines */


int main()
{
	unsigned int	i ;

	for (i = 0 ; i < NN ; i += 1) {
		fprintf(stdout," %u", fibonacci(i)) ;
	}

	fprintf(stdout,"\n") ;

	return 0 ;
}
/* end subroutine (main) */


