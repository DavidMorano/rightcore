/* sort_insertion */
/* lang=C++11 (updated) */


#define	CF_NORECURSE		0	/* do not use recursion */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2013-07-23, David A­D­ Morano
	Updated to C++11, to get rid of stupid restriction on not having two
	adjacent '>' characters!

*/

/* Copyright © 2000,2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Inserion sort

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
#include	<algorithm>
#include	<functional>


/* exported subroutines */


#if	CF_NORECURSE
template<typename T,typename Comp = std::greater<T>>
int sort_interstion(T a,int n)
{
	Comp	cmp ;
	int	i = 1 ;
	int	j ;
	T	x ;	/* target selection */
	while( i < n) {
	    x = a[i] ;
	    j = (i-1) ;
    	    while ((j >= 0) && cmp(a[j],x)) {
		a[j+1] = a[j] ;
		j -= 1 ;
	    }
	    a[j+1] = t ;
	    i += 1 ;
	}
	return n ;
}
/* end subroutine (sort_insertion) */
#else /* CF_NORECURSE */
template<typename T,typename Comp = std::greater<T>>
int sort_interstionrecurse(T a,int n,Comp &cmp)
{
	if (n > 0) {
	    T		t ;	/* target selection */
	    int		j ;
	    sort_insertion(a,n-1) ;
	    x = a[n] ;
	    j = (n-1) ;
    	    while ((j >= 0) && cmp(a[j],x)) {
		a[j+1] = a[j] ;
		j -= 1 ;
	    }
	    a[j+1] = t ;
	}
	return n ;
}
/* end subroutine (sort_insertionrecurse) */
template<typename T,typename Comp = std::greater<T>>
int sort_insertione(T a,int n)
{
	Comp	cmp ;
	return sort_insertionrecurse(a,(n-1),cmp) ;
}
#endif /* CF_NORECURSE */


