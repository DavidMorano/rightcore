/* testop */
/* lang=C++11 */

/* test operations */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_COPY		1
#define	CF_INIT		0
#define	CF_TESTLAMBDA	0		/* test Lambda Functions */
#define	CF_TESTIO	0		/* |testio()| */
#define	CF_LISTUP	1		/* list up things */
#define	CF_ADD		0		/* add */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>
#include	<cinttypes>
#include	<new>
#include	<initializer_list>
#include	<algorithm>
#include	<functional>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */

struct thing ;

template <typename T,typename Comp = std::less<T>>
class ourcon {
	vector<T>	list ;
	Comp		keycmp ;
public:
	int add(const T &v) {
	     int	rs = SR_OK ;
	     list.push_back(v) ;
	     return rs ;
	}
	int add(T &v) {
	     int	rs = SR_OK ;
	     list.push_back(v) ;
	     return rs ;
	}
	int findmin(const T **rpp) {
	    int		rs = SR_OK ;
	    if (rpp != NULL) {
		if (! list.empty()) {
		    const T *vminp = NULL  ;
		fprintf(stderr,"testop/ourcon_findmin: not-empty\n") ;
		    for (const T &v : list) {
		fprintf(stderr,"testop/ourcon_findmin: v=%d\n",
			v.getval()) ;
		        if (vminp != NULL) {
		            if (keycmp(v,*vminp)) {
			        vminp = &v ;
			    }
		        } else {
			    vminp = &v ;
		        }
	 	    } /* end for */
		    *rpp = vminp ;
		} else {
		    *rpp = NULL ;
		}
	    } else {
		rs = SR_FAULT ;
	    }
	    return rs ;
	} ;
} ;

struct thingless ;

struct thing {
	int		id = 0 ;
	thing() : id(0) {
	    fprintf(stderr,"main: thing:construct\n") ;
	} ;
	thing(int aid) : id(aid) {
	    fprintf(stderr,"main: thing:construct(%d)\n",aid) ;
	} ;
	thing(const thing &a) {
	    *this = a ;
	} ;
#if	CF_COPY
	thing &operator = (const thing &a) {
	    fprintf(stderr,"main: thing:assignment\n") ;
	    if (this != &a) {
	        id = a.id ;
	    }
	    return *this ;
	} ;
#endif /* CF_COPY */
	~thing() {
	    cchar	*fmt = "main: thing:destruct(%u)\n" ;
	    fprintf(stderr,fmt,id) ;
	    id = 0 ;
	} ;
	int		init(int) ;
	int		getval() const {
	    return id ;
	} ;
	thing		&operator += (const thing) ;
	thing		&operator += (const thing&) ;
	friend thing	operator + (const thing&,const thing&) ;
	friend bool less(const thing &t1,const thing &t2) {
	    return (t1.id < t2.id) ;
	} ;
	friend bool greater(const thing &t1,const thing &t2) {
	    return (t1.id > t2.id) ;
	} ;
	friend bool operator < (const thing &t1,const thing &t2) {
	    return (t1.id < t2.id) ;
	} ;
	friend bool operator > (const thing &t1,const thing &t2) {
	    return (t1.id > t2.id) ;
	} ;
	friend bool operator == (const thing &t1,const thing &t2) {
	    return (t1.id == t2.id) ;
	} ;
	friend thingless ;
} ;

struct thingless {
    bool operator () (const thing &t1,const thing &t2) const {
	return (t1.id < t2.id) ;
    } ;
} ;

int thing::init(int a)
{
	id = a ;
	return 0 ;
}

/* this IS is a MEMBER function */
thing &thing::operator += (const thing &b)
{
	fprintf(stderr,"main: operator C\n") ;
	this->id += b.id ;
	return *this ;
}

thing &thing::operator += (const thing b)
{
	fprintf(stderr,"main: operator B\n") ;
	this->id += b.id ;
	return *this ;
}

/* this is a NON-MEMBER function */
thing operator + (const thing &a,const thing &b)
{
	thing	r ;
	fprintf(stderr,"main: operator A\n") ;
	r.id = a.id + b.id ;
	r.id = a.id + b.id ;
	return r ;
}


/* forward references */

#if	CF_TESTLAMBDA
static int testlambda(void) ;
#endif /* CF_TESTLAMBDA */

#if	CF_TESTIO
static int testio() ;
static int readline(ifstream &,char *,int) ;
#endif /* CF_TESTIO */


/* local variables */


/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	FILE		*efp = stderr ;
	thing		a(11), b(23), c(9) ;
	int		rs = SR_OK ;

	fprintf(efp,"main: ent\n") ;

#if	CF_ADD
	c = a + b ;
	fprintf(efp,"main: thing:c id=%u\n",c.id) ;
#endif

#if	CF_LISTUP
	{
	    ourcon<thing,thingless> ol ;
	    ol.add(a) ;
	    ol.add(b) ;
	    ol.add(c) ;
	    {
	        const thing	*tp ;
	        if ((rs = ol.findmin(&tp)) >= 0) {
		    const int	v = tp->getval() ;
	            cout << "min=" << v << endl ;
		} else {
	            cout << "bad rs=" << rs << endl ;
		}
	    }
	}
#endif /* CF_LISTUP */

#if	CF_INIT
	{
	    int	r ;
	a.init(1) ;
	b.init(2) ;
	c.init(3) ;

	r = a.get() ;
	fprintf(efp,"main: a.r=%d\n",r) ;

	r = b.get() ;
	fprintf(efp,"main: b.r=%d\n",r) ;

	r = c.get() ;
	fprintf(efp,"main: c.r=%d\n",r) ;
	}
#endif /* CF_INIT */



#if	CF_TESTLAMBDA
	fprintf(efp,"main: lambda\n") ;
	(void) testlambda() ;
#endif

#if	CF_TESTIO
	testio() ;
#endif /* CF_TESTIO */

	fprintf(efp,"main: ret\n") ;

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_TESTIO

static int testio()
{
	int		rs = SR_OK ;
	{
	    cchar	*ofn = "ourout.txt" ;
	    {
	        ofstream	os(ofn) ;
		if (os.good()) {
		    os << "Hello world!" << endl ;
		} else {
		    cerr << "error opening file" << endl ;
		}
	    }
	}
	{
	    cchar	*ifn = "ourin" ;
	    {
	        ifstream	is(ifn) ;
		if (is.good()) {
		    const int	llen = LINEBUFLEN ;
		    char	lbuf[LINEBUFLEN+1] ;
		    if ((rs = readline(is,lbuf,llen)) > 0) {
		        cout << "read> " << lbuf << endl ;
		    } else if (rs == 0) {
		        cout << "read> *EOF*" << endl ;
		    }
		} else {
		    cerr << "bad open on input" << endl ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (testio) */

int readline(ifstream &is,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	if (is.getline(lbuf,llen)) {
	    rs = is.gcount() ;
	}
	return rs ;
}
/* end subroutine (readline) */

#endif /* CF_TESTIO */


#if	CF_TESTLAMBDA
static int testlambda(void)
{
	vector<int>	mv = { 1, 2, 3, 4 } ;
	int		a[3] = { 2, 1, 3 } ;
	int		sum = 0 ;

	cout << "range-for " ;
	for (auto v : a) {
	    cout << v << ' ' ;
	}
	    cout << '\n' ;

	cout << "range-for " ;
	for (auto v : mv) {
	    cout << v << ' ' ;
	}
	    cout << '\n' ;

	cout << "for-each " ;
	auto ibegin = mv.cbegin() ;
	auto iend = mv.cend() ;
	auto func = [&sum] (int v) { sum += v ; } ;
	for_each(ibegin,iend,func) ;
	cout << "sum=" << sum << '\n' ;

	return 0 ;
}
/* end subroutine (testlambda) */
#endif /* CF_TESTLAMBDA */


