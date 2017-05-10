/* envs */

/* environment list container */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* some safety */
#define	CF_SAFE2	1		/* more (little) safety */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object just provides a hash list of environment-like variables
	(with a key and an associated value).


*******************************************************************************/


#define	ENVS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecstr.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"envs.h"


/* local defines */

#define	ENVS_ENT	struct envs_e
#define	ENVS_DEFENTS	10

#define	ENVS_DBSTART(op,n,hash,cmp)	hdb_start((op),(n),1,(hash),(cmp))
#define	ENVS_DBSTORE(op,k,v)		hdb_store((op),(k),(v))
#define	ENVS_DBGETREC(op,cp,kp,vp)	hdb_getrec((op),(cp),(kp),(vp))
#define	ENVS_DBNEXT(op,cp)		hdb_next((op),(cp))
#define	ENVS_DBENUM(op,cp,kp,vp)	hdb_enum((op),(cp),(kp),(vp))
#define	ENVS_DBCURBEGIN(op,cp)		hdb_curbegin((op),(cp))
#define	ENVS_DBCUREND(op,cp)		hdb_curend((op),(cp))
#define	ENVS_DBFETCH(op,k,cp,vp)	hdb_fetch((op),(k),(cp),(vp))
#define	ENVS_DBCOUNT(op)		hdb_count((op))
#define	ENVS_DBFINISH(op)		hdb_finish((op))

#define	ENVS_DBDATA			HDB_DATUM
#define	ENVS_DBCURSOR			HDB_CUR


/* external subroutines */

extern int	vecstr_adduniq(vecstr *,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	strnnlen(cchar *,int,int) ;
#endif

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

struct envs_e {
	cchar		*kp ;
	int		kl ;
	VECSTR		list ;
} ;


/* forward references */

static int	entry_start(ENVS_ENT *,cchar *,cchar *,int,cchar **) ;
static int	entry_count(ENVS_ENT *) ;
static int	entry_set(ENVS_ENT *,cchar *,int) ;
static int	entry_append(ENVS_ENT *,cchar *,int) ;
static int	entry_get(ENVS_ENT *,int,cchar **) ;
static int	entry_substr(ENVS_ENT *,cchar *,int) ;
static int	entry_finish(ENVS_ENT *) ;


/* local variables */


/* exported subroutines */


int envs_start(ENVS *op,int n)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("envs_start: n=%d\n",n) ;
#endif

	if ((rs = ENVS_DBSTART(&op->vars,n,NULL,NULL)) >= 0) {
	    op->magic = ENVS_MAGIC ;
	}

	return rs ;
}
/* end subroutine (envs_start) */


/* free up the entire list object structure */
int envs_finish(ENVS *op)
{
	ENVS_DBCURSOR	cur ;
	ENVS_DBDATA	key, val ;
	ENVS_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != ENVS_MAGIC) return SR_NOTOPEN ;
#endif

/* free up all variable-entries */

	ENVS_DBCURBEGIN(&op->vars,&cur) ;
	while (ENVS_DBENUM(&op->vars,&cur,&key,&val) >= 0) {
	    ep = (ENVS_ENT *) val.buf ;
	    if (ep == NULL) continue ;
	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end while */
	ENVS_DBCUREND(&op->vars,&cur) ;

/* free up all variable-container */

	rs1 = ENVS_DBFINISH(&op->vars) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (envs_finish) */


int envs_store(ENVS *op,cchar *kp,int fa,cchar *vp,int vl)
{
	ENVS_DBDATA	key, val ;
	ENVS_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (op->magic != ENVS_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("envs_store: k=%s\n",kp) ;
	debugprintf("envs_store: f_add=%u\n",fa) ;
	debugprintf("envs_store: vp(%p)=%t\n",vp,
		vp,strnlen(vp,MIN(vl,40))) ;
#endif

	if ((vp == NULL) && (vl > 0)) return SR_FAULT ;

/* find this variable (if we have it) */

	key.buf = kp ;
	key.len = strlen(kp) ;

	val.buf = NULL ;
	val.len = -1 ;

	if ((rs1 = ENVS_DBFETCH(&op->vars,key,NULL,&val)) >= 0) {

	    ep = (ENVS_ENT *) val.buf ;

/* add the entry to this environment variable */

	    if (fa) {
	        rs = entry_append(ep,vp,vl) ;
	    } else {
	        rs = entry_set(ep,vp,vl) ;
	    }

	} else if (rs1 == SR_NOTFOUND) {
	    const int	size = sizeof(ENVS_ENT) ;
	    int		knl ;
	    cchar	*knp ;

	    if ((rs = uc_malloc(size,&ep)) >= 0) {

	        if ((rs = entry_start(ep,kp,vp,vl,&knp)) >= 0) {
	            knl = rs ;

	            key.buf = knp ;
	            key.len = knl ;

	            val.buf = ep ;
	            val.len = sizeof(ENVS_ENT) ;

	            rs = ENVS_DBSTORE(&op->vars,key,val) ;
	            if (rs < 0)
	                entry_finish(ep) ;

	        } /* end if */

		if (rs < 0)
	            uc_free(ep) ;

	    } /* end if (allocated entry) */

	} else
	    rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("envs_store: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envs_add) */


/* return the count of the number of items in this list */
int envs_count(ENVS *op)
{
	int		rs ;

	rs = ENVS_DBCOUNT(&op->vars) ;

	return rs ;
}
/* end subroutine (envs_count) */


/* search for an entry in the list */
int envs_present(ENVS *op,cchar *kp,int kl)
{
	ENVS_DBDATA	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("envs_present: ent k=%t\n",kp,kl) ;
#endif

	key.buf = kp ;
	key.len = kl ;

	if ((rs = ENVS_DBFETCH(&op->vars,key,NULL,&val)) >= 0) {
	    ENVS_ENT	*ep = (ENVS_ENT *) val.buf ;
	    rs = entry_count(ep) ;
	}

#if	CF_DEBUGS
	debugprintf("envs_present: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envs_present) */


/* does the variable: a. exist? b. contain the given sub-string? */
int envs_substr(ENVS *op,cchar *kp,int kl,cchar *sp,int sl)
{
	HDB		*vlp ;
	ENVS_DBDATA	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;
	if (sl < 0) sl = strlen(sp) ;

	key.buf = kp ;
	key.len = kl ;

	vlp = &op->vars ;
	if ((rs = ENVS_DBFETCH(vlp,key,NULL,&val)) >= 0) {
	    ENVS_ENT	*ep = (ENVS_ENT *) val.buf ;
	    rs = entry_substr(ep,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (envs_substr) */


int envs_curbegin(ENVS *op,ENVS_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	rs = ENVS_DBCURBEGIN(&op->vars,&curp->cur) ;

	return rs ;
}
/* end subroutine (envs_curbegin) */


int envs_curend(ENVS *op,ENVS_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	rs = ENVS_DBCUREND(&op->vars,&curp->cur) ;

	return rs ;
}
/* end subroutine (envs_curend) */


/* enumerate keys */
int envs_enumkey(ENVS *op,ENVS_CUR *curp,cchar **kpp)
{
	ENVS_DBDATA	key, val ;
	int		rs ;
	int		kl = 0 ;
	cchar		*kp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (kpp != NULL)
	    *kpp = NULL ;

	if ((rs = ENVS_DBENUM(&op->vars,&curp->cur,&key,&val)) >= 0) {

	    kp = (cchar *) key.buf ;
	    kl = strlen(kp) ;

	    if (kpp != NULL)
		*kpp = kp ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("envs_enumkey: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (envs_enumkey) */


/* enumerate key-values */
int envs_enum(ENVS *op,ENVS_CUR *curp,cchar **kpp,cchar **vpp)
{
	ENVS_DBDATA	key, val ;
	ENVS_ENT	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		kl = 0 ;
	cchar		*kp, *vp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (kpp != NULL)
	    *kpp = NULL ;

	if (vpp != NULL)
	    *vpp = NULL ;

	while (rs >= 0) {

	   if (curp->i < 0)
	       rs = ENVS_DBNEXT(&op->vars,&curp->cur) ;

	   if (rs >= 0)
	       rs = ENVS_DBGETREC(&op->vars,&curp->cur,&key,&val) ;

#if	CF_DEBUGS
	debugprintf("envs_enum: envs_dbenum() rs=%d\n",rs) ;
#endif

	   if (rs < 0)
		break ;

	    kp = (cchar *) key.buf ;
	    kl = strlen(kp) ;

	    if (kpp != NULL)
		*kpp = kp ;

	    ep = (ENVS_ENT *) val.buf ;
	    i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

#if	CF_DEBUGS
	debugprintf("envs_enum: entry_get() i=%d\n",i) ;
#endif

	    if ((rs = entry_get(ep,i,&vp)) >= 0) {

	        curp->i = i ;

	        if (vpp != NULL)
		    *vpp = vp ;

		break ;

	    } else if (rs == SR_NOTFOUND) {

		curp->i = -1 ;

	    } /* end if */

	} /* end while */

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (envs_enum) */


/* fetch a component of a variables */
int envs_fetch(ENVS *op,cchar *kp,int kl,ENVS_CUR *curp,cchar **rpp)
{
	ENVS_DBDATA	key, val ;
	ENVS_ENT	*ep ;
	int		rs ;
	int		i ;
	int		vl = 0 ;
	cchar		*vp ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

	if (rpp != NULL)
	    *rpp = NULL ;

	i = 0 ;
	if (curp != NULL)
	    i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

#if	CF_DEBUGS
	debugprintf("envs_fetch: ent k=%t\n",kp,kl) ;
#endif

	key.buf = kp ;
	key.len = kl ;

	if ((rs = ENVS_DBFETCH(&op->vars,key,NULL,&val)) >= 0) {
	    ep = (ENVS_ENT *) val.buf ;
	    if ((rs = entry_get(ep,i,&vp)) >= 0) {
		vl = rs ;
	        if (curp != NULL) curp->i = i ;
	        if (rpp != NULL) *rpp = vp ;
	    }
	} /* end if */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (envs_fetch) */


/* delete an entry by name */
int envs_delname(ENVS *op,cchar *kp,int kl)
{
	ENVS_DBDATA	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("envs_delname: ent k=%t\n",kp,kl) ;
#endif

	key.buf = kp ;
	key.len = kl ;

	rs = ENVS_DBFETCH(&op->vars,key,NULL,&val) ;

#if	CF_DEBUGS
	debugprintf("envs_delname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envs_delname) */


/* private subroutines */


static int entry_start(ENVS_ENT *ep,cchar kp[],cchar vn[],int vnlen,cchar **rpp)
{
	int		rs ;
	cchar		*cp ;

	if ((kp == NULL) || (kp[0] == '\0')) return SR_INVALID ;

	if ((rs = uc_mallocstrw(kp,-1,&cp)) >= 0) {
	    const int	n = ENVS_DEFENTS ;
	    int		opts = 0 ;
	    ep->kl = (rs - 1) ;
	    ep->kp = cp ;
	    opts |= (VECSTR_OCOMPACT | VECSTR_OORDERED) ;
	    opts |= (VECSTR_OSTSIZE | VECSTR_OREUSE) ;
	    if ((rs = vecstr_start(&ep->list,n,opts)) >= 0) {
	        if (vn == NULL) vn = (ep->kp + ep->kl) ;
	        if ((rs = vecstr_add(&ep->list,vn,vnlen)) >= 0) {
	            if (rpp != NULL) *rpp = ep->kp ;
	        }
	        if (rs < 0)
		    vecstr_finish(&ep->list) ;
	    } /* end if (vecstr_start) */
	    if (rs < 0) {
		uc_free(ep->kp) ;
		ep->kp = NULL ;
	    }
	} /* end if (memory-allocation) */

	return (rs >= 0) ? ep->kl : rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(ENVS_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->kp != NULL) {
	    rs1 = uc_free(ep->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->kp = NULL ;
	}

	rs1 = vecstr_finish(&ep->list) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_count(ENVS_ENT *ep)
{
	return vecstr_count(&ep->list) ;
}
/* end subroutine (entry_count) */


static int entry_set(ENVS_ENT *ep,cchar vp[],int vl)
{
	int		rs ;

	if ((rs = vecstr_delall(&ep->list)) >= 0) {
	    rs = vecstr_add(&ep->list,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (entry_set) */


static int entry_append(ENVS_ENT *ep,cchar vp[],int vl)
{
	int		rs ;

	rs = vecstr_add(&ep->list,vp,vl) ;

	return rs ;
}
/* end subroutine (entry_append) */


static int entry_get(ENVS_ENT *ep,int i,cchar **rpp)
{
	int		rs ;
	int		vl = 0 ;

	if ((rs = vecstr_get(&ep->list,i,rpp)) >= 0) {
	    if (*rpp != NULL) vl = strlen(*rpp) ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (entry_get) */


/* is the given string a substring of any component */
static int entry_substr(ENVS_ENT *ep,cchar *sp,int sl)
{
	NULSTR		n ;
	int		rs ;
	int		f = FALSE ;
	cchar		*ss ;

#if	CF_DEBUGS
	debugprintf("entry_substr: ent k=%t\n",ep->kp,ep->kl) ;
	debugprintf("entry_substr: ss=%t\n",sp,sl) ;
#endif

	if ((rs = nulstr_start(&n,sp,sl,&ss)) >= 0) {
	    VECSTR	*clp = &ep->list ;
	    int		i ;
	    cchar	*cp ;
	    for (i = 0 ; (rs = vecstr_get(clp,i,&cp)) >= 0 ; i += 1) {
	        if (cp != NULL) {
#if	CF_DEBUGS
	debugprintf("entry_substr: c=%s\n",cp) ;
#endif
		    f = (strstr(cp,ss) != NULL) ;
		    if (f) break ;
	        }
	    } /* end for */
	    if (rs == SR_NOTFOUND) rs = SR_OK ;
	    nulstr_finish(&n) ;
	} /* end if (nulstr) */

#if	CF_DEBUGS
	debugprintf("entry_substr: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (entry_substr) */


