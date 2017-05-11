/* ema_first */

/* get first non-empty E-Mail Address */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get the first non-empty EMA address.


*******************************************************************************/

#define	EMA_MASTER	0	/* we need declaration of |ema_get()| */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ema.h"


/* local defines */

/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif


/* external variables */


/* local structures */


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int ema_first(EMA *op,cchar **rpp)
{
	EMA_ENT		*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		rl = 0 ;
	cchar		*rp = NULL ;;
	for (i = 0 ; (rs = ema_get(op,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
		if ((ep->rp != NULL) || (ep->ap != NULL)) {
		    if (rl == 0) {
			rl = ep->rl ;
			rp = ep->rp ;
		    }
		    if (rl == 0) {
			rl = ep->al ;
			rp = ep->ap ;
		    }
		}
	    }
	    if (rl > 0) break ;
	} /* end for */
	if (rs >= 0) {
	    if (rpp != NULL) {
		*rpp = (rl > 0) ? rp : NULL ;
	    }
	} else if (rs == SR_NOTFOUND) {
	    if (rpp != NULL) *rpp = NULL ;
	    rs = SR_OK ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (ema_first) */


