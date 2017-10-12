/* main (MINSQUARES) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-08-29, David A­D­ Morano
	Written originally in response to a challenge.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Given a length and width of a piece of paper, we calculate the minimum
        number of squares that can be cut from it.


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

#define	MAXTRY		200

#define	VARDEBUGFNAME	"MINSQUARES_DEBUGFILE"


/* name spaces */

using namespace	std ;


/* external subroutines */

extern "C" int	igcd(int,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* local structures */

struct papercase_equal ;
struct papercase_less ;
struct papercase_hash ;

struct papercase {
	int		h = 0 ;	/* height */
	int		w = 0 ;	/* width */
	papercase() = default ;
	papercase(const papercase &) = default ;
	papercase(int _h,int _w) : h(_h), w(_w) { } ;
	friend papercase_equal ;
	friend papercase_less ;
	friend papercase_hash ;
	papercase &operator = (initializer_list<int> &il) {
	    int	p = 0 ;
	    for (auto e : il) {
		switch (p++) {
		case 0:
		    h = e ;
		    break ;
		case 1:
		    w = e ;
		    break ;
		}
	    }
	    return (*this) ;
	} ;
	friend ostream &operator  << (ostream &out,const papercase &pc) {
	    out << "(" << pc.h << "," << pc.w << ")" ;
	    return out ;
	} ;
} ;

struct papercase_equal {
	bool operator () (const papercase &p1,const papercase &p2) const {
	    return (p1.h == p2.h) && (p1.w == p2.w) ;
	} ;
} ;

struct papercase_less {
	bool operator () (const papercase &p1,const papercase &p2) const {
	    int	rc ;
	    if ((rc = (p1.h - p2.h)) == 0) {
		rc = (p1.w - p2.w) ;
	    }
	    return rc ;
	} ;
} ;

struct papercase_hash {
	size_t operator () (const papercase &pc) const {
	    size_t	h = pc.h ;
	    size_t	w = pc.w ;
	    return (h ^ w) ;
	} ;
} ;


/* forward references */

static void	printres(int) ;

static int	minsquares1(const papercase &) ;
static int	minsquares2(const papercase &) ;
static int	minsquares3(const papercase &) ;


/* local variables */

static const papercase	pcases[] = {
	{ 30, 35 },
	{ 13, 29 },
	{ 39, 17 },
	{ 36, 24 },
	{ 4, 4 },
	{ 8, 4 },
	{ 10, 4 },
	{ 0, 0 }
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	const int	algos[] = { 1, 2, 3 } ;
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

	for (auto pc : pcases) {
	    if (pc.h > 0) {
	        for (auto al : algos) {
	            int	res = 0 ;
	            cout << "algo=" << al << " pc=" << pc << endl ;
	            switch (al) {
	            case 1:
	                res = minsquares1(pc) ;
		        break ;
	            case 2:
	                res = minsquares2(pc) ;
		        break ;
	            case 3:
	                res = minsquares3(pc) ;
		        break ;
	            } /* end switch */
	            printres(res) ;
	        } /* end for (algo) */
	        cout << endl ;
	    } /* end if */
	} /* end for (pcases) */

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
	debugclose() ;
#endif

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


/* this is a (quite) bad solution! */
static int minsquares1(const papercase &pc) 
{
	const int	g = igcd(pc.h,pc.w) ;
	return (pc.h / g) * (pc.w / g) ;
}
/* end subroutine (minsquares1) */


/* standard greedy solution (cutting and rotating remaining paper) */
static int minsquares2(const papercase &pc)
{
	int		ans = 0 ;
	int		a = pc.h ;
	int		b = pc.w ;
	if (b > a) swap(a,b) ; /* get smallest side */
	while (b > 0) {
	    const int	r = (a % b) ;
	    ans += (a/b) ;
	    a = b ; /* rotate remaining piece 90 degrees */
	    b = r ; /* rotate remaining piece 90 degrees */
	} /* end while */
	return ans ;
}
/* end subroutine (minsquares2) */


/* third algorithm */

typedef unordered_map<papercase,int,papercase_hash,papercase_equal>	
		cache_t ;

typedef cache_t::const_iterator		cit_t ;

struct minsquares3_head {
	int		maxtry = MAXTRY ;
	cache_t		cache ;
	minsquares3_head() : minsquares3_head(MAXTRY) { 
	} ;
	minsquares3_head(int amaxtry) : maxtry(amaxtry) { 
	} ;
	int haveCache(int h,int w) const {
	    papercase	pc(h,w) ;
	    cit_t	end = cache.end() ;
	    cit_t	it ;
	    int		ans = 0 ;
	    if ((it = cache.find(pc)) != end) {
		ans = it->second ;
	    }
	    return ans ;
	} ;
	void storeCache(int h,int w,int ans) {
	    papercase	pc(h,w) ;
	    {
	        pair<papercase,int>	ent(pc,ans) ;
	        cache.insert(ent) ;
	    }
	} ;
	int find(int h,int w) { /* main calculation */
	    int		ans = 0 ;
    	    if (h == w) {
	        ans = 1 ;
	    } else if ((h == 1) || (w == 1)) {
		ans = (h == 1) ? w : h ;
	    } else if ((ans = haveCache(h,w)) == 0) {
    	        int	ver_min = INT_MAX ;
    	        int	hor_min = INT_MAX ;
		int	i ;
    	        for (i = 1 ; i <= (h/2) ; i += 1) { /* cut horizontally */
		    int wans = find(i,w) + find(h-i,w) ;
        	    hor_min = min(wans,hor_min) ; 
    		} /* end for */
    	        for (i = 1 ; i <= (w/2) ; i += 1) { /* cut vertically */
		    int	hans = find(h,i) + find(h,w-i) ;
        	    ver_min = min(hans,ver_min) ;
		} /* end for */
		ans = min(ver_min,hor_min) ; /* lesser of each ways */
    	    } /* end if */
            if (ans > 0) {
		storeCache(h,w,ans) ;
	    } 
    	    return ans ;
	} ;
} ;

static int minsquares3(const papercase &pc)
{
	minsquares3_head	obj ;
	int		res = 0 ;
	int		h = pc.h ;
	int		w = pc.w ;
	res = obj.find(h,w) ;
	return res ;
}
/* end subroutine (minsquares3) */


static void printres(int r)
{
	cout << r << endl ;
}
/* end subroutine (printres) */


