/* sethand */
/* lang=C99 */

/* set of strings */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides am ordered set of integers.  No two strings can
	be the same (desired for these purposes).


*******************************************************************************/

#define	SETHAND_MASTER	0	/* must to include "extern-C" classification */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"sethand.h"


/* local defines */


/* default name spaces */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int sethand_start(sethand *op,int n,sethandhash_t hf,sethandcmp_t cf)
{
	const int	at = TRUE ;
	return hdb_start(op,n,at,hf,cf) ;
}
/* end subroutine (sethand_start) */


int sethand_finish(sethand *op)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs1 = hdb_curbegin(op,&cur)) >= 0) {
	    int		rs2 ;
	    while ((rs2 = hdb_enum(op,&cur,&key,&val)) >= 0) {
	        if (key.buf != NULL) {
		    c += 1 ;
	            rs1 = uc_free(key.buf) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end while (enum) */
	    if ((rs >= 0) && (rs2 != rsn)) rs = rs2 ;
	    rs2 = hdb_curend(op,&cur) ;
	    if (rs >= 0) rs = rs2 ;
	} /* end if (cursor) */
	if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;

	rs1 = hdb_finish(op) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sethand_finish) */


int sethand_already(sethand *op,cchar *sp,int sl)
{
	HDB_DATUM	key, val ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		f = TRUE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	key.buf = sp ;
	key.len = sl ;
	val.buf = sp ;
	val.len = sl ;
	if ((rs = hdb_fetch(op,key,NULL,&val)) == rsn) {
	    f = FALSE ;
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("sethand_already: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sethand_already) */


int sethand_add(sethand *op,cchar *sp,int sl)
{
	HDB_DATUM	key, val ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	key.buf = sp ;
	key.len = sl ;
	val.buf = sp ;
	val.len = sl ;
	if ((rs = hdb_fetch(op,key,NULL,&val)) == rsn) {
	    cchar	*asp ;
	    if ((rs = uc_mallocstrw(sp,sl,&asp)) >= 0) {
		key.buf = asp ;
		val.buf = asp ;
	        f = TRUE ;
	        rs = hdb_store(op,key,val) ;
		if (rs < 0)
		    uc_free(asp) ;
	    } /* end if (m-a) */
	} /* end if (not already present) */
#if	CF_DEBUGS
	debugprintf("sethand_add: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sethand_add) */


int sethand_del(sethand *op,cchar *sp,int sl)
{
	HDB_DATUM	key, val ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (sl < 0) sl = strlen(sp) ;
	key.buf = sp ;
	key.len = sl ;
	if ((rs = hdb_fetch(op,key,NULL,&val)) >= 0) {
	    cchar	*asp = (cchar *) val.buf ;
	    rs1 = hdb_delkey(op,key) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(asp) ;
	    if (rs >= 0) rs = rs1 ;
	    f = TRUE ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("sethand_del: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (sethand_del) */


/* return the count of the number of items in this list */
int sethand_count(sethand *op)
{
	return hdb_count(op) ;
}
/* end subroutine (sethand_count) */


int sethand_curbegin(sethand *op,sethand_cur *curp)
{
	return hdb_curbegin(op,curp) ;
}
/* end subroutine (sethand_curbegin) */


int sethand_curend(sethand *op,sethand_cur *curp)
{
	return hdb_curend(op,curp) ;
}
/* end subroutine (sethand_curend) */


int sethand_enum(sethand *op,sethand_cur *curp,cchar **rpp)
{
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if ((rs = hdb_enum(op,curp,&key,&val)) >= 0) {
	    rl = val.len ;
	    if (rpp != NULL) {
	        *rpp = val.buf ;
	    }
	} else {
	    if (rpp != NULL) *rpp = NULL ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (sethand_enum) */


