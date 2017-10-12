/* testpartitionai */
/* lang=C++11 */

/* test |partitionai()| function */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGPRINTA	1		/* print arrays */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

	= 2017-09-15, David A­D­ Morano
	Updated to use (or potentionally use) C++11 features.

*/

/* Copyright © 2001,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test the |partitionai()| function described here below.

	Partition an array of integers.

	Synopsis:

	int partitionai(int *a,int al,partpred_t partpred,int)

	Arguments:

	a		array
	al		array length
	partpred	function to evaluate the predicate
	int		value to pass to the predicate function

	Returns:

	-	index of pivot (based from 'ri')

	Notes:

        I knew this was correct from the start, but we had to get another
        oponion.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
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

#define		VARDEBUGFNAME	"TESTPART_DEBUGFILE"


/* name spaces */

using namespace	std ;


/* typedefs */

typedef int	(*partpred_t)(int,int) ;


/* external subroutines */

extern "C" int	partitionai(int *,int,partpred_t,int) ;
extern "C" int	partidxi(int *,int,int,int) ;

extern "C" void	arrswapi(int *,int,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

static int	ourpred(int,int) ;
static int	partest(int *,int,partpred_t,int) ;
static int	isPart(const int *,int,partpred_t,int) ;

static void	loadrand(int *,int) ;
static void	loadagain(int *,int *,int) ;
static void	loadfromvec(int *,const vector<int> &,int) ;
static void	vecload(vector<int> &,const int *,int) ;

#if	CF_DEBUGS && CF_DEBUGPRINTA
static int	debugprinta(const int *,int) ;
#endif


/* local variables */


/* export subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	al = 15 ;
	int		rs ;
	int		ex = 0 ;
	int		asize ;
	int		*a ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

	asize = (2*(al+1)*sizeof(int)) ;
	if ((rs = uc_malloc(asize,&a)) >= 0) {
	    const int	algo[] = { 0, 1, 2, 3 } ;
	    const int	on = 100 ;
	    int		i ;
	    int		*aa = (a+(al+1)) ;
	    for (i = 0 ; i < on ; i += 1) {
		const int	gn = nelem(algo) ;
		int		j ;
	        loadrand(a,al) ;
		for (j = 0 ; j < al ; j += 1) {
		    const int	pv = a[j] ;
		    int		gi ;
		    int		ans[gn+1] ;
#if	CF_DEBUGS
			debugprintf("main: new i=%u j=%u pv=%u\n",i,j,pv) ;
		        loadagain(aa,a,al) ;
			debugprinta(aa,al) ;
#endif
		    for (gi = 0 ; gi < gn ; gi += 1) {
		        const int	g = algo[gi] ;
		        loadagain(aa,a,al) ;
#if	CF_DEBUGS
			debugprintf("main: i=%u j=%u pv=%u g=%u\n",i,j,pv,g) ;
#endif
		        switch (g) {
			case 0:
			    {
				vector<int>	v ;
				vecload(v,aa,al) ;
			        {
				    vector<int>::iterator end = v.end() ;
				    vector<int>::iterator it = v.begin() ;
				    vector<int>::iterator res ;
				    auto fn = [pv] (int e) -> bool { 
					return (e < pv) ;
				    } ;
				    res = partition(it,end,fn) ;
				    if (res != end) {
					ans[gi] = distance(it,res) ;
				    } else {
				        ans[gi] = al ;
				    }
				    loadfromvec(aa,v,al) ;
				}
			    } /* end block */
			    break ;
		        case 1:
			    ans[gi] = partest(aa,al,ourpred,pv) ;
			    break ;
		        case 2:
			    ans[gi] = partitionai(aa,al,ourpred,pv) ;
			    break ;
		        case 3:
			    ans[gi] = partidxi(aa,0,al,j) ;
			    break ;
			} /* end switch */
#if	CF_DEBUGS
			debugprintf("main: ans=%u\n",ans[gi]) ;
			debugprinta(aa,al) ;
#endif
			if (! isPart(aa,al,ourpred,pv)) rs = SR_PROTO ;
			if (rs >= 0) rs = ans[gi] ; /* compiler complaint */
			if (rs < 0) break ;
		    } /* end for (algos) */
#ifdef	COMMENT
		    if (rs >= 0) {
			for (g = 1 ; g < gn ; g += 1) {
			    if (ans[g] != ans[0]) rs = SR_NOMSG ;
			    if (rs < 0) break ;
			}
		    }
#endif /* COMMENT */
#if	CF_DEBUGS
			debugprintf("main: \n") ;
#endif
		    if (rs < 0) break ;
		} /* end for (pivot indices) */
		if (rs < 0) break ;
	    } /* end for (number of times) */
	    uc_free(a) ;
	} /* end if (m-a-f) */

	if (rs < 0) printf("bad\n") ;
	if (rs < 0) ex = 1 ;

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int ourpred(int e,int pv)
{
	return (e < pv) ;
}

static int partest(int *a,int al,partpred_t fn,int pv)
{
	int		i ;
	int		pi = 0 ;
	for (i = 0 ; i < al ; i += 1) {
	    int	f = (*fn)(a[i],pv) ;
	    if (f) {
		if (i != pi) arrswapi(a,i,pi) ;
		pi += 1 ;
	    }
	}
	return pi ;
}
/* end subroutine (partest) */

static int isPart(const int *a,int al,partpred_t fn,int pv)
{
	int		f = TRUE ;
	if (al > 1) {
	    int		i ;
	    int		is = al ;
	    for (i = 0 ; i < al ; i += 1) {
	        const int f_pred = (fn)(a[i],pv) ;
		if (i < is) {
		    if (is != al) {
		        f = f_pred ;
		    } else {
			if (! f_pred) is = i ;
		    }
		} else {
		    f = (! f_pred) ;
		}
	        if (!f) break ;
	    } /* end for */
	} /* end if */
	return f ;
}
/* end subroutine (isPart) */

static void loadrand(int *aa,int al)
{
	int		i ;
	for (i = 0 ; i < al ; i += 1) {
	    aa[i] = rand() % 100 ;
	}
}

static void loadagain(int *aa,int *a,int al)
{
	const int	size = (al*sizeof(int)) ;
	memcpy(aa,a,size) ;
}

static void loadfromvec(int *aa,const vector<int> &v,int al) 
{
	for (int i = 0 ; i < al ; i += 1) {
	    aa[i] = v[i] ;
	} 
}

static void vecload(vector<int> &v,const int *aa,int al) 
{
	for (int i = 0 ; i < al ; i += 1) {
	    v.push_back(aa[i]) ;
	} 
}


#if	CF_DEBUGS && CF_DEBUGPRINTA
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


