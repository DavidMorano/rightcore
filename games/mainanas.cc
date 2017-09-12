/* main (ANAS) */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2017-09-01, David A­D­ Morano
	Written for fun!

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Given a string, we find all anagrams of it (and print them out).


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
#include	<unordered_map>
#include	<set>
#include	<vector>
#include	<list>
#include	<stack>
#include	<deque>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<ostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	VARDEBUGFNAME	"ANAS_DEBUGFILE"


/* name spaces */

using namespace	std ;


/* typedefs */

typedef set<string>	res_t ;


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */

struct anas_char {
	int		ch ;
	int		max = 1 ;
	vector<int>	counts ;
	anas_char(char _ch,int n) : ch(_ch), counts(n,0) { 
	} ;
} ;

typedef vector<anas_char>	anasmap_t ;

struct anas {
	anasmap_t	m ;
	string		src ;
	string		tmp ;
	res_t		*resp = NULL ;
	int		total = 0 ;
	int		n = 0 ;
	anas() = default ;
	anas(res_t *_rp,cchar *sp) : src(sp), resp(_rp) {
	    n = src.length() ;
	    debugprintf("anas::ctor: ent s=%s n=%u\n",src.c_str(),n) ;
	    tmp.resize(n,'-') ;
	    for (auto ch : src) {
		loadchar(ch) ;
	    } /* end for */
	} ;
	void loadchar(char ch) {
	    const int	wn = m.size() ;
	    bool	f = false ;
	    int		w ;
	    for (w = 0 ; w < wn ; w += 1) {
		f = (m[w].ch == ch) ;
		if (f) break ;
	    }
	    if (f) {
		m[w].max += 1 ;
	    } else {
		struct anas_char	item(ch,n) ;
		m.push_back(item) ;
	    }
	} ;
	bool canadd(int i,int w) {
	    bool	f = (m[w].counts[i] < m[w].max) ;
	    debugprintf("anas::canadd: i=%u w=%u f=%u\n",i,w,f) ;
	    return f ;
	} ;
	void inc(int i,int w) {
	    debugprintf("anas::inc: i=%u w=%u\n",i,w) ;
	    m[w].counts[i] += 1 ;
	    debugprintf("anas::inc: new c=%u\n",m[w].counts[i]) ;
	} ;
	void enter(int i) {
	    const int	n = m.size() ;
	    debugprintf("anas::enter: i=%u\n",i) ;
	    if (i > 0) {
		for (int w = 0 ; w < n ; w += 1) {
		    m[w].counts[i] = m[w].counts[i-1] ;
		    debugprintf("anas::enter: counts[%u]=%u\n",
			w,m[w].counts[i]) ;
		}
	    } else {
		for (int w = 0 ; w < n ; w += 1) {
		    m[w].counts[i] = 0 ;
		}
	    }
	} ;
	bool mkstr(int i,int w) {
	    bool	f = false ;
	    debugprintf("anas::mkstr: ent i=%u w=%u\n",i,w) ;
	    if (i < n) {
	    debugprintf("anas::mkstr: enter() \n") ;
	        enter(i) ;
	    debugprintf("anas::mkstr: enter() ret\n") ;
		    if (canadd(i,w)) {	/* restriction */
	    debugprintf("anas::mkstr: canadd() yes\n") ;
		        tmp[i] = m[w].ch ;
		        inc(i,w) ;
			f = true ;
		        if ((i+1) == n) {
	    debugprintf("anas::mkstr: hit=%s\n",tmp.c_str()) ;
	    	            resp->insert(tmp) ;
			    total += 1 ;
		        } else {
	                    recurse(i+1) ;
		        }
		    } /* end if (hit) */
	    } /* end if (possible) */
	    debugprintf("anas::mkstr: ret f=%u\n",f) ;
	    return f ;
	} ; /* end method (mkstr) */
	int num() const {
	    return total ;
	} ;
	void recurse(int d) {
	    int		n = m.size() ;
	    for (int j = 0 ; j < n ; j += 1) {
    	        mkstr(d,j) ;
	    }
	} ;
} ;


/* forward references */

static int	anas1(res_t *,cchar *) ;

static void	printres(res_t *) ;


/* local variables */

static cchar	*cases[] = {
	"ab",
	"abc",
	"abcd",
	"abcdc",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	res_t		res ;
	const int	algos[] = { 1 } ;
	int		ex = 0 ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    cchar	*cp ;
	    if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	        rs = debugopen(cp) ;
	        debugprintf("main: starting DFD=%d\n",rs) ;
	    }
	}
#endif /* CF_DEBUGS */

	for (auto sc : cases) {
	    if (sc != NULL) {
	        for (auto al : algos) {
	            int	rs = 0 ;
		    res.clear() ;
	                cout << "algo=" << al << " sc=" << sc << endl ;
	                switch (al) {
	                case 1:
	                    rs = anas1(&res,sc) ;
		            break ;
	                } /* end switch */
	                printres(&res) ;
		    if (rs < 0) break ;
	        } /* end for (algo) */
	        cout << endl ;
	    } /* end if (non-null) */
	    if (rs < 0) break ;
	} /* end for (cases) */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


/* this is a very bad solution! */
static int anas1(res_t *resp,cchar *sc)
{
	const int	n = strlen(sc) ;
	int		rs = SR_OK ;
	if (n > 0) {
	    anas	worker(resp,sc) ;
	    worker.recurse(0) ;
	    cout << "num=" << worker.num() << endl ;
  	} /* end if (valid) */
	return rs ;
}
/* end subroutine (anas1) */


static void printres(res_t *rp)
{
	res_t::iterator	end = rp->end() ;
	res_t::iterator	it = rp->begin() ;
	while (it != end) {
	    cout << (*it) << endl ;
	    it++ ;
	}
}
/* end subroutine (printres) */


