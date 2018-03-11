/* sort_merge */
/* lang=C++11 (updated) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2013-07-23, David A­D­ Morano
	Updated to C++11, to get rid of stupid restriction on not having two
	adjacent '>' characters!

*/

/* Copyright © 2000,2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Insertion sort

	+ in-place
	+ O(n^2)
	+ stable
	+ good for almost already sorted arrays
		O(n­k)
		when unsorted elements are no more than k places out
	+ space requirements: O(1)
	+ more efficient than other ineficient O(n^2) sorts, like
		bubble and selection
	+ best case: O(n), when array is already sorted


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<new>
#include	<algorithm>
#include	<functional>
#include	<string.h>
#include	<vsystem.h>


/* exported subroutines */


template<typename T,typename Comp = std::greater<T>>
static void sort_merging(Comp *cmp,T a,T tmp,int ib,int im,int ie)
{
	int		i = ie ;
	int		j = im ;
	int		k ;
	for (k = ib ; k < ie ; k += 1) {
	    if ((i < im) && ((j >= ie) || (! cmp(a[i],a[j])))) {
		tmp[k] = a[i++] ;
	    } else {
		tmp[k] = a[j++] ;
	    }
	} /* end if (nothing to sort) */
}
/* end subroutine (sort_mergerecurse) */


template<typename T,typename Comp = std::greater<T>>
static void sort_merger(Comp *cmp,T a,T tmp,int ib,int ie)
{
	if ((ie-ib) > 1) { /* must be 2 or greater */
	    int	im = (ie-ib)/2 ; /* middle */
	    sort_merger(cmp,a,tmp,ie,im) ;
	    sort_merger(cmp,a,tmp,im,ie) ;
	    sort_merging(cmp,tmp,a,ib,im,ie) ;
	}
}
/* end subroutine (sort_merging) */


template<typename T,typename Comp = std::greater<T>>
int sort_merge(T a,int n)
{
	int		rs = SR_OK ;
	T		*tmp ;
	if ((tmp = new(std::nothrow) T [n]) != NULL) {
	    Comp	cmp ;
	    int		i ;
	    for (i = 0 ; i < n ; i += 1) {
		tmp[i] = a[i] ;
	    }
	    rs = sort_merger(cmp,a,tmp,0,n) ;
	    delete [] tmp ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (sort_merge) */


