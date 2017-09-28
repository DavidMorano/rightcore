/* setint */
/* lang=C++11 */

/* unordered set of integers */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* pointer safety */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides a set of integers. No two elements can be the same
        (desired for these purposes).


*******************************************************************************/

#define	SETINT_MASTER	0	/* must to include "extern-C" classification */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<unordered_set>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"setint.h"


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* forward references */


/* local variables */


/* exported subroutines */


int setint_start(setint *op)
{
	unordered_set<int>	*setp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if ((setp = new(nothrow) unordered_set<int>) != NULL) {
	    op->setp = (void *) setp ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (setint_start) */


int setint_finish(setint *op)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    delete setp ;
	    op->setp = NULL ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return rs ;
}
/* end subroutine (setint_finish) */


int setint_addval(setint *op,int v)
{
	int		rs = SR_OK ;
	int		f = INT_MAX ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    pair<unordered_set<int>::iterator,bool>	ret ;
	    ret = setp->insert(v) ;
	    if (ret.second == true) f = 0 ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (setint_addval) */


int setint_delval(setint *op,int v)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    setp->erase(v) ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return rs ;
}
/* end subroutine (setint_delval) */


/* return the count of the number of items in this list */
int setint_count(setint *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    c = setp->size() ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (setint_count) */


/* return the extent of the number of items in this list */
int setint_extent(setint *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    c = setp->max_size() ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (setint_extent) */


int setint_mkvec(setint *op,int *va)
{
	unordered_set<int>	*setp ;
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if ((setp = (unordered_set<int> *) op->setp) != NULL) {
	    if (va != NULL) {
	        unordered_set<int>::iterator it = setp->begin() ;
	        unordered_set<int>::iterator it_end = setp->end() ;
	        while (it != it_end) {
	            va[c++] = *it++ ;
	        } /* end while */
	    } else {
		c = setp->size() ;
	    }
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (setint_mkvec) */


int setint_curbegin(setint *op,setint_cur *curp)
{
	unordered_set<int>::iterator	*interp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((interp = new(nothrow) unordered_set<int>::iterator) != NULL) {
	    unordered_set<int>	*setp = (unordered_set<int> *) op->setp ;
	    *interp = setp->begin() ;
	    curp->interp = (void *) interp ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (setint_curbegin) */


int setint_curend(setint *op,setint_cur *curp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    unordered_set<int>::iterator *interp = 
		(unordered_set<int>::iterator *) curp->interp ;
	    delete interp ;
	    curp->interp = NULL ;
	} else {
	    rs = SR_BUGCHECK ;
	}
	return rs ;
}
/* end subroutine (setint_curend) */


int setint_enum(setint *op,setint_cur *curp,int *rp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    unordered_set<int> *setp = (unordered_set<int> *) op->setp ;
	    unordered_set<int>::iterator it_end ;
	    unordered_set<int>::iterator *interp = 
		(unordered_set<int>::iterator *) curp->interp ;
	    it_end = setp->end() ;
	    if (*interp != it_end) {
	        *rp = *(*interp) ;
	        (*interp)++ ;
	    } else {
		rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}
	return rs ;
}
/* end subroutine (setint_enum) */


