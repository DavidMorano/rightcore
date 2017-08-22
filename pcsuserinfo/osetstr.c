/* osetstr */
/* lang=C++98 */

/* ordered set of strings */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides am ordered set of strings.  No two strings can
	be the same (desired for these purposes).


*******************************************************************************/

#define	OSETSTR_MASTER	0	/* must to include "extern-C" classification */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<utility>
#include	<set>
#include	<string>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"osetstr.h"


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int osetstr_start(osetstr *op,int n)
{
	set<string>	*setp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (n < 0) n = 0 ;
	if ((setp = new(nothrow) set<string>) != NULL) {
	    op->setp = (void *) setp ;
	} else {
	    rs = SR_NOMEM ;
	}
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (osetstr_start) */


int osetstr_finish(osetstr *op)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<string>	*setp  = (set<string> *) op->setp ;
	    delete setp ;
	    op->setp = NULL ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return rs ;
}
/* end subroutine (osetstr_finish) */


int osetstr_already(osetstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f = TRUE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
#if	CF_DEBUGS
	debugprintf("osetstr_already: ent s=%t\n",sp,sl) ;
#endif
	if (op->setp != NULL) {
	    set<string>	*setp  = (set<string> *) op->setp ;
	    string	*strp ;
	    if ((strp = new(nothrow) string(sp,sl)) != NULL) {
	        set<string>::iterator it_end = setp->end() ;
	        set<string>::iterator it ;
	        if ((it = setp->find(*strp)) == it_end) {
	            f = FALSE ;
		}
		delete strp ;
	    } else {
		rs = SR_NOMEM ;
	    }
	} else {
	    rs = SR_NOTOPEN ;
	}
#if	CF_DEBUGS
	debugprintf("osetstr_already: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (osetstr_already) */


int osetstr_add(osetstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
#if	CF_DEBUGS
	debugprintf("osetstr_add: ent s=%t\n",sp,sl) ;
#endif
	if (op->setp != NULL) {
	    set<string>	*setp  = (set<string> *) op->setp ;
	    pair<set<string>::iterator,bool>	ret ;
	    string	v(sp,sl) ;
	    ret = setp->insert(v) ;
	    f = (ret.second == false) ;
	} else {
	    rs = SR_NOTOPEN ;
	}
#if	CF_DEBUGS
	debugprintf("osetstr_add: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (osetstr_add) */


int osetstr_del(osetstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	if (op->setp != NULL) {
	    set<string>			*setp  = (set<string> *) op->setp ;
	    set<string>::iterator	it, end = setp->end() ;
	    string			v(sp,sl) ;
	    if ((it = setp->find(v)) != end) {
		f = TRUE ;
		setp->erase(it) ;
	    }
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (osetstr_del) */


/* return the count of the number of items in this list */
int osetstr_count(osetstr *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->setp != NULL) {
	    set<string>	*setp  = (set<string> *) op->setp ;
	    c = setp->size() ;
	} else {
	    rs = SR_NOTOPEN ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (osetstr_count) */


int osetstr_curbegin(osetstr *op,osetstr_cur *curp)
{
	set<string>::iterator	*interp ;
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((interp = new(nothrow) set<string>::iterator) != NULL) {
	    set<string>	*setp  = (set<string> *) op->setp ;
	    *interp = setp->begin() ;
	    curp->interp = (void *) interp ;
	} else {
	    rs = SR_NOMEM ;
	}
	return rs ;
}
/* end subroutine (osetstr_curbegin) */


int osetstr_curend(osetstr *op,osetstr_cur *curp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    set<string>::iterator *interp = 
		(set<string>::iterator *) curp->interp ;
	    delete interp ;
	    curp->interp = NULL ;
	} else {
	    rs = SR_BUGCHECK ;
	}
	return rs ;
}
/* end subroutine (osetstr_curend) */


int osetstr_enum(osetstr *op,osetstr_cur *curp,cchar **rpp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	if (curp->interp != NULL) {
	    set<string>		*setp  = (set<string> *) op->setp ;
	    set<string>::iterator it_end ;
	    set<string>::iterator *interp = 
			(set<string>::iterator *) curp->interp ;
	    it_end = setp->end() ;
	    if (*interp != it_end) {
		*rpp = (*(*interp)).c_str() ;
		rs = (*(*interp)).length() ;
	        (*interp)++ ;
	    } else {
		rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}
	return rs ;
}
/* end subroutine (osetstr_enum) */


