/* main */
/* lang=C++11 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_COPY		1
#define	CF_INIT		0
#define	CF_TESTLAMBDA	1		/* test Lambda Functions */


#include	<envstandards.h>

#include	<stdio.h>
#include	<cinttypes>
#include	<new>
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

struct testcon {
	int		a ;
	testcon() : a(0) {
	    printf("testcon: construct a=%d\n",a) ;
	} ;
	~testcon() {
	    printf("testcon: destruct\n") ;
	} ;
	bool have() ;
} ;

struct thing {
	int		id ;
	int		a1 ;
	int		a2 ;
public:
	thing() : id(0), a1(0), a2(0) {
	    printf("main: thing:construct\n") ;
	} ;
	thing(int id) : id(id), a1(0), a2(0) {
	    printf("main: thing:construct(%d)\n",id) ;
	} ;
	thing(const thing &a) {
	    *this = a ;
	} ;
#if	CF_COPY
	thing &operator = (const thing &a) {
	    printf("main: thing:assignment\n") ;
	    if (this != &a) {
	        id = 17 ;
	        a1 = a.a1 ;
	        a2 = a.a2 ;
	    }
	    return *this ;
	} ;
#endif /* CF_COPY */
	~thing() {
	    printf("main: thing:destruct(%u) a1=%d a2=%d\n",id,a1,a2) ;
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
	printf("main: operator C\n") ;
	this->a1 += b.a1 ;
	this->a2 += b.a2 ;
	return *this ;
}

thing &thing::operator += (const thing b)
{
	printf("main: operator B\n") ;
	this->a1 += b.a1 ;
	this->a2 += b.a2 ;
	return *this ;
}

/* this is a NON-MEMBER function */
thing operator + (const thing &a,const thing &b)
{
	thing	r(18) ;
	printf("main: operator A\n") ;
	r.a1 = a.a1 + b.a1 ;
	r.a2 = a.a1 + b.a2 ;
	return r ;
}


/* forward references */

#if	CF_TESTLAMBDA
static int testlambda(void) ;
#endif /* CF_TESTLAMBDA */

static int testio() ;

static int readline(ifstream &,char *,int) ;


/* local variables */

static cchar	*hello = "hello world!" ;


/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	testcon		tc ;
	thing		a(1), b(2), c(3) ;
	const int	n = 10 ;

	printf("main: ent\n") ;

	tc.have() ;

	c = a + b ;
	printf("main: thing:c id=%u\n",c.id) ;

#if	CF_INIT
	{
	    int	r ;
	a.init(1) ;
	b.init(2) ;
	c.init(3) ;

	r = a.get() ;
	printf("main: a.r=%d\n",r) ;

	r = b.get() ;
	printf("main: b.r=%d\n",r) ;

	r = c.get() ;
	printf("main: c.r=%d\n",r) ;
	}
#endif /* CF_INIT */

	{
	string	as ;
	as = "hello world!\n" ;
	cout << as ;
	}

	{
	    int	a[n+1] ;
	    int	i ;
	    for (i = 0 ; i < n ; i += 1) a[i] = i ;
	    for (i = 0 ; i < n ; i += 1) {
		cout << a[i] ;
	    }
	    cout << '\n' ;
	}

	{
	    int	rch = '¿' ;
	    int	ch = MKCHAR('¿') ;
	    printf("main: rch=%08x ch=%08x\n",rch,ch) ;
	}

	(void) testlambda() ;

#ifdef	COMMENT
	{
	    ofstream	*osp ;
	    cchar	*fn = "ourout" ;
	    if ((ofstream *osp = new(nothrow) ofstream(fn)) != NULL) {
	        (*osp) << "Hello world!" << endl ;
	        osp->close() ;
	        delete osp ;
	    } /* end if (file) */
	}
#else /* COMMENT */
	testio() ;
#endif /* COMMENT */

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */


static int testio()
{
	int		rs = SR_OK ;
	{
	    cchar	*ofn = "ourout" ;
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


bool testcon::have() {
	const int	ans = sisub(hello,-1,"hello") ;
	printf("ans=%u\n",(ans >= 0)) ;
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


