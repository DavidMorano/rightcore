/* numincr */

/* number incrementing operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_VSTRSORT	0		/* heap sort instead of quick-sort? */


/* revision history:

	- 1998-12-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object procides a "number incrementing" operation.


*******************************************************************************/


#define	NUMINCR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<cfdec.h>
#include	<cfb26.h>
#include	<ctdec.h>
#include	<ctdecp.h>
#include	<ctb26.h>
#include	<toxc.h>
#include	<localmisc.h>

#include	"numincr.h"


/* local defines */


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		numincr_load(NUMINCR *op,cchar *s,int) ;


/* local variables */


/* exported subroutines */


int numincr_start(NUMINCR *op,cchar *sp,int sl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(NUMINCR)) ;
	op->prec = 1 ;
	rs = numincr_load(op,sp,sl) ;

	return rs ;
}
/* end subroutine (numincr_start) */


int numincr_finish(NUMINCR *op)
{

	if (op == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (numincr_finish) */


int numincr_load(NUMINCR *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		ch ;
	int		v = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl= strlen(sp) ;

	if (op->prec < sl) op->prec = sl ;

	ch = sp[0] ;
	if (isdigitlatin(ch)) {
	    rs = cfdeci(sp,sl,&v) ;
	    op->v = v ;
	} else if (isalphalatin(ch)) {
	    op->f.alpha = TRUE ;
	    op->f.uc = touc(ch) ;
	    rs = cfb26i(sp,sl,&v) ;
	    op->v = v ;
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (numincr_load) */


int numincr_setprec(NUMINCR *op,int prec)
{

	if (op == NULL) return SR_FAULT ;

	if (prec < 1) prec = 1 ;

	if (prec > NUMINCR_MAXPREC) prec = NUMINCR_MAXPREC ;

	if (prec > op->prec) op->prec = prec ;

	return SR_OK ;
}
/* end subroutine (numincr_setprec) */


int numincr_incr(NUMINCR *op,int incr)
{

	if (op == NULL) return SR_FAULT ;

	op->v += incr ;
	return SR_OK ;
}
/* end subroutine (numincr_incr) */


int numincr_cvtstr(NUMINCR *op,char *rbuf,int rlen,int prec)
{
	int		rs = SR_OK ;
	int		type ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->f.alpha) {
	    type = (op->f.uc) ? 'A' : 'a' ;
	    rs = ctb26i(rbuf,rlen,type,op->prec,op->v) ;
	} else {
	    rs = ctdecpi(rbuf,rlen,op->prec,op->v) ;
	} /* end if */

	return rs ;
}
/* end subroutine (numincr_cvtstr) */


