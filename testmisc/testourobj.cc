/* testset */
/* lang=C++98 */

/* test the SET object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEL		1		/* delete */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform some tests on the SET (container) object.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<set>
#include	<string>
#include	<iostream>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugopen(cchar *) ;
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */

struct ourless ;

class ourobj {
	int		v ;
public:
	ourobj(int av = 0) : v(av) {
	    cout << "ourobj val-construct " << v << endl ;
	} ;
	ourobj(const ourobj &oo) {
	    v = oo.v ;
	    cout << "ourobj copy-construct " << v << endl ;
	} ;
	~ourobj() {
	    cout << "ourobj destruct " << v << endl ;
	} ;
	int get() const {
	    return v ;
	} ;
	ourobj& operator = (const ourobj &oo) {
	    v += oo.v ;
	    return (*this) ;
	} ;
	ourobj& operator += (const ourobj &oo) {
	    v += oo.v ;
	    return (*this) ;
	} ;
	friend ostream& operator << (ostream &os,const ourobj &oo) {
	    os << oo.v ;
	    return os ;
	} ;
	bool operator () (const ourobj &o1,const ourobj &o2) const {
	    return (o1.v < o2.v) ;
	} ;
	friend bool operator < (const ourobj &o1,const ourobj &o2) {
	    return (o1.v < o2.v) ;
	} ;
	friend bool operator > (const ourobj &o1,const ourobj &o2) {
	    return (o1.v > o2.v) ;
	} ;
	friend bool operator == (const ourobj &o1,const ourobj &o2) {
	    return (o1.v == o2.v) ;
	} ;
	friend struct ourless ;
} ;

struct ourless {
	bool operator () (const ourobj &o1,const ourobj &o2) const {
	    return (o1.v < o2.v) ;
	} ;
} ;

template <class T>
struct genless {
	bool operator () (const T &o1,const T &o2) const {
	    int	v1 = o1.get() ;
	    int	v2 = o2.get() ;
	    return (v1 < v2) ;
	} ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	set<ourobj>	os ;
	int		c = 0 ;

	{
	    ourobj	ov(1) ;
	    os.insert(ov) ;
	}

#if	CF_DEL
	{
	    set<ourobj>::iterator	it, end = os.end() ;
	    ourobj	v(1) ;
	    if ((it = os.find(v)) != end) {
		os.erase(it) ;
	    }
	}
#endif /* CF_DEL */

	c = os.size() ;
	cout << "count " << c << endl ;

	return 0 ;
}
/* end subroutine (main) */


