/* mainlister */
/* lang=C++11 */

/* test of SLIST object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_DEBUGADD	1		/* debug adding */
#define	CF_DEBUGONE	0		/* debug one */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We really are just (mostly) testing the SLIST object.


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
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<ucmallreg.h>
#include	<localmisc.h>

#include	"slist.hh"


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	VARDEBUGFNAME	"LISTER_DEBUGFILE"


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

#if	CF_DEBUGS
static int	debugprintlist(slist<int> &,cchar *) ;
#endif

typedef slist<int>	ourlist ;

class listadder {
	ourlist			&l1 ;
	ourlist			&l2 ;
	ourlist			&lr ;
	ourlist::iterator	i1 ;
	ourlist::iterator	i2 ;
	ourlist::iterator	e1 ;
	ourlist::iterator	e2 ;
public:
	listadder(ourlist &alr,ourlist &al1,ourlist &al2)
	    : l1(al1), l2(al2), lr(alr) { 
	} ;
	int addall(int c) {
	    i1 = l1.begin() ;
	    i2 = l2.begin() ;
	    e1 = l1.end() ;
	    e2 = l2.end() ;
#if	CF_DEBUGS
	    debugprintlist(l1,"listadder::l1") ;
	    debugprintlist(l2,"listadder::l2") ;
#endif
	    return addone(c) ;
	} ;
	int addone(int c) {
#if	CF_DEBUGS
	    debugprintf("main/listadder_addone: ent c=%d\n",c) ;
#endif
	    if (c >= 0) {
#if	CF_DEBUGS
	        debugprintf("main/listadder_addone: in c=%d\n",c) ;
#endif
	        if ((i1 != e1) || (i2 != e2)) {
	            int		vr = c ;
	            c = 0 ;
	            if (i1 != e1) {
	                int	v = *i1 ; i1 += 1 ;
#if	CF_DEBUGS
	                debugprintf("main/listadder_addone: l1=%d\n",v) ;
#endif
	                vr += v ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: 1 vr=%d\n",vr) ;
#endif
	            if (i2 != e2) {
	                int	v = *i2 ; i2 += 1 ;
#if	CF_DEBUGS
	                debugprintf("main/listadder_addone: l2=%d\n",v) ;
#endif
	                vr += v ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: 2 vr=%d\n",vr) ;
#endif
	            if (vr >= 10) {
	                vr %= 10 ;
	                c = 1 ;
	            }
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: carrying c=%d\n",c) ;
#endif
	            lr.add(vr) ;
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: inserting done\n") ;
#endif
	            c = addone(c) ;
#if	CF_DEBUGS
	            debugprintf("main/listadder_addone: recurse done\n") ;
#endif
		} else {
		    if (c > 0) {
	                lr.add(1) ;
		    }
	            c = -1 ;
	        }
	    }
#if	CF_DEBUGS
	    debugprintf("main/listadder_addone: ret c=%d\n",c) ;
#endif
	    return c ;
	} ;
} ;


/* forward references */

static int	printlist(slist<int> &,cchar *) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start ;
#endif

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	{
	    slist<int>	l1 = { 2, 4, 6, 1, 7 	} ;
	    slist<int>	l2 = { 1, 5, 7, 2, 9 	} ;
	    slist<int>	lr ;

#if	CF_DEBUGS
	    debugprintlist(l1,"main-l1") ;
	    debugprintlist(l2,"main-l2") ;
#endif
	    printlist(l1,"l1") ;
	    printlist(l2,"l2") ;

#if	CF_DEBUGONE
	    {
	        lr.add(17) ;
#if	CF_DEBUGS
	        debugprintf("main: insert-print\n") ;
#endif
	        printlist(lr,"lr-test") ;
	        lr.clear() ;
	        printlist(lr,"lr-clear") ;
	    }
#endif /* CF_DEBUGONE */

#if	CF_DEBUGADD
	    {
	        listadder	a(lr,l1,l2) ;
#if	CF_DEBUGS
	        debugprintf("main: insert (%d)\n",CF_DEBUGADD) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: addall()\n") ;
#endif
	        a.addall(0) ;
#if	CF_DEBUGS
	        debugprintf("main: output()\n") ;
	        debugprintlist(lr,"lr-final") ;
#endif
	        printlist(lr,"lr") ;
	    } /* end block */
#endif /* CF_DEBUGADD */

	} /* end block (central) */

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


static int printlist(slist<int> &l,cchar *s)
{
	int		c = 0 ;
	cout << s ;
	for (auto v : l) {
	    c += 1 ;
	    cout << " " << v ;
	}
	cout << endl ;
	return c ;
}
/* end subroutine (printlist) */


#if	CF_DEBUGS
static int debugprintlist(slist<int> &l,cchar *s)
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


