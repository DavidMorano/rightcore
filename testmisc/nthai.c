/* nthai */
/* lang=C++98 */

/* find Nth index-value function (for an array of integers) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Find nth element (by index) of an array of numbers.

	Synopsis:

	int nthai(int *a,int ri,int rl,int n)

	Arguments:

	a	array
	ri	low
	rl	high
	n	n-th smallest element

	Returns:

	-	


*******************************************************************************/


#include	<envstandards.h>
#include	<limits.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

typedef int	(*partpred_t)(int,int) ;

extern "C" int	nthai(int *,int,int,int) ;

extern "C" int	partitionai(int *,int,partpred_t,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct recurser {
	int		*a ;
	int		al ;
	int		n ;
	recurser(int *aa,int aal,int an) : a(aa), al(aal), n(an) { 
	} ;
	int	nrecurse(int,int) ;
} ;


/* forward references */

static int	partpred1(int,int) ;
static int	partpred2(int,int) ;
static int	getpivot(const int *,int) ;

#if	CF_DEBUGS
static int	debugprinta(const int *,int) ;
#endif


/* local variables */


int nthai(int *a,int ri,int rl,int n)
{
	recurser	r(a+ri,rl,(n-ri)) ;
	int		ans = INT_MAX ;
#if	CF_DEBUGS
	debugprintf("nthai: ent ri=%u rl=%u n=%u\n",ri,rl,n) ;
	debugprinta(a,rl) ;
#endif
	if ((n >= ri) && (n < (ri+rl))) {
	    ans = r.nrecurse(0,rl) ;
	} /* end if (needed) */
	return ans ;
}
/* end subroutine (nthai) */


/* local subroutines */


int recurser::nrecurse(int ri,int rl)
{
	int		ans = INT_MAX ;
#if	CF_DEBUGS
	debugprintf("nthai/nrecurse: ent ri=%u rl=%u \n",ri,rl) ;
	debugprinta(a,al) ;
#endif
	if ((n >= 0) && (n < (ri+rl))) {
	    int		pv ;
	    int		pi1, pi2 ;
	    int		new_ri, new_rl ;
	    pv = getpivot(a+ri,rl) ;
	    pi1 = partitionai(a+ri,rl,partpred1,pv) + ri ;
	    pi2 = partitionai(a+pi1,((ri+rl)-pi1),partpred2,pv) + pi1 ;
	    if ((n >= pi1) && (n < pi2)) {
	        ans = a[n] ;
	    } else if (n < pi1) {
		new_ri = ri ;
	  	new_rl = (pi1-ri) ;
	        ans = nrecurse(new_ri,new_rl) ;
	    } else {
		new_ri = pi2 ;
	  	new_rl = ((ri+rl)-pi2) ;
	        ans = nrecurse(new_ri,new_rl) ;
	    }
	} /* end if (valid) */
#if	CF_DEBUGS
	debugprintf("nthai/nrecurse: ret ans=%d\n",ans) ;
	debugprinta(a,al) ;
#endif
	return ans ;
}
/* end subroutine (recursoer::nrecurse) */


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
#endif /* CF_DEBUGS */


