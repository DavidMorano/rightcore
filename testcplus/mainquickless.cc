/* mainquickless */
/* lang=C++11 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<list>
#include	<array>
#include	<localmisc.h>


/* name spaces */

using namespace	std ;

typedef	vector<int>::iterator	 ouriter ;


/* C++ */
extern "C" void	quickselect(int *,int,int,int) ;

extern int kthSmallest(int *,int,int,int) ;


/* forward */

static void printa(const int *,int) ;


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	int		a[] = { 2, 3, 4, 1, 27, 9, 13, 17 } ;
	int		n = nelem(a) ;
	int		k  ;

	printa(a,n) ;

	for (k = 1 ; k < (n-1) ; k += 1) {
	    printf("k=%u\n",k) ;
	    quickselect(a,0,n,k) ;
	    printa(a,n) ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static void printa(const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    printf(" %u",a[i]) ;
	}
	printf("\n") ;
}
/* end subroutine (printa) */


