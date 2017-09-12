/* sethand */
/* lang=C++98 */

/* SETHAND object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEL		1		/* delete */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object is a container that implements a "set" semantic on
        its entries. The thing that is different is that the entries are
        stored as pointers to the actual entries. This allows for
        entries to be created outside of the container and passed around
        as usch, and then entered into the container as may be needed.


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

extern "C" uint	elfhash(const void *,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */

struct ourless ;

class sethandent {
	int		kl = 0 ;
	int		vl = 0 ;
	const void	*kp = NULL ;
	const void	*vp = NULL ;
public:
	virtual uint	hash(sethandent *) {
	   int		hv = 0 ;
	   if (kp != NULL) elfhash(kp,kl) ;
	   return hv ;
	} ;
	virtual int	cmp(sethandent *,sethandent *) {
	    int		rc = 0 ;
		int		c1l, c2l ;
	        const char	*c1p, *c2p ;
		c1l = 
		rc = strnncmp(c1p,c1l,c2p,c2l) ;
	    }
	    return rc ;
	} ;
	virtual int	getkey(const void **rpp) {
	    if (rpp != NULL) *rpp = (const void *) kp ;
	    return kl ;
	} ;
	virtual int	getval(const void **rpp) {
	    if (rpp != NULL) *rpp = (const void *) vp ;
	    return vl ;
	} ;
	sethandent(const void *kp,int kl,const void *vp,int vl) { } ;
	~sethandent() {
	    kp = NULL ;
	    vp = NULL ;
	    kl = 0 ;
	    vl = 0 ;
	} ;
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


