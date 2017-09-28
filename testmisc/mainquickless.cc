/* mainquickless */
/* lang=C++11 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-09-18, David A­D­ Morano
	Updated to reload array on each loop.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	What is this?  Do not quite know.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<new>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<list>
#include	<localmisc.h>


/* name spaces */

using namespace	std ;

typedef	vector<int>::iterator	 ouriter ;


/* C++ */
extern "C" void	quickselecti(int *,int,int,int) ;


/* forward */

static void	arrload(int *,const int *,int) ;
static void	printa(const int *,int) ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	a[] = { 2, 3, 4, 1, 27, 1, 9, 13, 17 } ;
	int		n ;
	int		k  ;

	n = nelem(a) ;
	printa(a,n) ;

	for (k = 1 ; k < (n-1) ; k += 1) {
	    int		aa[n+1] ;
	    arrload(aa,a,n) ;
	    printf("k=%u\n",k) ;
	    quickselecti(aa,0,n,k) ;
	    printa(aa,n) ;
	} /* end for */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static void arrload(int *aa,const int *a,int al)
{
	for (int i = 0 ; i < al ; i += 1) {
	     aa[i] = a[i] ;
	}
}


static void printa(const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    printf(" %u",a[i]) ;
	}
	printf("\n") ;
}
/* end subroutine (printa) */


