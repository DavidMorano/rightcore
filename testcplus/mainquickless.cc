/* mainquickless */
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
#include	<localmisc.h>


using namespace	std ;

typedef	vector<int>::iterator	 ouriter ;


/* C++ */
extern "C" void	quickselect(int *,int,int,int) ;

extern int kthSmallest(int *,int,int,int) ;


/* forward */

static void printa(const int *,int) ;


int main()
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


