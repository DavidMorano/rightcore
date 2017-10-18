/* mainsorts */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test some sorting algorithms.


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

#define	VARDEBUGFNAME	"QUICK_DEBUGFILE"


/* name-spaces */

using namespace	std ;


/* type-defs */

typedef	vector<int>::iterator		vit ;
typedef	vector<int>::const_iterator	cvit ;
typedef int	(*sortcmp_t)(const void *,const void *) ;


/* external subroutines */

extern "C" int	isort(void *,int,int,sortcmp_t) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */


/* forward references */

static int	ourcmp(const void *,const void *) ;

static void	arrload(int *,const vector<int> &,int) ;
static void	printa(const int *,int) ;

#if	CF_DEBUGS
static int	debugprinta(const int *,int) ;
#endif


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argvv,cchar **envv)
{
	int		rs = SR_OK ;
	int		ex = 0 ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    const vector<int> a = { 20,10,10,7,3,3,6,5,6,9,8,7,6,5,4,3,2,1,0 } ;
	    const int	algo[] = { 1, 2 } ;
	    const int	nn = 20 ;
	    const int	esize = sizeof(int) ;
	    int		al ;
	    int		asize ;
	    int		*aa ;
	    al = a.size() ;
	    asize = ((al+1)*sizeof(int)) ;
	    if ((rs = uc_malloc(asize,&aa)) >= 0) {
	        int	n ;
	        for (n = 2 ; n <= MIN(nn,al) ; n += 1) {
	      	    arrload(aa,a,al) ;
	     	    printa(aa,al) ;
#if	CF_DEBUGS
		    debugprinta(aa,al) ;
#endif
		    memset(aa,0,asize) ;
		    for (auto g : algo) {
	        	arrload(aa,a,n) ;
		        cout << "n=" << n << " g=" << g << endl ;
		        switch (g) {
		            case 1:
	        	        qsort(aa,n,esize,ourcmp) ;
			    break ;
		            case 2:
	        	        isort(aa,n,esize,ourcmp) ;
			    break ;
		        } /* end switch */
	        	printa(aa,al) ;
		    } /* end for (algo) */ 
		cout << endl ;
		} /* end for (n) */
	    } /* end for (data) */
	} /* end if (ok) */

	if (rs < 0) ex = 1 ;

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static void arrload(int *aa,const vector<int> &a,int al)
{
	for (int i = 0 ; i < al ; i += 1) {
	     aa[i] = a[i] ;
	}
}


static int ourcmp(const void *v1p,const void *v2p)
{
	int		*i1p = (int *) v1p ;
	int		*i2p = (int *) v2p ;
	return (*i1p - *i2p) ;
}


static void printa(const int *a,int n)
{
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    cout << " " << setw(2) << a[i] ;
	}
	cout << endl ;
}
/* end subroutine (printa) */


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


