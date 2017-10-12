/* quickselecti */
/* lang=C++98 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written elsewhere as part of some subroutine code modeule.

	= 2018-09-15, David A­D­ Morano
	Took from another file and made it a stand-alone (exported) subroutine.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform a "quick-select" function on an array of integers.

	Synopsis:

	int quickselecti(int *a,int first,int last,int k)

	Arguments:

	a		array of integers to sort
	first		index of first element in range to sort
	last		index of over-last element of range to sort
	k		index of pivot element

	Returns:

	-		inconsequential at this time


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<localmisc.h>


/* local defines */


/* name-spaces */


/* type-defs */


/* external subroutines */

typedef int	(*partpred_t)(int,int) ;

extern void	arrswapi(int *,int,int) ;
extern int	partitionai(int *,int,partpred_t,int) ;
extern int	nthai(int *,int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,cchar *,int) ;
#endif


/* local structures */


/* forward references */

static int	getpivot(const int *,int) ;
static int	partpred1(int,int) ;
static int	partpred2(int,int) ;

#if	CF_DEBUGS
static int	debugprinta(const int *,int) ;
#endif


/* exported subroutines */


int quickselecti(int *a,int first,int last,int k)
{
	int		kv = a[k] ;
	if ((k >= first) && (k < last)) {
	    if ((last-first) == 2) {
	        if (a[first] > a[last-1]) arrswapi(a,first,(last-1)) ;
	        kv = a[k] ;
	    } else if ((last-first) > 2) {
	        const int	pv = getpivot(a+first,(last-first)) ;
	        int		m1, m2 ;
	        m1 = partitionai(a+first,(last-first),partpred1,pv) + first ;
	        m2 = partitionai(a+m1,(last-m1),partpred2,pv) + m1 ;
	        if (k < m1) {
		    kv = quickselecti(a,first,m1,k) ;
	        } else if (k >= m2) {
		    kv = quickselecti(a,m2,last,k) ;
	        } else {
	    	    kv = a[k] ;
		}
	    }
	} /* end if (in range) */
	return kv ;
}
/* end subroutine (quickselecti) */


/* local subroutines */


static int getpivot(const int *a,int al)
{
	int	pvi = (al/2) ;
	if (pvi == 0) {
	    if (al > 1) pvi = 1 ;
	}
	return a[pvi] ;
}

static int partpred1(int e,int pv)
{
	return (e < pv) ;
}

static int partpred2(int e,int pv)
{
	return (e <= pv) ;
}


#if	CF_DEBUGS
static int debugprinta(const int *a,int al)
{
	int		i ;
	for (i = 0 ; i < al ; i += 1) {
	    debugprintf(" %2u\\",a[i]) ;
	}
	debugprintf("\n") ;
	return 0 ;
}
/* end subroutine (debugprinta) */
#endif /* CF_DEBUGS */


