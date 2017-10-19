/* linehist */
/* lang=C++98 */

/* Line History (object) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Process characters (a line at a time) for balanced pairs. We record line
        numbers so that when we are left with some sort of unbalance, we can
        report the associated line numbers.

		linehist_start
		linehist_proc
		linehist_count
		linehist_get
		linehist_finish


*******************************************************************************/


#define	LINEHIST_MASTER		0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"linehist.h"


/* local defines */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */

class item {
	int		ln ;		/* line number */
	int		it ;		/* 0=opening, 1=closing */
public:
	item(int aln,int ait) : ln(aln), it(ait) { } ;
	int type() const { return it ; } ;
	int line() const { return ln ; } ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int linehist_start(LINEHIST *op,cchar *ss)
{
	int		rs = SR_OK ;
	vector<item>	*lvp ;

	if (op == NULL) return SR_FAULT ;
	if (ss == NULL) return SR_FAULT ;

	if (ss[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(LINEHIST)) ;

	if ((lvp = new(nothrow) vector<item>) != NULL) {
	    op->lvp = (void *) lvp ;
	    if ((rs = langstate_start(&op->ls)) >= 0) {
	        strncpy(op->ss,ss,2) ;
	        op->magic = LINEHIST_MAGIC ;
	    } /* end if (langstate_start) */
	    if (rs < 0) {
		delete lvp ;
		op->lvp = NULL ;
	    }
	} else {
	    rs = SR_NOMEM ;
	}

	return rs ;
}
/* end subroutine (linehist_start) */


int linehist_finish(LINEHIST *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEHIST_MAGIC) return SR_NOTOPEN ;

	if (op->lvp != NULL) {
	    vector<item>	*lvp = (vector<item> *) op->lvp ;
	    delete lvp ;
	    op->lvp = NULL ;
	}

	rs1 = langstate_finish(&op->ls) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (linehist_finish) */


int linehist_proc(LINEHIST *op,int ln,cchar *sp,int sl)
{
	vector<item>	*lvp ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("linehist_proc: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != LINEHIST_MAGIC) return SR_NOTOPEN ;

	if ((lvp = ((vector<item> *) op->lvp)) != NULL) {
	    const int	sch0 = MKCHAR(op->ss[0]) ;
	    const int	sch1 = MKCHAR(op->ss[1]) ;
	    int		ch ;
	    while ((rs >= 0) && sl && *sp) {
		ch = MKCHAR(*sp) ;
		if ((rs = langstate_proc(&op->ls,ln,ch)) > 0) {
		    if (ch == sch0) {
		        item	a(ln,0) ;
		        lvp->push_back(a) ;
		    } else if (ch == sch1) {
		        int	f = TRUE ;
		        if (lvp->size() > 0) {
			    const item	li = lvp->back() ;
			    if (li.type() == 0) {
			        f = FALSE ;
			        lvp->pop_back() ;
			    }
		        }
		        if (f) {
		            item	a(ln,1) ;
		            lvp->push_back(a) ;
		        }
		    } /* end if */
		} /* end if (langstate_proc) */
		sp += 1 ;
		sl -= 1 ;
	    } /* end while */
	    c = lvp->size() ;
	} else {
	    rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linehist_proc) */


int linehist_count(LINEHIST *op)
{
	vector<item>	*lvp ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEHIST_MAGIC) return SR_NOTOPEN ;

	if ((lvp = ((vector<item> *) op->lvp)) != NULL) {
	    c = lvp->size() ;
	} else {
	    rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (linehist_count) */


int linehist_get(LINEHIST *op,int i,int *lnp)
{
	vector<item>	*lvp ;
	int		rs = SR_OK ;
	int		type = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lnp == NULL) return SR_FAULT ;

	if (op->magic != LINEHIST_MAGIC) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

	if ((lvp = ((vector<item> *) op->lvp)) != NULL) {
	    const int	len = lvp->size() ;
	    if (i < len) {
	        item	vi = lvp->at(i) ;
		type = (vi.type()+1) ;
		if (lnp != NULL) {
		    *lnp = vi.line() ;
		}
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? type : rs ;
}
/* end subroutine (linehist_get) */


