/* mainquick */
/* lang=C++98 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_PRINTA	1		/* define |printa()| */
#define	CF_NTH		0		/* nth element */
#define	CF_PAR1		0		/* partition */
#define	CF_PAR2		0		/* partition */
#define	CF_PARNTH	0		/* partition nth */
#define	CF_SUBQUICK	0		/* quick sort */
#define CF_SEL1		1		/* selection-1 */
#define CF_SEL2		1		/* selection-2 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-09-15, David A­D­ Morano
	Looking over again. Added standard |quickselecti()| in the form of the
	C++ STL |partial_sort()| function.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<list>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#if	CF_NTH
#if	(! CF_PARNTH)
#undef	CF_PARNTH
#define	CF_PARNTH	1
#endif
#endif

#define	VARDEBUGFNAME	"QUICK_DEBUGFILE"


/* name-spaces */

using namespace	std ;


/* type-defs */

typedef	vector<int>::iterator		vit_t ;
typedef	vector<int>::const_iterator	cvit_t ;


/* external subroutines */

typedef int	(*partpred_t)(int,int) ;

extern "C" int	quickselecti(int *,int,int,int) ;
extern "C" int	partitionai(int *,int,partpred_t,int) ;
extern "C" int	nthai(int *,int,int,int) ;

extern "C" void	arrswapi(int *,int,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */


/* forward references */

#if	CF_SUBQUICK
static int	partpred1(int,int) ;
static int	partpred2(int,int) ;
static int	getpivot(const int *,int) ;
#endif

#if	CF_PAR1
static int	subpar1(const vector<int> &,int,int) ;
#endif

#if	CF_PAR2
static int	subpar2(const vector<int> &,int,int) ;
#endif

#if	CF_PARNTH || CF_NTH
static int	subparnth(const vector<int> &,int,int) ;
#endif

#if	CF_SUBQUICK
static int	subquick() ;
#endif

#if	CF_PAR1 || CF_PAR2 || CF_NTH || CF_SEL2
static void	printv(const vector<int> &,int) ;
#endif

#if	CF_SEL1 || CF_SEL2 || CF_SUBQUICK
#if	CF_SEL1 || CF_SUBQUICK
static int	arr_load(int *,const int *,int) ;
#endif
#if	CF_SEL2
static int	vec_load(vector<int> &,const int *,int) ;
#endif
#endif

#if	CF_PRINTA
static void	printa(const int *,int) ;
#endif /* CF_PRINTA */

#if	CF_DEBUGS && CF_SUBQUICK
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

#if	CF_PAR1 || CF_PAR2
	if (rs >= 0) {
	    vector<int>	a = { 10,0,1,10,9,8,7,6,5,4,3,2,1,0 } ;
	    int		al ;

	    al = a.size() ;

	    for (int n = 0 ; n < al ; n += 1) {
	        cout << "al=" << al << " n=" << n << " val=" << a[n] << endl ;
	        printv(a,al) ;

#if	CF_PAR1
	        if (rs >= 0) {
	           rs = subpar1(a,al,n) ;
	        }
#endif /* CF_PAR1 */

#if	CF_PAR2
	        if (rs >= 0) {
	           rs = subpar2(a,al,n) ;
	        }
#endif /* CF_PAR2 */

	        cout << endl ;
	    } /* end for */

	} /* end if (ok) */
#endif /* CF_PAR1 || CF_PAR2 */

#if	CF_NTH
	if (rs >= 0) {
	    const vector<int> a = { 10,10,20,7,3,3,6,5,6,9,8,7,6,5,4,3,2,1,0 } ;
	    const int	algo[] = { 1, 2 } ;
	    const int	nn = 21 ;
	    int		al ;
	    al = a.size() ;
	    for (int n = 0 ; n < MIN(nn,al) ; n += 1) {
		cout << "n=" << n << endl ;
		printv(a,al) ;
		for (auto g : algo) {
		    switch (g) {
		    case 1:
			{
			    vector<int>	aa = a ;
	        	    vit_t	ib = aa.begin() ;
	        	    vit_t 	ie = aa.end() ;
	        	    nth_element<vit_t>(ib,(ib+n),ie) ;
	        	    printv(aa,al) ;
	        	    cout << "g=" << g << " nth=" << aa[n] << endl ;
			}
			break ;
		    case 2:
	   		rs = subparnth(a,al,n) ;
	        	cout << "g=" << g << " nth=" << rs << endl ;
			break ;
		    } /* end switch */
		} /* end for (algo) */
		    cout << endl ;
	    } /* end for (data) */
	}
#endif /* CF_NTH */

#if	CF_SUBQUICK
	if (rs >= 0) {
	    rs = subquick() ;
	}
#endif /* CF_SUBQUICK */

#if	CF_SEL1 || CF_SEL2
	if (rs >= 0) {
	    const int	a[] = { 10,9,8,7,6,5,4,3,2,1,0 } ;
	    const int	k = 4 ; /* kth element (by index) */
	    int		n ;
	    n = nelem(a) ;

	    printa(a,n) ;

#if	CF_SEL1
	if (rs >= 0) {
	    vit_t	it, end ;
	    vector<int>	va(n) ;
	    less<int>	lcmp ;
	    vec_load(va,a,n) ;
	    it = va.begin() ;
	    end = va.end() ;
	    partial_sort(it,it+k,end,lcmp) ;
	    cout << "qs> k=" << k << " kv=" << va[k] << endl ;
	    printv(va,n) ;
	}
#endif /* CF_SEL1 */

#if	CF_SEL2
	if (rs >= 0) {
	    int		*aa ;
	    if ((aa = new(nothrow) int[n+1]) != NULL) {
		arr_load(aa,a,n) ;
	        quickselecti(aa,0,n,k) ;
	        cout << "qs> k=" << k << " kv=" << aa[k] << endl ;
	        printa(aa,n) ;
		delete [] aa ;
	    } /* end if (m-a-f) */
	}
#endif /* CF_SEL2 */

	} /* end if (ok) */
#endif /* CF_SEL1 || CF_SEL2 */

	if (rs < 0) ex = 1 ;

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_PAR1
static int subpar1(const vector<int> &a,int al,int n)
{
	int		rs = SR_OK ;

	if (n < al) {
	vector<int>	aa = a ;
	const int	pv = a[n] ;

	auto pfunc = [pv] (int e) -> bool { 
	    bool f = ( e < pv ) ; 
	    /* cout << " " << e << ":" << f ; */
	    return f ;
        } ;

	{
	    vit_t	end = aa.end() ;
	    vit_t	it = aa.begin() ;
	    vit_t	it_res ;
	    it_res = partition(it,end,pfunc) ;
	    cout << endl ;
	    if (it_res != end) {
	        int d = distance(it,it_res) ;
	        cout << "pv=" << pv << " is=" << d << endl ;
	    } else {
	        int d = distance(it,it_res) ;
	        cout << "nval=" << "NA" << " is=" << d << endl ;
	    }
	}

	printv(aa,al) ;
	} else {
	    rs = SR_DOM ;
	}

	return rs ;
}
/* end subroutine (subpar1) */
#endif /* CF_PAR1 */


#if	CF_PAR2
static int partpred(int e,int p)
{
	return (e < p) ;
}

static int subpar2(const vector<int> &a,int al,int n)
{
	int		rs = SR_OK ;
	int		size ;

	if (n < al) {
	    const int	size = ((al+1)*sizeof(int)) ;
	    int		*aa ;
	if ((rs = uc_malloc(size,&aa)) >= 0) {
	    const int	pv = a[n] ;
	    int		is ;
	    int		i ;

	    for (i = 0 ; i < al ; i += 1) aa[i] = a[i] ;
	    is = partitionai(aa,al,partpred,pv) ;
	    if ((is >= 0) && (is < al)) {
	        cout << "pv=" << pv << " is=" << is << endl ;
	    } else {
	        cout << "*RANGE* is=" << is << endl ;
	    }
	    printa(aa,al) ;

	    uc_free(aa) ;
	} /* end if (m-a-f) */
	} else {
	    rs = SR_DOM ;
	}

	return rs ;
}
/* end subroutine (subpar2) */
#endif /* CF_PAR2 */


#if	CF_PARNTH || CF_NTH
static int subparnth(const vector<int> &a,int al,int n)
{
	int		rs ;
	int		size ;
	int		*aa ;

	size = ((al+1)*sizeof(int)) ;
	if ((rs = uc_malloc(size,&aa)) >= 0) {
	    int		i ;
	    for (i = 0 ; i < al ; i += 1) aa[i] = a[i] ;

	    rs = nthai(aa,0,al,n) ;
	    printa(aa,al) ;

	    uc_free(aa) ;
	} /* end if (m-a-f) */

	return rs ;
}
/* end subroutine (subparnth) */
#endif /* CF_PARNTH */


#if	CF_SUBQUICK

static int oursort(int lvl,int *a,int first,int last)
{
	int		ff = FALSE ;
#if	CF_DEBUGS
	debugprintf("oursort: ent lvl=%u f=%u l=%u\n",lvl,first,last) ;
#endif
	if ((last-first) == 2) {
	    if (a[first] > a[last-1]) arrswapi(a,first,(last-1)) ;
	    ff = TRUE ;
	} else if ((last-first) > 2) {
	    const int	pv = getpivot(a+first,(last-first)) ;
	    int		m1, m2 ;
	    ff = TRUE ;
#if	CF_DEBUGS
	    debugprinta(a,6) ;
#endif
	    m1 = partitionai(a+first,(last-first),partpred1,pv) + first ;
	    m2 = partitionai(a+m1,(last-m1),partpred2,pv) + m1 ;
#if	CF_DEBUGS
	    debugprintf("oursort: pv=%u\n",pv) ;
	    debugprinta(a,6) ;
	    debugprintf("oursort: m1=%u m2=%u\n",m1,m2) ;
#endif
	    if ((m1-first) > 1) oursort(lvl+1,a,first,m1) ;
	    if ((last-m2) > 1) oursort(lvl+1,a,m2,last) ;
	}
#if	CF_DEBUGS
	debugprintf("oursort: ret lvl=%u f=%u\n",lvl,ff) ;
#endif
	return ff ;
}
/* end subroutine (oursort) */

static int subquick() 
{
	const int	a[] = { 10,10,20,7,3,3,6,5,6,9,8,7,6,5,4,3,2,1,0 } ;
	const int	nn = 21 ;
	int		al = nelem(a) ;
	int		rs ;
	int		asize ;
	int		*aa ;
	asize = ((al+1)*sizeof(int)) ;
	if ((rs = uc_malloc(asize,&aa)) >= 0) {
	    for (int n = 1 ; n <= MIN(nn,al) ; n += 1) {
		cout << "n=" << n << endl ;
		printa(a,al) ;
		memset(aa,0,asize) ;
		arr_load(aa,a,n) ;
		debugprinta(a,al) ;
	        rs = oursort(0,aa,0,n) ;
		debugprinta(a,al) ;
	        printa(aa,al) ;
		cout << endl ;
		if (rs < 0) break ;
	    } /* end for (data) */
	    uc_free(aa) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (subquick) */

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

#endif /* CF_SUBQUICK */


#if	CF_SEL1 || CF_SEL2 || CF_SUBQUICK

#if	CF_SEL1 || CF_SUBQUICK

static int arr_load(int *aa,const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    aa[i] = a[i] ;
	}
	return i ;
}
#endif /* CF_SEL1 */

#if	CF_SEL2
static int vec_load(vector<int> &va,const int *a,int n)
{
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    va[i] = a[i] ;
	}
	return i ;
}
#endif /* CF_SEL2 */

#endif /* CF_SEL1 || CF_SEL2 */


#if	CF_PAR1 || CF_PAR2 || CF_NTH || CF_SEL2
static void printv(const vector<int> &a,int al)
{
	int		i ;
	for (i = 0 ; i < al ; i += 1) {
	    cout << " " << setw(2) << a[i] ;
	}
	cout << endl ;
}
/* end subroutine (printv) */
#endif


#if	CF_PRINTA
static void printa(const int *a,int n)
{
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    cout << " " << setw(2) << a[i] ;
	}
	cout << endl ;
}
/* end subroutine (printa) */
#endif /* CF_PRINTA */


#if	CF_DEBUGS && CF_SUBQUICK
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


