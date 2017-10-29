/* osetstr */

/* Statis-Sset-String object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1		/* safety */


/* revision history:

	= 1998-03-12, David A­D­ Morano

	This module was originally written (from scratch).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module provides operations and management on a static set of
	strings.


*******************************************************************************/


#define	OSETSTR_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"osetstr.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int osetstr_start(OSETSTR *op,int n)
{
	int		csize = 0 ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (n > 0) csize = (n*6) ;
	if ((rs = vecpstr_start(&op->ents,n,csize,0)) >= 0) {
	    op->magic = OSETSTR_MAGIC ;
	}

	return rs ;
}
/* end subroutine (osetstr_start) */


int osetstr_finish(OSETSTR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	rs1 = vecpstr_finish(&op->ents) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (osetstr_finish) */


int osetstr_look(OSETSTR *op,cchar *sbuf,int slen)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	rs = vecpstr_already(&op->ents,sbuf,slen) ;

	return rs ;
}
/* end subroutine (osetstr_look) */


/* add a string pair to the database */
int osetstr_add(OSETSTR *op,cchar *sbuf,int slen)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (sbuf == NULL) return SR_FAULT ;

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	if (slen < 0) slen = strlen(sbuf) ;

	if ((rs = vecpstr_findn(&op->ents,sbuf,slen)) == nrs) {
	    rs = vecpstr_add(&op->ents,sbuf,slen) ;
	} else if (rs >= 0)
	    rs = INT_MAX ;

#if	CF_DEBUGS
	debugprintf("osetstr_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (osetstr_add) */


int osetstr_curbegin(OSETSTR *op,OSETSTR_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (osetstr_curbegin) */


int osetstr_curend(OSETSTR *op,OSETSTR_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (osetstr_curend) */


int osetstr_curdel(OSETSTR *op,OSETSTR_CUR *curp)
{
	int		rs ;
	int		i ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	    i = curp->i ;
	    rs = vecpstr_del(&op->ents,i) ;

	return rs ;
}
/* end subroutine (osetstr_curdel) */


int osetstr_findn(OSETSTR *op,cchar *sp,int sl)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	return vecpstr_findn(&op->ents,sp,sl) ;
}
/* end subroutine (osetstr_findn) */


/* enumerate all of the entries */
int osetstr_enum(OSETSTR *op,OSETSTR_CUR *curp,cchar **vpp)
{
	int		rs ;
	int		i ;
	int		rl = 0 ;
	const char	*rp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs = vecpstr_get(&op->ents,i,&rp)) >= 0) {
	    if (rp != NULL) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {
	    curp->i = i ;
	    rl = strlen(rp) ;
	}

	if (vpp != NULL) {
	    *vpp = (rs >= 0) ? rp : NULL ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (osetstr_enum) */


/* advance the cursor to the next entry regardless of key */
int osetstr_next(OSETSTR *op,OSETSTR_CUR *curp)
{
	int		rs = SR_OK ;
	int		i ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (op->magic != OSETSTR_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	curp->i = i ;

	return rs ;
}
/* end subroutine (osetstr_next) */


int osetstr_count(OSETSTR *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	rs = vecpstr_count(&op->ents) ;

	return rs ;
}
/* end subroutine (osetstr_count) */


