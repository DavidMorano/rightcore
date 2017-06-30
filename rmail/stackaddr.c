/* stackaddr */

/* stack-address management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage stack-addresses.

        We needed something like this because SBUF and BUFFER to not have the
        back-up facility that we need.

	Synopsis:

	stackaddr_start(op,abuf,alen)
	STACKADDR	*op ;
	char		abuf[] ;
	int		alen ;

	Arguments:

	op		pointer to the STACKADDR object
	abuf		result buffer to stack-address
	alen		length of result buffer

	Returns:

	>=0		accummlated length
	<0		error


*******************************************************************************/


#define	STACKADDR_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"stackaddr.h"


/* local defines */


/* external subroutines */

extern int	strwcmp(const char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int stackaddr_start(STACKADDR *op,char *dbuf,int dlen)
{

	if (op == NULL) return SR_FAULT ;
	if (dbuf == NULL) return SR_FAULT ;

	dbuf[0] = '\0' ;
	memset(op,0,sizeof(STACKADDR)) ;
	op->dbuf = dbuf ;
	op->dlen = dlen ;

	return SR_OK ;
}
/* end subroutine (stackaddr_start) */


int stackaddr_finish(STACKADDR *op)
{
	if (op == NULL) return SR_FAULT ;
	return (op->i) ;
}
/* end subroutine (stackaddr_finish) */


int stackaddr_add(STACKADDR *op,cchar *hp,int hl,cchar *up,int ul)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

	if (op->i < 0) return (op->i) ;

	if ((rs >= 0) && (op->i > 0)) {
	    op->i = op->ri ;
	}

	if ((rs >= 0) && (hp != NULL) && (hp[0] != '\0')) {
	    NULSTR	h ;
	    const char	*nhp ;
	    if (hl < 0) hl = strlen(hp) ;
	    if ((rs = nulstr_start(&h,hp,hl,&nhp)) >= 0) {
		const char	*lhp = op->lhp ;
		int		lhl = op->lhl ;

#if	CF_DEBUGS
		debugprintf("stackaddr_add: lhl=%u\n",lhl) ;
		debugprintf("stackaddr_add: lh=%t\n",lhp,lhl) ;
		debugprintf("stackaddr_add: nh=%t\n",hp,hl) ;
#endif

	        if ((lhp == NULL) || (strwcmp(nhp,lhp,lhl) != 0)) {

#if	CF_DEBUGS
		    debugprintf("stackaddr_add: adding\n") ;
#endif

	            if (rs >= 0) {
		        op->lhp = (op->dbuf + op->i) ;
		        op->lhl = hl ;
	                rs = storebuf_strw(op->dbuf,op->dlen,op->i,hp,hl) ;
	                op->i += rs ;
	                len += rs ;
	            }

	            if (rs >= 0) {
	                rs = storebuf_char(op->dbuf,op->dlen,op->i,'!') ;
	                op->i += rs ;
	                len += rs ;
	                op->ri = op->i ;
	            }

		} /* end if (a new different host) */

		rs1 = nulstr_finish(&h) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (nulstr) */
	} /* end if (had a host) */

	if (rs >= 0) {
	    rs = storebuf_strw(op->dbuf,op->dlen,op->i,up,ul) ;
	    op->i += rs ;
	    len += rs ;
	}

	if (rs < 0) op->i = rs ;

#if	CF_DEBUGS
	debugprintf("stackaddr_add: sa=%t\n",op->dbuf,op->i) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (stackaddr_add) */


