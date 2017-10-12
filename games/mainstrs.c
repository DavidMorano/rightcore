/* main (strs) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-08-29, David A­D­ Morano
	Written originally in response to a challenge.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Given a length, we return a set of strings which can be formed out of
        the characters 'a', 'b', and 'c' -- given the restrictions:

	+ as many 'a' characters as you want
	+ at most one 'b' character
	+ at most two 'c' characters


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<climits>
#include	<cstring>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<vector>
#include	<list>
#include	<stack>
#include	<deque>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"STRS_DEBUGFILE"


/* name spaces */

using namespace	std ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */

typedef std::set<std::string>	res_t ;


/* forward references */

static void printres(const set<string> &res) ;

static set<string> strs1(int) ;
static set<string> strs2(int) ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	algos[] = { 1, 2 } ;
	const int	lengths[] = { 1, 2, 3, 4 } ;
	int		ex = 0 ;
	int		rs = SR_OK ;

#if	CF_DEBUGS || CF_DEBUG
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

	for (auto n : lengths) {
	    for (auto al : algos) {
	        set<string>	res ;
	        cout << "algo=" << al << " n=" << n << endl ;
	        switch (al) {
	        case 1:
	            res = strs1(n) ;
		    break ;
	        case 2:
	            res = strs2(n) ;
		    break ;
	        } /* end switch */
	        printres(res) ;
	    } /* end for */
	} /* end for */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#define	STRS1_NUM	3

static cchar	*strs1_strs = "abc" ;

struct strs1_item {
	int	c[STRS1_NUM] ;
	strs1_item() {
	    ClearCounts() ;
	} ;
	strs1_item &operator = (const strs1_item &wi) = default ;
	void ClearCounts() {
	    for (int i = 0 ; i < STRS1_NUM ; i += 1) c[i] = 0 ;
	} ;
	/* do not need 'move' constructor or assignment (too simple) */
	bool canadd(int w) { /* restrictions */
	    bool f = false ;
	    switch(w) {
	    case 0:
		f = true ;
		break ;
	    case 1:
		f = (c[w] < 1) ;
		break ;
	    case 2:
		f = (c[w] < 2) ;
		break ;
	    }
	    return f ;
	} ;
	void inc(int w) {
	    c[w] += 1 ;
	} ;
} ;

struct strs1_head {
	res_t		*resp ;
	strs1_item	wi ;
	string		s ;
	int		N = 0 ;
	int		total = 0 ;
	strs1_head(res_t *aresp,int an) : resp(aresp), N(an) { 
	    s.resize(N,'-') ;
	} ;
	void mkstr(strs1_item wi,int i,int w) {
		if (i < N) {
		    if (wi.canadd(w)) {	/* restriction */
		        s[i] = strs1_strs[w] ;
		        wi.inc(w) ;
		        if ((i+1) == N) {
	    	            resp->insert(s) ;
			    total += 1 ;
		        } else {
	                    recurse(wi,i+1) ;
		        }
		    } /* end if (hit) */
		} /* end if (possible) */
	} ; /* end method (mkstr) */
	int num() const {
	    return total ;
	} ;
	void recurse(strs1_item &wi,int i) {
    	    mkstr(wi,i,0) ;
    	    mkstr(wi,i,1) ;
    	    mkstr(wi,i,2) ;
	} ;
} ;

static res_t strs1(const int N) {
  	set<string>	res ;
	if (N > 0) {
	    strs1_item	wi ;
	    strs1_head	worker(&res,N) ;
    	    worker.recurse(wi,0) ;
	    cout << "num=" << worker.num() << endl ;
  	} /* end if (valid) */
	return res ;
}
/* end subroutine (strs1) */


#define	STRS2_NUM	3

static cchar	*strs2_strs = "abc" ;

struct strs2_item {
	int	c[STRS2_NUM] ;
	int	i = 0 ; /* index */
	int	w = 0 ; /* which-type */
	strs2_item() { } ;
	strs2_item(int _i,int _w) : i(_i), w(_w) { 
	    ClearCounts() ;
	} ;
	strs2_item &operator = (const strs2_item &wi) = default ;
	void ClearCounts() {
	    for (int i = 0 ; i < STRS2_NUM ; i += 1) c[i] = 0 ;
	} ;
	/* do not need 'move' constructor or assignment (too simple) */
	bool canadd(int w) { /* restrictions */
	    bool f = false ;
	    switch(w) {
	    case 0:
		f = true ;
		break ;
	    case 1:
		f = (c[w] < 1) ;
		break ;
	    case 2:
		f = (c[w] < 2) ;
		break ;
	    }
	    return f ;
	} ;
	void inc(int w) {
	    c[w] += 1 ;
	} ;
	void setindex(int ai) {
	    i = ai ;
	} ;
	void setwhich(int aw) {
	    w = aw ;
	} ;
} ;

static void strs2_push(stack<strs2_item> &s,struct strs2_item wi,int i)
{
	wi.setindex(i) ;
	for (int w = 0 ; w < 3 ; w += 1) {
	    wi.setwhich(w) ;
	    s.push(wi) ;
	}
}

static res_t strs2(const int N) {
  	set<string>	res ;
	int		total = 0 ;
	if (N > 0) {
    	    stack<strs2_item>	work ;
    	    strs2_item		wi ;
    	    string		s ;
    	    s.resize(N,'-') ;
    	    strs2_push(work,wi,0) ;
    	    while (! work.empty()) {
      		wi = work.top() ;
      		work.pop() ;
		if (wi.i < N) {
	    	    const int	i = wi.i ;
	    	    const int	w = wi.w ;
		    if (wi.canadd(w)) {	/* restriction */
		        s[i] = strs2_strs[w] ;
		        wi.inc(w) ;
		        if ((i+1) == N) {
	    	            res.insert(s) ;
			    total += 1 ;
		        } else {
	                    strs2_push(work,wi,i+1) ;
		        }
		    } /* end if (hit) */
		} /* end if (possible) */
    	    } /* end while */
  	} /* end if (valid) */
	cout << "num=" << total << endl ;
	return res ;
}
/* end subroutine (strs2) */


static void printres(const set<string> &res)
{
	for (auto &s : res) {
	    cout << s << endl ;
	}
}
/* end subroutine (printres) */


