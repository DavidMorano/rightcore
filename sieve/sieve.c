/* sieve */

/* Sieve calculation */


#define	CF_DEBUGS	0


/* revision history:

	86/05/01, David A­D­ Morano

	This is an adaptation of the classic Sieve of Eratosthenes
	algorithm.


*/


/******************************************************************************

	This subroutine implements the Sieve of Eratosthenes
	algorithm.


	Synopsis:

	int sieve(ba,n)
	uchar	ba[] ;
	uint	n ;


	Arguments:

	ba	user supplied array to hold the sieve flags
	n	the maximum to which primes are desired


	Returns:

	count   count of the primes found less than or equal to the
		maximum



******************************************************************************/


#include	<sys/types.h>
#include	<string.h>

#include	<baops.h>

#include	"localmisc.h"



/* local defines */



/* external subroutines */

extern int	isprime(uint) ;







int sieve(ba,n)
uchar	ba[] ;
uint	n ;
{
	uint	i, k ;

	int	size = (n >> 3) + 2 ;
	int	prime, count ;


	memset(ba,0xFF,size) ;

	BACLR(ba,0) ;

	BACLR(ba,1) ;

	for (i = 4 ; i <= n ; i += 2)
		BACLR(ba,i) ;

	count = 0 ;
	for (i = 3 ; i <= n ; i += 2) {

#if	CF_DEBUGS
	debugprintf("sieve: i=%u \n",i) ;
#endif

	    if (BATST(ba,i)) {

#if	CF_DEBUGS
	debugprintf("sieve: i=%u is set\n",i) ;
#endif

	        for (k = (3 * i) ; k <= n ; k += (i << 1)) {

#if	CF_DEBUGS
	debugprintf("sieve: k=%u clearing\n",k) ;
#endif

	            BACLR(ba,k) ;

		}

	        count += 1 ;

	    } /* end if */

	} /* end for */

	return count ;
}
/* end subroutine (sieve) */




