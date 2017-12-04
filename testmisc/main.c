/* main (testmisc) */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_HELLO	0		/* hello */
#define	CF_FILEHELLO	0		/* file-hello */
#define	CF_TESTCON	0		/* test-con */
#define	CF_TESTIO	1
#define	CF_DYMARRAY	0		/* dynamic arrays */
#define	CF_SIGNED	0		/* signed numbers */
#define	CF_COPY		0		
#define	CF_OP		1		/* operator */
#define	CF_INIT		1		
#define	CF_LAMBDA	0		/* test Lambda Functions */
#define	CF_VECTUP	0		
#define	CF_SET		1		/* set */
#define	CF_HASDUP	1		/* has duplicate */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	General testing.


*******************************************************************************/


#include	<envstandards.h>
#include	<stdio.h>
#include	<cstdlib>
#include	<cinttypes>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<map>
#include	<unordered_set>
#include	<unordered_map>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<ostream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<hasduplicate.hh>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */

struct testcon {
	int		a ;
	testcon() : a(0) {
	    fprintf(stderr,"main/testcon: construct a=%d\n",a) ;
	} ;
	~testcon() {
	    fprintf(stderr,"main/testcon: destruct\n") ;
	} ;
	bool have() ;
} ;

struct thing {
	int		id ;
	int		a1 ;
	int		a2 ;
public:
	thing() : id(0), a1(0), a2(0) {
	    fprintf(stderr,"main/thing: thing:construct\n") ;
	} ;
	thing(int id) : id(id), a1(0), a2(0) {
	    fprintf(stderr,"main/thing: thing:construct(%d)\n",id) ;
	} ;
	thing(const thing &a) {
	    *this = a ;
	} ;
#if	CF_COPY
	thing &operator = (const thing &a) {
	    fprintf(stderr,"main/thing: thing:assignment\n") ;
	    if (this != &a) {
	        id = 17 ;
	        a1 = a.a1 ;
	        a2 = a.a2 ;
	    }
	    return *this ;
	} ;
#endif /* CF_COPY */
#if	CF_OP
	int operator * () {
	    return id ;
	} ;
#endif /* CF_OP */
	~thing() {
	    fprintf(stderr,"main/thing: thing:destruct(%u) a1=%d a2=%d\n",
		id,a1,a2) ;
	    id = 0 ;
	} ;
	int		init(int) ;
	int		get() ;
	thing		&operator += (const thing) ;
	thing		&operator += (const thing&) ;
	friend thing	operator + (const thing&,const thing&) ;
} ;

int thing::init(int a)
{
	a1 = a ;
	a2 = a * 2 ;
	return 0 ;
}

int thing::get()
{
	return (a1 + a2) ;
}

/* this IS is a MEMBER function */
thing &thing::operator += (const thing &b)
{
	fprintf(stderr,"main: operator C\n") ;
	this->a1 += b.a1 ;
	this->a2 += b.a2 ;
	return *this ;
}

thing &thing::operator += (const thing b)
{
	fprintf(stderr,"main: operator B\n") ;
	this->a1 += b.a1 ;
	this->a2 += b.a2 ;
	return *this ;
}

/* this is a NON-MEMBER function */
thing operator + (const thing &a,const thing &b)
{
	thing	r(18) ;
	fprintf(stderr,"main: operator A\n") ;
	r.a1 = a.a1 + b.a1 ;
	r.a2 = a.a1 + b.a2 ;
	return r ;
}

struct tupler {
	int	a = 0 ;
	int	b = 0 ;
	int	c = 0 ;
	tupler() { } ;
	tupler(const tupler &) = default ;
	tupler(const initializer_list<int> &il) {
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
	tupler &operator = (const tupler &) = default ;
	tupler &operator = (const initializer_list<int> &il) {
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
	    return (*this) ;
	} ;
	friend ostream &operator << (ostream &out,const tupler &t) {
	    out << "(" << t.a << "," << t.b << "," << t.c << ")" << endl ;
	    return out ;
	} ;
} ;


/* forward references */

#if	CF_LAMBDA
static int testlambda(void) ;
#endif /* CF_LAMBDA */

#if	CF_TESTIO
static int testio() ;
#endif

static int readline(ifstream &,char *,int) ;


/* local variables */

static cchar	*hello = "hello world!" ;


/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	testcon		tc ;
	thing		a(1), b(2), c(3) ;

	fprintf(stderr,"main: ent\n") ;

#if	CF_HELLO
	{
	string	as ;
	as = "hello world!\n" ;
	cout << as ;
	}
#endif /* CF_HELLO */

#if	CF_TESTCON
	tc.have() ;
	c = a + b ;
	fprintf(stderr,"main: thing:c id=%u\n",c.id) ;
#endif	/* CF_TESTCON */

#if	CF_OP
	fprintf(stderr,"main: OP a=%d b=%d c=%d\n",*a,*b,*c) ;
#endif /* CF_OP */

#if	CF_INIT
	{
	    int	r ;
	a.init(1) ;
	b.init(2) ;
	c.init(3) ;

	r = a.get() ;
	fprintf(stderr,"main: a.r=%d\n",r) ;

	r = b.get() ;
	fprintf(stderr,"main: b.r=%d\n",r) ;

	r = c.get() ;
	fprintf(stderr,"main: c.r=%d\n",r) ;

	    {
		tupler	a = { 1, 3, 5 } ;
		cout << a << endl ;
	    }

	}
#endif /* CF_INIT */

#if	CF_VECTUP
	    {
		vector<tupler>	v = { { 1, 3, 5 } } ;
		cout << v[0] << endl ;
	    }
#endif /* COMMENT */

#if	CF_DYMARRAY
	{
	    const int	n = 10 ;
	    {
	        int	a[n+1] ;
	        int	i ;
	        for (i = 0 ; i < n ; i += 1) a[i] = i ;
	        for (i = 0 ; i < n ; i += 1) {
		    cout << a[i] ;
	        }
	        cout << '\n' ;
	    }
	}
#endif /* CF_DYMARRAY */

#if	CF_SIGNED
	{
	    int	rch = '¿' ;
	    int	ch = MKCHAR('¿') ;
	    fprintf(stderr,"main: rch=%08x ch=%08x\n",rch,ch) ;
	}
#endif /* CF_SIGNED */

#if	CF_LAMBDA
	(void) testlambda() ;
#endif /* CF_LAMBDA */

#if	CF_SET
	{
	    typedef unordered_set<int>	set_t ;
	    const unordered_set<int>	os = { 0, 1, 2, 18, 27 } ;
	    {
	        set_t::const_iterator	end = os.cend() ;
	        set_t::const_iterator	it = os.cbegin() ;
	        cout << "set\n" ;
	        auto fo = [] (int e) { cout << " " << e ; } ;
	        for_each(it,end,fo) ;
	        cout << endl ;
	    }
	}
#endif /* CF_SET */

#if	CF_HASDUP
	{
	    const int	os[] = { 0, 1, 2, 1, 18, 27 } ;
	    {
		int	sl = nelem(os) ;
		bool	f = false ;
		if (hasduplicate(os,sl)) {
		    f = true ;
		}
	        cout << "dup=" << f << endl ;
	    }
	}
#endif /* CF_HASDUP */

#if	CF_FILEHELLO
	{
	    ofstream	*osp ;
	    cchar	*fn = "ourout.txt" ;
	    if ((ofstream *osp = new(nothrow) ofstream(fn)) != NULL) {
	        (*osp) << "Hello world!" << endl ;
	        osp->close() ;
	        delete osp ;
	    } /* end if (file) */
	}
#endif /* CF_FILEHELLO */

#if	CF_TESTIO
	testio() ;
#endif /* CF_TESTIO */

	return 0 ;
}
/* end subroutine (main) */


auto special(int a, int b) -> int
{
	return (a+b) ;
}


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
#endif /* CF_TESTIO */


#if	CF_LAMBDA
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
	auto itb = mv.cbegin() ;
	auto ite = mv.cend() ;
	auto func = [&sum] (int v) { sum += v ; } ;
	for_each(itb,ite,func) ;
	cout << "sum=" << sum << '\n' ;

	return 0 ;
}
/* end subroutine (testlambda) */
#endif /* CF_LAMBDA */


bool testcon::have() {
	const int	ans = sisub(hello,-1,"hello") ;
	fprintf(stderr,"main/testconn::have: ans=%u\n",(ans >= 0)) ;
	return (ans >= 0) ;
}


int readline(ifstream &is,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	if (is.getline(lbuf,llen)) {
	    rs = is.gcount() ;
	}
	return rs ;
}
/* end subroutine (readline) */


