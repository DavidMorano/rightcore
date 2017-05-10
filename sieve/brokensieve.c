/* sieve */

/* Sieve calculation */


#define	CF_DEBUGS	1


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

#ifdef	COMMENT
	BACLR(ba,0) ;

	BACLR(ba,1) ;
#endif

	count = 0 ;
	for (i = 0 ; i <= n ; i += 1) {

#if	CF_DEBUGS
	debugprintf("sieve: i=%u \n",i) ;
#endif

	    if (BATST(ba,i)) {

#if	CF_DEBUGS
	debugprintf("sieve: i=%u is set\n",i) ;
#endif

	        prime = i + i + 3 ;

#if	CF_DEBUGS
	debugprintf("sieve: prime %d is %s\n",
		prime,((isprime(prime)) ? "prime" : "not")) ;
#endif

	        for (k = (i + prime) ; k <= n ; k += prime) {

#if	CF_DEBUGS
	debugprintf("sieve: k=%u clearing\n",k) ;
#endif

	            BACLR(ba,k) ;

		}

	        count += 1 ;

	    } /* end if */

	} /* end for */

	BASET(ba,2) ;

	BASET(ba,3) ;

	return count ;
}
/* end subroutine (sieve) */




