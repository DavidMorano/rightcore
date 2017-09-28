/* mainhide */
/* lang=C++11 */

/* name hiding in C++ */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

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
extern "C" int	mkrevstr(char *,int) ;

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* global variables */


/* local structures (and methods) */

class hunker {
	int		v = 1 ;
public:
	hunker() { } ;
	hunker &operator = (int av) {
	    v = av ;
	    return (*this) ;
	} ;
	friend ostream& operator << (ostream &out,const hunker &obj) {
 	    out << obj.v ;
	    return out ;
	} ;
} ;

class junker {
	int		v = 2 ;
public:
	junker() { } ;
	junker(int av) : v(av) { } ;
	junker &operator = (int av) {
	    v = av ;
	    return (*this) ;
	} ;
	operator int() {
	    cout << "junker::converting_to_int\n" ;
	    return v ;
	} ;
	friend ostream& operator << (ostream &out,const junker &obj) {
 	    out << obj.v ;
	    return out ;
	} ;
} ;

class a {
public:
	int		av = 5 ;
    void something(int v) {
	cout << "a::something-int " << v << endl ;
    }
    void something(double v) {
	cout << "a::something-double " << v << endl ;
    }
    int something(junker &v) {
	cout << "a::something-junker " << v << endl ;
	return 0 ;
    }
    int something(hunker &v) {
	cout << "a::something-hunker " << v << endl ;
	return 0 ;
    }
} ;

class b : public a {
public:
    int something(int v) {
	cout << "b::something-int " << v << endl ;
	return 0 ;
    }
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	hunker		h ;
	junker		j ;
	b		b1 ;
	int		rs = SR_OK ;

	b1.something(3) ;
	b1.something(3.1) ;
	b1.something(j) ;
	// b1.something(h) ;
	cout << b1.av << endl ;
	return 0 ;
}
/* end subroutine (main) */


