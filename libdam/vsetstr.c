/* vsetstr */

/* Vector-Implemented Ordered-Set-String object */


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


#define	VSETSTR_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vsetstr.h"


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


int vsetstr_start(VSETSTR *op,int n)
{
	int		csize = 0 ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (n > 0) csize = (n*6) ;
	if ((rs = vecpstr_start(&op->ents,n,csize,0)) >= 0) {
	    op->magic = VSETSTR_MAGIC ;
	}

	return rs ;
}
/* end subroutine (vsetstr_start) */


int vsetstr_finish(VSETSTR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	rs1 = vecpstr_finish(&op->ents) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (vsetstr_finish) */


int vsetstr_look(VSETSTR *op,cchar *sbuf,int slen)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	rs = vecpstr_already(&op->ents,sbuf,slen) ;

	return rs ;
}
/* end subroutine (vsetstr_look) */


/* add a string to the database */
int vsetstr_add(VSETSTR *op,cchar *sbuf,int slen)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (sbuf == NULL) return SR_FAULT ;

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	if (slen < 0) slen = strlen(sbuf) ;

	if ((rs = vecpstr_findn(&op->ents,sbuf,slen)) == rsn) {
	    rs = vecpstr_add(&op->ents,sbuf,slen) ;
	} else if (rs >= 0)
	    rs = INT_MAX ;

#if	CF_DEBUGS
	debugprintf("vsetstr_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (vsetstr_add) */


int vsetstr_curbegin(VSETSTR *op,VSETSTR_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (vsetstr_curbegin) */


int vsetstr_curend(VSETSTR *op,VSETSTR_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (vsetstr_curend) */


int vsetstr_curdel(VSETSTR *op,VSETSTR_CUR *curp)
{
	int		rs ;
	int		i ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	i = curp->i ;
	rs = vecpstr_del(&op->ents,i) ;

	return rs ;
}
/* end subroutine (vsetstr_curdel) */


int vsetstr_already(VSETSTR *op,cchar *sp,int sl)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecpstr_findn(&op->ents,sp,sl)) >= 0) {
	    rs = TRUE ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (vsetstr_already) */


/* enumerate all of the entries */
int vsetstr_enum(VSETSTR *op,VSETSTR_CUR *curp,cchar **vpp)
{
	int		rs ;
	int		i ;
	int		rl = 0 ;
	const char	*rp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (vsetstr_enum) */


/* advance the cursor to the next entry regardless of key */
int vsetstr_next(VSETSTR *op,VSETSTR_CUR *curp)
{
	int		rs = SR_OK ;
	int		i ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VSETSTR_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	curp->i = i ;

	return rs ;
}
/* end subroutine (vsetstr_next) */


int vsetstr_count(VSETSTR *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	rs = vecpstr_count(&op->ents) ;

	return rs ;
}
/* end subroutine (vsetstr_count) */


int vsetstr_extent(VSETSTR *op)
{
	return vsetstr_count(op) ;
}
/* end subroutine (vsetstr_extent) */


