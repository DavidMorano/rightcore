/* outstore */

/* manage some dynamic output storage (mostly for SHIO use) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages a small buffer for output storage purposes.  This
	is mostly (or perhaps only) used as a helper object for the SHIO object
	when it is:

	1. in SFIO mode
	2. the output is a terminal

	Othersize, SHIO just outputs data using the regular calls.


*******************************************************************************/


#define	OUTSTORE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"outstore.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,cchar *,int) ;


/* exported subroutines */


int outstore_start(OUTSTORE *op)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(OUTSTORE)) ;
	return SR_OK ;
}
/* end subroutine (outstore_start) */


int outstore_finish(OUTSTORE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (op->dbuf != NULL) {
	    op->dbuf[0] = '\0' ; /* cute safety trick */
	    rs1 = uc_free(op->dbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbuf = NULL ;
	}
	op->sbuf[0] = '\0' ;
	op->len = 0 ;
	return rs ;
}
/* end subroutine (outstore_finish) */


int outstore_get(OUTSTORE *op,cchar **rpp)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	rs = op->len ;
	if (rpp != NULL) {
	    *rpp = (op->dbuf != NULL) ? op->dbuf : op->sbuf ;
	}
	return rs ;
}
/* end subroutine (outstore_get) */


int outstore_clear(OUTSTORE *op)
{
	if (op == NULL) return SR_FAULT ;
	op->len = 0 ;
	return SR_OK ;
}
/* end subroutine (outstore_clear) */


int outstore_strw(OUTSTORE *op,cchar *sp,int sl)
{
	const int	slen = OUTSTORE_SLEN ;
	int		rs = SR_OK ;
	int		rlen ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	if (op->dbuf != NULL) {
	    rlen = (op->dlen-op->len) ;
	    if (sl > rlen) {
		const int	dlen = (op->dlen+sl+slen) ;
		char		*dp ;
		if ((rs = uc_realloc(op->dbuf,(dlen+1),&dp)) >= 0) {
		    char	*rp = (dp+op->len) ;
		    op->dbuf = dp ;
		    op->dlen = dlen ;
		    rs = (strwcpy(rp,sp,sl) - rp) ;
		    op->len += rs ;
		}
	    } else {
		char	*rp = (op->dbuf+op->len) ;
		rs = (strwcpy(rp,sp,sl) - rp) ;
		op->len += rs ;
	    }
	} else {
	    rlen = (slen-op->len) ;
	    if (sl > rlen) {
		const int	dlen = MAX((sl+slen),(2*slen)) ;
		char		*dp ;
		if ((rs = uc_malloc((dlen+1),&dp)) >= 0) {
		    op->dlen = dlen ;
		    op->dbuf = dp ;
		    dp = strwcpy(dp,op->sbuf,op->len) ;
		    rs = (strwcpy(dp,sp,sl) - dp) ;
		    op->len += rs ;
		}
	    } else {
		char	*rp = (op->sbuf+op->len) ;
		rs = (strwcpy(rp,sp,sl) - rp) ;
		op->len += rs ;
	    }
	} /* end if (current mode) */
	return rs ;
}
/* end subroutine (outstore_strw) */


