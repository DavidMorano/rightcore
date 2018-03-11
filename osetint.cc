/* osetint */
/* lang=C++98 */

/* ordered set of integers */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* pointer safety */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides am ordered set of integers.  No two elements can
	be the same (desired for these purposes).

	= Implementation

	We use the C++ STL |set| container object and let the comparison
	object default to |less|.


*******************************************************************************/

#define	OSETINT_MASTER	0	/* must to include "extern-C" classification */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<new>
#include	<utility>
#include	<set>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"osetint.h"


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* forward references */


/* local variables */


/* exported subroutines */


int osetint_start(osetint *op)
{
	set<int>	*setp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if ((setp = new(nothrow) set<int>) != NULL) {
	    op->setp = (void *) setp ;
	} else
	    rs = SR_NOMEM ;
	return rs ;
}
/* end subroutine (osetint_start) */


int osetint_finish(osetint *op)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    delete setp ;
	    op->setp = NULL ;
	} else
	    rs = SR_NOTOPEN ;
	return rs ;
}
/* end subroutine (osetint_finish) */


int osetint_addval(osetint *op,int v)
{
	int		rs = SR_OK ;
	int		f = INT_MAX ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    pair<set<int>::iterator,bool>	ret ;
	    ret = setp->insert(v) ;
	    if (ret.second == true) f = 0 ;
	} else
	    rs = SR_NOTOPEN ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (osetint_addval) */


int osetint_delval(osetint *op,int v)
{
	set<int>	*setp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    setp->erase(v) ;
	} else
	    rs = SR_NOTOPEN ;
	return rs ;
}
/* end subroutine (osetint_delval) */


/* return the count of the number of items in this list */
int osetint_count(osetint *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    c = setp->size() ;
	} else
	    rs = SR_NOTOPEN ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (osetint_count) */


/* return the extent of the number of items in this list */
int osetint_extent(osetint *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    c = setp->max_size() ;
	} else
	    rs = SR_NOTOPEN ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (osetint_extent) */


int osetint_mkvec(osetint *op,int *va)
{
	set<int>	*setp ;
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if ((setp = (set<int> *) op->setp) != NULL) {
	    if (va != NULL) {
	        set<int>::iterator it = setp->begin() ;
	        set<int>::iterator it_end = setp->end() ;
	        while (it != it_end) {
	            va[c++] = *it++ ;
	        } /* end while */
	    } else {
		c = setp->size() ;
	    }
	} else
	    rs = SR_NOTOPEN ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (osetint_mkvec) */


int osetint_curbegin(osetint *op,osetint_cur *curp)
{
	set<int>::iterator	*interp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((interp = new(nothrow) set<int>::iterator) != NULL) {
	    set<int>	*setp  = (set<int> *) op->setp ;
	    *interp = setp->begin() ;
	    curp->interp = (void *) interp ;
	} else
	    rs = SR_NOMEM ;
	return rs ;
}
/* end subroutine (osetint_curbegin) */


int osetint_curend(osetint *op,osetint_cur *curp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    set<int>::iterator *interp = (set<int>::iterator *) curp->interp ;
	    delete interp ;
	    curp->interp = NULL ;
	} else
	    rs = SR_BUGCHECK ;
	return rs ;
}
/* end subroutine (osetint_curend) */


int osetint_enum(osetint *op,osetint_cur *curp,int *rp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    set<int>		*setp  = (set<int> *) op->setp ;
	    set<int>::iterator it_end ;
	    set<int>::iterator *interp = (set<int>::iterator *) curp->interp ;
	    it_end = setp->end() ;
	    if (*interp != it_end) {
	        *rp = *(*interp) ;
	        (*interp)++ ;
	    } else
		rs = SR_NOTFOUND ;
	} else
	    rs = SR_BUGCHECK ;
	return rs ;
}
/* end subroutine (osetint_enum) */


