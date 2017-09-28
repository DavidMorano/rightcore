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

        This object is a container that implements a "set" semantic on its
        entries. The thing that is different is that the entries are stored as
        pointers to the actual entries. This allows for entries to be created
        outside of the container and passed around as usch, and then entered
        into the container as may be needed.


*******************************************************************************/

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<utility>
#include	<functional>
#include	<set>
#include	<string>
#include	<stdexcept>
#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"returnstatus.h"


/* local defines */

#define	SETHAND_DEFENTS	10


/* default name spaces */

using namespace	std ;


/* external subroutines */

extern "C" uint	elfhash(const void *,int) ;

extern "C" int	strnncmp(cchar *,int,cchar *,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */

class sethandent {
	int		osize = 0 ;
protected:
	int		kl = 0 ;
	int		vl = 0 ;
	const void	*kp = NULL ;
	const void	*vp = NULL ;
public:
	virtual int	getkey(const void **rpp) const {
	    if (rpp != NULL) *rpp = (const void *) kp ;
	    return kl ;
	} ;
	virtual int	getval(const void **rpp) const {
	    if (rpp != NULL) *rpp = (const void *) vp ;
	    return vl ;
	} ;
	virtual uint	hash() {
	   int		hv = 0 ;
	   if (kp != NULL) hv = elfhash(kp,kl) ;
	   return hv ;
	} ;
	virtual int	cmp(sethandent *e1p,sethandent *e2p) {
	    int		rc = 0 ;
	    int		k1l, k2l ;
	    const void	*v1p, *v2p ;
	    if ((k1l = e1p->getkey(&v1p)) >= 0) {
	        if ((k2l = e2p->getkey(&v2p)) >= 0) {
	    	    cchar	*c1p = (cchar *) v1p ;
	    	    cchar	*c2p = (cchar *) v2p ;
		    rc = strnncmp(c1p,k1l,c2p,k2l) ;
		}
	    }
	    return rc ;
	} ;
	virtual int	setosize(int aosize) {
	    osize = aosize ;
	    return SR_OK ;
	} ;
	virtual int	getosize() const {
	    return osize ;
	} ;
	sethandent() { } ;
	sethandent(const void *akp,int akl,const void *avp,int avl) :
	    kl(akl), vl(avl), kp(akp), vp(avp) {
	} ;
	~sethandent() {
	    kp = NULL ;
	    vp = NULL ;
	    kl = 0 ;
	    vl = 0 ;
	} ;
} ;
/* end class (sethandent) */

class sethand ;

class sethand_it {
	HDB_CUR		cur ;
	friend		sethand ;
} ;

class sethand {
	hdb		list ;
	int		f_started = FALSE ;
	int		n = 0 ;
	int		at = 0 ;
public:
	typedef sethand_it iterator ;
	sethand(int an = SETHAND_DEFENTS) : n(an) { 
	} ;
	sethand(int an,int aat) : n(an), at(aat) { 
	} ;
	~sethand() {
	    if (f_started) {
		f_started = FALSE ;
		hdb_finish(&list) ;
	    }
	} ;
	int		begin(int an) {
	    int		rs ;
	    n = an ;
	    if ((rs = hdb_start(&list,n,at,NULL,NULL)) >= 0) {
		f_started = TRUE ;
	    }
	    return rs ;
	} ;
	int		begin() {
	    int		rs ;
	    if ((rs = hdb_start(&list,n,at,NULL,NULL)) >= 0) {
		f_started = TRUE ;
	    }
	    return rs ;
	} ;
	int		end() {
	    int		rs = SR_OK ;
	    if (f_started) {
		f_started = FALSE ;
		rs = hdb_finish(&list) ;
	    }
	    return rs ;
	} ;
	int		itbegin(sethand_it *itp) {
	    return hdb_curbegin(&list,&itp->cur) ;
	} ;
	int		itend(sethand_it *itp) {
	    return hdb_curend(&list,&itp->cur) ;
	} ;
	int itfetch(sethand_it *itp,sethandent **rpp,cchar *kp,int kl) {
	    HDB_DATUM	key, val ;
	    HDB_CUR	*curp = &itp->cur ;
	    int		rs ;
	    key.buf = kp ;
	    key.len = kl ;
	    if ((rs = hdb_fetch(&list,key,curp,&val)) >= 0) {
		if (rpp != NULL) *rpp = (sethandent *) val.buf ;
		rs = val.len ;
	    }
	    return rs ;
	} ;
	int itenum(sethand_it *itp,sethandent **rpp) {
	    HDB_DATUM	key, val ;
	    HDB_CUR	*curp = &itp->cur ;
	    int		rs ;
	    if ((rs = hdb_enum(&list,curp,&key,&val)) >= 0) {
		if (rpp != NULL) *rpp = (sethandent *) val.buf ;
		rs = val.len ;
	    }
	    return rs ;
	} ;
	int add(const sethandent *ep) {
	    const int	os = ep->getosize() ;
	    int		rs = SR_OK ;
	    int		kl ;
	    const void	*kp ;
	    if ((kl = ep->getkey(&kp)) >= 0) {
		HDB_DATUM	key, val ;
		const int	rsn = SR_NOTFOUND ;
		key.buf = kp ;
		key.len = kl ;
		if ((rs = hdb_fetch(&list,key,NULL,&val)) == rsn) {
		    val.buf = ep ;
		    val.len = os ;
	            if ((rs = hdb_store(&list,key,val)) >= 0) {
			rs = 1 ;
		    }
		}
	    }
	    return rs ;
	} ;
	sethand& operator += (const sethandent *ep) {
	    int	rs = add(ep) ;
	    if (rs < 0) {
		returnstatus a(rs) ;
		throw a ;
	    }
	    return (*this) ;
	} ;
} ;
/* end class (sethand) */


