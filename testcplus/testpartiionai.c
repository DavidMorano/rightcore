/* partitionao */
/* lang=C99 */

/* partitionai function */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGMAIN	0		/* debug w/ test program */
#define	CF_DEBUGPRINTA	0		/* print arrays */


/* revision history:

	= 2001-10-04, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

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
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define		VARDEBUGFNAME	"TESTPART_DEBUGFILE"
#define		NALGO		2


/* typedefs */

typedef int	(*partpred_t)(int,int) ;


/* external subroutines */

extern void	arrswap(int *,int,int) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,cchar *,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

#if	CF_DEBUGS && CF_DEBUGPRINTA
static int	debugprinta(int *,int) ;
#endif


/* local variables */


int partitionai(int *a,int al,partpred_t fn,int pv)
{
	int		last = al ;
	int		i ;
#if	CF_DEBUGS
	debugprintf("partitionai: ent al=%u pv=%u\n",al,pv) ;
#if	CF_DEBUGPRINTA
	debugprinta(a,al) ;
#endif /* CF_DEBUGPRINTA */
#endif
	for (i = 0 ; i < last ; i += 1) {
	    const int	f = (*fn)(a[i],pv) ;
#if	CF_DEBUGS
	debugprintf("partitionai: i=%u e=%u:%u",i,a[i],f) ;
#endif
	    if (! f) {
		arrswap(a,i--,--last) ;
	    }
#if	CF_DEBUGS
	debugprinta(a,al) ;
#endif
	} /* end for */
#if	CF_DEBUGS
#if	CF_DEBUGPRINTA
	debugprinta(a,al) ;
#endif /* CF_DEBUGPRINTA */
	debugprintf("partitionai: ret p=%u\n",last) ;
#endif
	return last ;
}
/* end subroutine (partitionai) */


#if	CF_DEBUGS && CF_DEBUGPRINTA
static int debugprinta(int *a,int al)
{
	int		i ;
	for (i = 0 ; i < al ; i += 1) {
	    debugprintf(" %2u\\",a[i]) ;
	}
	debugprintf("\n") ;
	return 0 ;
}
#endif /* CF_DEBUGS */


#if	CF_DEBUGMAIN

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
		arrswap(a,i,pi++) ;
	    }
	}
	return pi ;
}

static int isPart(const int *a,int al,partpred_t fn,int pi)
{
	int		f = TRUE ;
	if (al > 1) {
	    const int	pv = a[pi-1] ;
	    int		i ;
	    for (i = 0 ; i < al ; i += 1) {
	        const int f_pred = (fn)(a[i],pv) ;
	        f = ((i < pi) && f_pred) || (! f_pred) ;
	        if (!f) break ;
	    }
	}
	return f ;
}

static void loadrand(int *a,int al)
{
	int		i ;
	for (i = 0 ; i < al ; i += 1) {
	    a[i] = rand() % 1000 ;
	}
}

static void loadagain(int *aa,int *a,int al)
{
	const int	size = (al*sizeof(int)) ;
	memcpy(aa,a,size) ;
}

int main(int argc,cchar **argv,cchar **envv)
{
	const int	al = 100 ;
	int		rs = SR_OK ;
	int		ex = 0 ;
	int		size ;
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

	size = (2*(al+1)*sizeof(int)) ;
	if ((rs = uc_malloc(size,&a)) >= 0) {
	    const int	on = 1000 ;
	    int		i ;
	    int		*aa = (a+(al+1)) ;
	    for (i = 0 ; i < on ; i += 1) {
		int	j ;
	        loadrand(a,al) ;
		for (j = 0 ; j < al ; j += 1) {
		    int	ans[NALGO] ;
		    int	pv = a[j] ;
		    int	g ;
		    for (g = 0 ; g < NALGO ; g += 1) {
			int	pi ;
		        loadagain(aa,a,al) ;
#if	CF_DEBUGS
			debugprintf("main: j=%u pv=%u g=%u\n",j,pv,g) ;
#endif
		        switch (g) {
		        case 0:
			    ans[g] = partest(aa,al,ourpred,pv) ;
			    break ;
		        case 1:
			    ans[g] = partitionai(aa,al,ourpred,pv) ;
			    break ;
			} /* end switch */
			if (! isPart(aa,al,ourpred,pi)) rs = SR_PROTO ;
#if	CF_DEBUGS
			debugprintf("main: while-out rs=%d\n",rs) ;
#endif
			if (rs < 0) break ;
		    } /* end if (algos) */
		    if (rs >= 0) {
			for (g = 1 ; g < NALGO ; g += 1) {
			    if (ans[g] != ans[0]) rs = SR_NOMSG ;
			    if (rs < 0) break ;
			}
		    }
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
#endif /* CF_DEBUGMAIN */


