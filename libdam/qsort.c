/* qsort */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Our version of Quick Sort (sort of the daddy of all sorts, mostly).

	Everyone has their own, right?

	Synopsis:
	typedef int	(*qsortcmp_t)(const void *,const void *) ;
	void qsort(void *base,int nelem,int esize,qsortcmp_t *cmp)

	Arguments:
	base		pointer to base of array to sort
	nelem		number of elements in array
	esize		size in bytes of an array element
	cmp		comparison function

	Returns:
	-		nothing (sorted array in place)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<list>
#include	<array>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* name-spaces */

using namespace	std ;


/* type-defs */


/* external subroutines */

typedef int	(*partpred_t)(int,int) ;

extern "C" void	arrswap(int *,int,int) ;
extern "C" int	partitionai(int *,int,partpred_t,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */


/* forward references */

static int	partpred1(int,int) ;
static int	partpred2(int,int) ;
static int	getpivot(const int *,int) ;


/* exported subroutines */


void qsort(void *base,int nelem,int esize,qsortcmp_t *cmp)
{


}
/* end subroutine (qsort) */


/* local subroutines */


void oursort(int *a,int first,int last)
{
	int		pvi = (last-first)/2 ;
	int		ff = FALSE ;
#if	CF_DEBUGS
	debugprintf("oursort: ent f=%u l=%u\n",first,last) ;
#endif
	if ((last-first) == 2) {
	    if (a[first] > a[last-1]) arrswap(a,first,(last-1)) ;
	    ff = TRUE ;
	} else if ((last-first) > 2) {
	    int	pv = a[pvi] ;
	    int	m1, m2 ;
	    ff = TRUE ;
#if	CF_DEBUGS
	    debugprintf("oursort: pv=%u\n",pv) ;
#endif
	    debugprinta(a,6) ;
	    pv = getpivot(a+first,(last-first)) ;
	    m1 = partitionai(a+first,(last-first),partpred1,pv) + first ;
	    debugprinta(a,6) ;
	    m2 = partitionai(a+m1,(last-m1),partpred2,pv) + m1 ;
#if	CF_DEBUGS
	    debugprintf("oursort: m1=%u m2=%u\n",m1,m2) ;
#endif
	    if ((m1-first) > 1) oursort(a,first,m1) ;
	    if ((last-m2) > 1) oursort(a,m2,last) ;
	}
#if	CF_DEBUGS
	debugprintf("oursort: ret f=%u\n",ff) ;
#endif
	return ff ;
}
/* end subroutine (oursort) */


static int partpred1(int e,int pv)
{
	return (e < pv) ;
}


static int partpred2(int e,int pv)
{
	return (e <= pv) ;
}


static int getpivot(const int *a,int al)
{
	int	pvi = (al/2) ;
	if (pvi == 0) {
	    if (al > 1) pvi = 1 ;
	}
	return a[pvi] ;
}


