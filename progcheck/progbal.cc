/* progbal */
/* lang=C++98 */

/* program character balance */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We determine if the character balance in the input (given a piece at
	a time) is correct or not.

		progbal_start
		progbal_load
		progbal_finish


*******************************************************************************/


#define	PROGBAL_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"progbal.h"


/* local defines */


/* namespaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	sichar(cchar *,int,int) ;

extern "C" int	hexval(int) ;
extern "C" int	isprintlatin(int) ;
extern "C" int	ishexlatin(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static cchar	chopen[] = { CH_LPAREN, CH_LBRACE, CH_LBRACK, '\0' } ;
static cchar	chclose[] = { CH_RPAREN, CH_RBRACE, CH_RBRACK, '\0' } ;


/* exported subroutines */


int progbal_start(PROGBAL *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(PROGBAL)) ;

	return rs ;
}
/* end subroutine (progbal_start) */


/* returns if all of the input was balanced */
int progbal_finish(PROGBAL *op)
{
	int		rs = SR_OK ;
	int		f_bal ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PROGBAL_MAGIC) return SR_NOTOPEN ;

	f_bal = (!op->f_fail) ;
	if (f_bal) {
	    int	i ;
	    for (i = 0 ; i < PROGBAL_NCH ; i += 1) {
		f_bal = (op->counts[i] == 0) ;
		if (!f_bal) break ;
	    } /* end for */
	}

	op->magic = 0 ;
	return (rs >= 0) ? f_bal : rs ;
}
/* end subroutine (progbal_finish) */


int progbal_load(PROGBAL *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f_fail = FALSE ;

#if	CF_DEBUGS
	debugprintf("progbal_load: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != PROGBAL_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	while (sl-- && *sp) {
	    const int	ch = MKCHAR(*sp++) ;
	    int		w ;
	    switch (ch) {
	    case CH_LPAREN:
	    case CH_LBRACE:
	    case CH_LBRACK:
		w = sichar(chopen,-1,ch) ;
		op->counts[w] += 1 ;
		break ;
	    case CH_RPAREN:
	    case CH_RBRACE:
	    case CH_RBRACK:
		w = sichar(chclose,-1,ch) ;
		if (op->counts[w] > 0) {
		    op->counts[w] -= 1 ;
		} else {
		    f_fail = TRUE ;
		}
		break ;
	    } /* end switch */
	    if (f_fail) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (progbal_load) */


/* private subroutines */


