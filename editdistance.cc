/* editdistance */
/* lang=C++11 */

/* find the edit-distance between two given strings */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We find (calculate) the edit-distance to convert one string ('a')
	into abother ('b').

	This is the standard formula for getting the edit distance from one
	string to another, using: change, insert, and delete operations.

	a -> b


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<new>
#include	<utility>
#include	<vsystem.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* local structures */

template<typename T>
void ourswap(T &a,T &b) 
{
	T tmp = a ;
	a = b ;
	b = tmp ;
}
/* end subroutine (ourswap) */


/* forward references */

static int	miner(int,int,int) ;


/* local variables */


/* exported subroutines */


int editdistance(cchar *a,cchar *b)
{
	int		al = strlen(a) ;
	int		bl = strlen(b) ;
	int		rs = SR_OK ;
	int		n ;
	int		el = 0 ;
	int		*ibuf ;

	if (al < bl) { /* swap inputs */
	    ourswap(a,b) ;
	    ourswap(al,bl) ;
	}

	n = ((bl+1)*2) ;
	if ((ibuf = new(nothrow) int [n]) != NULL) {
	    int		*previous = (ibuf+(0*(bl+1))) ;
	    int		*current = (ibuf+(1*(bl+1))) ;
	    int		i, j ;
	    memset(ibuf,0,(n*sizeof(int))) ;

	    for (i = 1; i <= al ; i += 1) {
	        current[0] = i ;

		for (j = 1 ; j <= bl ; j += 1) {
		    if (a[i-1] == b[j-1]) {
			current[j] = previous[j-1] ;
		    } else {
			const int	a = previous[j-1] ;
			const int	b = previous[j] ;
			const int	c = current[j-1] ;
		 	{
			    const int	m = miner(a,b,c) ;
		            current[j] = 1 + m ;
			}
		    }
		} /* end for */

		for (j = 0 ; j <= bl ; j += 1) {
		    previous[j] = current[j] ;
		    current[j] = 0 ;
		}

	   }  /* end for */
	   el = previous[bl] ;
	   delete [] ibuf ;
	} else {
	    rs = SR_NOMEM ;
	} /* end if */
	return (rs >= 0) ? el : rs ;
}
/* end subroutine (editdistance) */


/* local subroutines */


static int miner(int a,int b,int c)
{
	return MIN(MIN(a,b),c) ;
}
/* end subroutine (miner) */


