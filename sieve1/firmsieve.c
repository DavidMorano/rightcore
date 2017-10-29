/* Sieve benchamrk */

/* Sieve of Eratosthenes */


/* revision history:

	= David A­D­ Morano, 1986-05-01
	Idea by B­J­ Hudson.

*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine implements the Sieve of Eratosthenes algorithm.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	"csc.h"

#include	"julian.h"


/* local defines */

#define TRUE 1
#define FALSE 0
#define SIZE 8190
#define REG	register


char	sieve_flags[SIZE+1];

int sieve()
{
	struct julian	jul ;

	long	start[2], end[2], elapsed[2] ;

	REG int		i, prime, k, count, iter;


	printf("100 iterations\n") ;

	sc_daytime(start) ;

/* we start */

	for (iter = 0 ; iter < 100 ; iter += 1) {

	    count = 0;
	    for (i = 0 ; i <= SIZE ; i += 1)
	        sieve_flags[i] = TRUE ;

	    for (i = 0 ; i <= SIZE ; i += 1) {

	        if (sieve_flags[i]) {

	            prime = i + i + 3 ;
	            for (k = (i + prime) ; k <= SIZE; k += prime)
	                sieve_flags[k] = FALSE ;

	            count += 1 ;

	        } /* end if */

	    } /* end for */

	} /* end for */

/* we're done */

	sc_daytime(end) ;

	subd3(start,end,elapsed) ;

	jtime(elapsed,&jul) ;

	printf("elapsed time %02u:%02u.%03u\n",jul.min,jul.sec,jul.thou) ;

	printf("%d primes\n", count);

	return 0 ;
}
/* end subroutine (sieve) */


