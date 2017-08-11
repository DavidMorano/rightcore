/* mainquick */
/* lang=C++11 */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<new>
#include	<algorithm>
#include	<functional>
#include	<vector>
#include	<list>
#include	<array>
#include	<iostream>
#include	<iomanip>
#include	<localmisc.h>


using namespace	std ;

typedef	vector<int>::iterator	 ouriter ;


/* external subroutines */

extern int	kthSmallest(int *,int,int,int) ;

extern "C" void	quickselect(int *,int,int,int) ;


/* forward references */

static void printv(vector<int> &,int) ;
static void printa(int *,int) ;


/* exported subroutines */


int main()
{
	vector<int>	a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } ;
	ouriter		ib, ie ;
	int		n ;
	int		k  ;
	int		i ;

	n = a.size() ;

	printv(a,n) ;

	ib = a.begin() ;
	ie = a.end() ;
	for (k = 0 ; k < n ; k += 1) {
	    printf("k=%u\n",k) ;
	    nth_element<ouriter>(ib,(ib+k),ie) ;
	    printv(a,n) ;
	}

	k = 4 ;
	{
	    int	a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } ;
	    int	ans ;
	    ans = kthSmallest(a,0,n-1,k) ;
	    cout << "-> k=" << k << " ans=" << ans << endl ;
	    printa(a,n) ;
	    quickselect(a,0,n-1,k) ;
	    cout << "qs> k=" << k << endl ;
	    printa(a,n) ;
	}

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static void printv(vector<int> &a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
		printf(" %u",a[i]) ;
	}
	printf("\n") ;
}
/* end subroutine (printv) */

static void printa(int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
		printf(" %u",a[i]) ;
	}
	printf("\n") ;
}
/* end subroutine (printa) */


