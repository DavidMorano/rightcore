/* langstate */
/* lang=C++98 */

/* Language (parse) State (object) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

	= 2017-10-19, David A­D­ Morano
	Enhanced to ignore characters within character literals.

*/

/* Copyright © 2016,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We track the parse state of C-language type input.


*******************************************************************************/


#define	LANGSTATE_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>
#include	<new>
#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"langstate.h"


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


int langstate_start(LANGSTATE *op)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(LANGSTATE)) ;
	op->f.clear = TRUE ;
	op->magic = LANGSTATE_MAGIC ;
	return SR_OK ;
}
/* end subroutine (langstate_start) */


int langstate_finish(LANGSTATE *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != LANGSTATE_MAGIC) return SR_NOTOPEN ;
	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (langstate_finish) */


int langstate_proc(LANGSTATE *op,int ln,int ch)
{
	int		f ;

#if	CF_DEBUGS
	debugprintf("langstate_proc: ent ln=%u ch=%c (%02x)\n",ln,ch,ch) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (op->magic != LANGSTATE_MAGIC) return SR_NOTOPEN ;

	f = op->f.clear ;
	if (op->f.comment) {
	    if ((op->pch == '*') && (ch == '/')) {
		op->f.comment = FALSE ;
		op->f.clear = TRUE ;
		op->line = ln ;
	    }
	} else if (op->f.quote) {
	    if (op->f.skip) {
		op->f.skip = FALSE ;
	    } else {
		if (ch == CH_BSLASH) {
		    op->f.skip = TRUE ;
	        } else if (ch == CH_DQUOTE) {
		    op->f.quote = FALSE ;
		    op->f.clear = TRUE ;
		    op->line = ln ;
		}
	    }
	} else if (op->f.literal) {
	    if (op->f.skip) {
		op->f.skip = FALSE ;
	    } else {
		if (ch == CH_BSLASH) {
		    op->f.skip = TRUE ;
	        } else if (ch == CH_SQUOTE) {
		    op->f.literal = FALSE ;
		    op->f.clear = TRUE ;
		    op->line = ln ;
		}
	    }
	} else {
	    switch (ch) {
	    case '*':
		if (op->pch == '/') {
		    op->f.comment = TRUE ;
		    op->f.clear = FALSE ;
		    op->line = 0 ;
		    f = FALSE ;
		}
		break ;
	    case CH_DQUOTE:
		if ((op->pch != CH_BSLASH) && (op->pch != CH_SQUOTE)) {
		    op->f.quote = TRUE ;
		    op->f.clear = FALSE ;
		    op->line = 0 ;
		    f = FALSE ;
		}
		break ;
	    case CH_SQUOTE:
		if (op->pch != CH_BSLASH) {
		    op->f.literal = TRUE ;
		    op->f.clear = FALSE ;
		    op->line = 0 ;
		    f = FALSE ;
		}
		break ;
	    } /* end switch */
	} /* end if */
	op->pch = ch ;

#if	CF_DEBUGS
	debugprintf("langstate_proc: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (langstate_proc) */


