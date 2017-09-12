/* testnew */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_COPY		1


/* revision history:

	= 2013 -07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<stdio.h>
#include	<cstdlib>
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
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */

extern "C" int	sisub(cchar *,int,cchar *) ;


/* global variables */


/* local structures (and methods) */

void *operator new (size_t sz) {
    void *p = malloc(sz) ;
    return p ;
}

void *operator new (size_t sz,const nothrow_t &nt) noexcept {
    void *p = malloc(sz) ;
    return p ;
}

void operator delete (void *p) noexcept {
    free(p) ;
}

void operator delete (void *p,const std::nothrow_t &nt) noexcept {
    free(p) ;
}


class testcon {
	int		a ;
public:
	testcon() : a(0) {
	    fprintf(stderr,"testcon: construct 0 a=%d\n",a) ;
	} ;
	testcon(int n) : a(n) {
	    fprintf(stderr,"testcon: construct 1 a=%d\n",a) ;
	} ;
	~testcon() {
	    fprintf(stderr,"testcon: destruct\n") ;
	} ;
	bool have() ;
	friend ostream& operator << (ostream &out,const testcon &obj) {
 	    out << obj.a ;
	    return out ;
	} ;
} ;

struct thing {
	int		id ;
	int		a1 ;
	int		a2 ;
public:
	thing() : id(0), a1(0), a2(0) {
	    fprintf(stderr,"main: thing:construct\n") ;
	} ;
	thing(int id) : id(id), a1(0), a2(0) {
	    fprintf(stderr,"main: thing:construct(%d)\n",id) ;
	} ;
	thing(const thing &a) {
	    *this = a ;
	} ;
#if	CF_COPY
	thing &operator = (const thing &a) {
	    fprintf(stderr,"main: thing:assignment\n") ;
	    if (this != &a) {
	        id = 17 ;
	        a1 = a.a1 ;
	        a2 = a.a2 ;
	    }
	    return *this ;
	} ;
#endif /* CF_COPY */
	~thing() {
	    fprintf(stderr,"main: thing:destruct(%u) a1=%d a2=%d\n",id,a1,a2) ;
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


/* forward references */

static int testio() ;

static int readline(ifstream &,char *,int) ;


/* local variables */

static cchar	*hello = "hello world!" ;


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	FILE		*efp = stderr ;
	fprintf(efp,"main: ent\n") ;
	{
	    testcon *np = new(nothrow) testcon(1) ;
	    cout << "testcon> " << *np << endl ;


	    delete np ;
	}
	fprintf(efp,"main: testio\n") ;
	testio() ;
	fprintf(efp,"main: ret\n") ;
	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


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


bool testcon::have() {
	const int	ans = sisub(hello,-1,"hello") ;
	fprintf(stderr,"ans=%u\n",(ans >= 0)) ;
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


