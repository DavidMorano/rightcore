/* testop */
/* lang=C++11 */

/* test operations */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_INIT		0
#define	CF_TESTLAMBDA	0		/* test Lambda Functions */
#define	CF_TESTIO	0		/* |testio()| */
#define	CF_TESTIN	0		/* |testin()| */
#define	CF_LISTUP	0		/* list up things */
#define	CF_ADD		0		/* add */
#define	CF_INH		0		/* inheritance */
#define	CF_CALLOBJ	1		/* call-object */


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
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<ostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */

struct staticobj {
    staticobj() {
        cout << "staticobj:ctor\n" ;
    } ;
    ~staticobj() {
        cout << "staticobj:dtor\n" ;
    } ;
} ;

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
} ; /* end structure (ourcon) */

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
	thing &operator = (const thing &a) {
	    fprintf(stderr,"main: thing:assignment\n") ;
	    if (this != &a) {
	        id = a.id ;
	    }
	    return *this ;
	} ;
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
} ; /* end structure (thing) */

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

/* this IS a MEMBER function */
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

struct tupler {
	int	a = 0 ;
	int	b = 0 ;
	int	c = 0 ;
	tupler() { } ;
	tupler(const tupler &) = default ;
	tupler(initializer_list<int> &il) {
	    int	p = 0 ;
	    for (auto &e : il) {
		switch (p++) {
		case 0:
		    a = e ;
		    break ;
		case 1:
		    b = e ;
		    break ;
		case 2:
		    c = e ;
		    break ;
		} /* end switch */
	    }
	} ;
	friend ostream &operator << (ostream &out,const tupler &t) {
	    cout << "(" << t.a << "," << t.b << "m" << t.c << endl ;
	    return out ;
	} ;
} ;


/* forward references */

#if	CF_TESTLAMBDA
static int testlambda(void) ;
#endif /* CF_TESTLAMBDA */

#if	CF_TESTIO
static int testio() ;
static int readline(ifstream &,char *,int) ;
#endif /* CF_TESTIO */

#if	CF_TESTIN
static int testin() ;
#endif /* CF_TESTIN */

#if	CF_INH
static int inh() ;
#endif /* CF_INH */

#if	CF_CALLOBJ
static int testcallobj() ;
#endif


/* local variables */

static staticobj	so ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	FILE		*efp = stderr ;
	thing		a(11), b(23), c(9) ;
	int		ex = 0 ;
	int		rs = SR_OK ;

	fprintf(efp,"main: ent\n") ;

#if	CF_ADD
	if (rs >= 0) {
	c = a + b ;
	fprintf(efp,"main: thing:c id=%u\n",c.id) ;
	}
#endif

#if	CF_LISTUP
	if (rs >= 0) {
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
	if (rs >= 0) {
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
	if (rs >= 0) {
	    fprintf(efp,"main: lambda\n") ;
	    rs = testlambda() ;
	}
#endif

#if	CF_TESTIO
	if (rs >= 0) rs = testio() ;
#endif /* CF_TESTIO */

#if	CF_TESTIN
	if (rs >= 0) rs = testin() ;
#endif /* CF_TESTIN */

#if	CF_INH
	if (rs >= 0) rs = inh() ;
#endif

#if	CF_CALLOBJ
	testcallobj() ;
#endif /* CF_CALLOBJ */

	{
	    int	r = (-10 % 3) ;
	    fprintf(efp,"main: mod r=%d\n",r) ;
	}

	fprintf(efp,"main: ret rs=%d\n",rs) ;

	if (rs < 0) ex = 1 ;
	return ex ;
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


#if	CF_TESTIN
static int testin()
{
	const int	n = 20 ;
	int		rs = SR_OK ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    int	v ;
 	    cin >> v ;
	    if (cin.eof()) break ;
	    cout << "i" << i << "=" << v << endl ;
	}
	return rs ;
}
/* end subroutine (testin) */
#endif /* CF_TESTIN */


#if	CF_INH
struct A {
	int		n = 0 ;
	A() { 
	   fprintf(stderr,"A(%u) ctor ­\n",n) ;
	} ;
	A(int an) : n(an) {
	   fprintf(stderr,"A(%u) ctor\n",n) ;
	} ;
	~A() {
	   fprintf(stderr,"A(%u) dtor\n",n) ;
	} ;
} ;

struct B : public A {
	int		n = 0 ;
	B() {
	   fprintf(stderr,"B(%u) ctor ­\n",n) ;
	} ;
	B(int an) : n(an) {
	   fprintf(stderr,"B(%u) ctor\n",n) ;
	} ;
	~B() {
	   fprintf(stderr,"B(%u) dtor\n",n) ;
	} ;
} ;

static int inh() {
	A		a(23) ;
	B		b(1) ;
	cout << "n=" << a.n << endl ;
	return 0 ;
}
/* end subroutine (inh) */
#endif /* CG_INH */


#if	CF_CALLOBJ
struct callobj {
	int	v = 0 ;
	callobj() {
	    cout << "callobj::ctor\n" ;
	} ;
	~callobj() {
	    cout << "callobj::dtor\n" ;
	} ;
	int timeout() {
	    cout << "callobj::timeout\n" ;
	    return v ;
	} ;
} ;
static int testcallobj_sub(void *objp)
{
	callobj		*op = (callobj *) objp ;
	return op->timeout() ;
}
static int testcallobj()
{
	callobj		obj ;
	void		*objp ;
	void		*metp ;

	objp = &obj ;
	cout << "objp=" << objp << endl ;
	testcallobj_sub(objp) ;

	return 0 ;
}
#endif /* CF_CALLOBJ */


