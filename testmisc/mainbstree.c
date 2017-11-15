/* mainbstree */
/* lang=C++11 */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUGPL	0
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEBUGADD	1		/* debug adding */
#define	CF_DEBUGONE	1		/* debug one */
#define	CF_PRINTLIST	1		/* |printlist()| */
#define	CF_CLASSLESS	1		/* class |less| */
#define	CF_DEPTH	1		/* find max-depth */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the test jig for testing the BSTREE object.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<cstring>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<list>
#include	<deque>
#include	<stack>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<ucmallreg.h>
#include	<localmisc.h>

#include	"bstree.hh"


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	VARDEBUGFNAME	"BSTREE_DEBUGFILE"


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;
extern "C" int	mkrevstr(char *,int) ;
extern "C" int	ctdeci(char *,int,int) ;

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugprinthexblock(cchar *,int,const void *,int) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,cchar *,int) ;
#endif

extern "C" cchar	*getourenv(cchar **,cchar *) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */

#if	CF_CLASSLESS
struct intless {
	bool operator () (const int &v1,const int &v2) const {
	    return (v1 < v2) ;
	} ;
	bool operator () (int &v1,int &v2) const {
	    return (v1 < v2) ;
	} ;
} ;
#else /* CF_CLASSLESS */
static bool intless(const int &v1,const int &v2) {
	return (v1 < v2) ;
} ;
#endif /* CF_CLASSLESS */

typedef bstree<int,intless>	ourlist ;

#if	CF_DEBUGS && CF_DEBUGPL
static int	debugprintlist(ourlist &,cchar *) ;
#endif

static int printwalk(ourlist &) ;
static int printinter(ourlist &) ;


/* forward references */

#if	CF_PRINTLIST
static int	printlist(ourlist &,cchar *) ;
#endif


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start ;
#endif
	int		rs = SR_OK ;
	cchar		*cp ;
#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */
#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	{
	    ourlist 	listsrc = { 4, 2, 6, 1, 7, 3 } ;
	    ourlist 	lr ;

#if	CF_DEBUGS && CF_DEBUGPL
	debugprintlist(listsrc,"main-listsrc") ;
#endif

	printwalk(listsrc) ;
	printinter(listsrc) ;

#if	CF_PRINTLIST
	printlist(listsrc,"listsrc") ;
#endif

#if	CF_DEBUGONE
	    {
	        lr.add(17) ;
#if	CF_DEBUGS
	        debugprintf("main: insert-print\n") ;
#endif

		printwalk(lr) ;

#if	CF_PRINTLIST
	        printlist(lr,"lr-test") ;
	        lr.clear() ;
	        printlist(lr,"lr-clear") ;
#endif

	    } /* end block */
#endif /* CF_DEBUGONE */

#if	CF_DEPTH
	{
	    bstree_depth	md ;
	    int	d = listsrc.depth(&md) ;
	    cout << "listsrc depth=" << d << endl ;
	    cout << "listsrc min=" << md.min << endl ;
	    cout << "listsrc max=" << md.max << endl ;
	}
#endif

#if	CF_DEBUGADD
	{
	    const int	n = 23 ;
	    int		rc ;

	    cout << "add-all-before\n" ;
	    rc = lr.add(listsrc) ;
	    cout << "add after c=" << rc << endl ;
	    printinter(lr) ;

	    cout << "add-one-before\n" ;
	    printinter(lr) ;
	    rc = lr.add(n) ;
	    cout << "add-one-after c=" << rc << endl ;
	    printinter(lr) ;

	    {
	        bstree<int,intless>::iterator	it ;
	        cout << "find-one-before" << endl ;
	        if (it = lr.find(n)) {
#if	CF_DEBUGS
		    debugprintf("main: found\n") ;
#endif
	            cout << "found" << endl ;
	            cout << "delete-one-before" << endl ;
	            printinter(lr) ;
	            rc = lr.del(it) ;
	            cout << "delete-one-after c=" << rc << endl ;
	            printinter(lr) ;
	        }
	    }

/* delete all that we can find */

	    cout << "delete-all-before\n" ;
	    printinter(lr) ;
	    for (const int &v : listsrc) {
		int	c ;
		lr.delval(v) ;
		c = lr.count() ;
		cout << "c=" << c << endl ;
	    }
	    cout << "delete-all-after\n" ;
	    printinter(lr) ;

#if	CF_PRINTLIST
	    printlist(lr,"lr") ;
#endif

	} /* end block */
#endif /* CF_DEBUGADD */

	} /* end block (main) */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        const char	*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif
	return 0 ;
}
/* end subroutine (main) */


/* for memory allocation tracking */
void *operator new(size_t sz) {
    	void *p ;
	uc_malloc(sz,&p) ;
    	return p ;
}

/* for memory allocation tracking */
void *operator new(size_t sz,const nothrow_t &nt) noexcept {
    	void *p ;
	uc_malloc(sz,&p) ;
    	return p ;
}

/* for memory allocation tracking */
void operator delete(void *p) noexcept {
    	uc_free(p) ;
}

/* for memory allocation tracking */
void operator delete(void *p,const std::nothrow_t &nt) noexcept {
    	uc_free(p) ;
}


/* local subroutines */


#if	CF_PRINTLIST
static int printlist(ourlist &l,cchar *s)
{
	int		c = 0 ;
	cout << s << ">" ;
	for (auto v : l) {
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
}
/* end subroutine (printlist) */
#endif /* CF_PRINTLIST */


#if	CF_DEBUGS && CF_DEBUGPL
static int debugprintlist(ourlist &l,cchar *s)
{
	const int	plen = LINEBUFLEN ;
	int		rs = SR_OK ;
	char		*pbuf ;
	if ((pbuf = new(nothrow) char [plen+1]) >= 0) {
	    const int	dlen = DIGBUFLEN ;
	    const int	n = 28 ;
	    int		i = 0 ;
	    char	dbuf[DIGBUFLEN+1] ;
	    char	*bp = pbuf ;
	    for (auto v : l) {
	        if (i++ >= n) break ;
	        if (i > 0) *bp++ = ' ' ;
	        rs = ctdeci(dbuf,dlen,v) ;
	        bp = strwcpy(bp,dbuf,rs) ;
	    } /* end if (for) */
	    debugprintf("main: %s> %s\n",s,pbuf) ;
	    delete [] pbuf ;
	} /* end if (m-a-f) */
	return rs ;
}
#endif /* CF_DEBUGS */


static int printwalk(ourlist &l)
{
	vector<int>		res ;
	int	c = 0 ;
	l.storevec(res) ;
	for (int v : res) {
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
} 
/* end suroutine (printwalk) */


static int printinter(ourlist &l)
{
	ourlist::iterator	it = l.begin() ;
	int	c = 0 ;
	while (it) {
	    int	v = *it ; it += 1 ;
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
} 
/* end suroutine (printwalk) */


