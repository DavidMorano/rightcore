/* citedb */

/* maintain a DB of encountered citations */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This code module was originally written.

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

*/

/* Copyright © 1987,1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This code module (object) maintains a citation database. It stores the
        citation keys, and a count for each, that are found within the document
        text.

	No emumeration is required since only lookups by key are needed.


******************************************************************************/


#define	CITEDB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"citedb.h"


/* local defines */

#define	CITEDB_MAGIC	31
#define	CITEDB_DEFENTS	200

#define	STORE		CITEDB_STORE


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

int		citedb_add(CITEDB *,int,uint,const char *,int) ;

static int	mkcitestr(char *,int) ;

static int	store_start(CITEDB_STORE *,const char *,int) ;
static int	store_update(CITEDB_STORE *,int) ;
static int	store_finish(CITEDB_STORE *) ;

static int	entry_load(CITEDB_ENT *,CITEDB_STORE *,CITEDB_OFF *) ;


/* local variables */


/* exported subroutines */


int citedb_start(CITEDB *op)
{
	const int	n = CITEDB_DEFENTS ;
	const int	size = sizeof(CITEDB_OFF) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("citedb_start: ent\n") ;
#endif

	memset(op,0,sizeof(CITEDB)) ;
	op->citestrindex = 0 ;

	if ((rs = vecobj_start(&op->list,size,n,0)) >= 0) {
	    if ((rs = hdb_start(&op->store,n,1,NULL,NULL)) >= 0) {
		op->magic = CITEDB_MAGIC ;
	    }
	    if (rs < 0)
		vecobj_finish(&op->list) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("citedb_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (citedb_start) */


int citedb_adds(CITEDB *op,int fi,uint coff,cchar *kp,int kl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	if (kp[0] == '\0') return SR_INVALID ;

	if (kl < 0) kl = strlen(kp) ;

	while ((tp = strnchr(kp,kl,',')) != NULL) {
	    if ((cl = sfshrink(kp,(tp - kp),&cp)) > 0) {
		c += 1 ;
	        rs = citedb_add(op,fi,coff,cp,cl) ;
	    }
	    kl -= ((tp + 1) - kp) ;
	    kp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (kl > 0)) {
	    if ((cl = sfshrink(kp,kl,&cp)) > 0) {
		c += 1 ;
	        rs = citedb_add(op,fi,coff,cp,cl) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (citedb_adds) */


/* load a string parameter into the DB */
int citedb_add(CITEDB *op,int fi,uint coff,cchar *kp,int kl)
{
	CITEDB_STORE	*sp ;
	HDB_DATUM	key, value ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("citedb_add: coff=%u\n",coff) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	if (kp[0] == '\0') return SR_INVALID ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("citedb_add: kp=%t\n",kp,kl) ;
#endif

#if	CF_DEBUGS
	debugprintf("citedb_add: continuing\n") ;
#endif

/* check if this citation is in the general "store" DB */

	key.buf = (void *) kp ;
	key.len = (kl >= 0) ? kl : strlen(kp) ;

	if ((rs = hdb_fetch(&op->store,key,NULL,&value)) >= 0) {

	    sp = (CITEDB_STORE *) value.buf ;
	    rs = store_update(sp,op->citestrindex) ;

	    if (sp->n == 2)
	    	op->citestrindex += 1 ;

	} else if (rs == SR_NOTFOUND) {
	    const int	size = sizeof(CITEDB_STORE) ;

	    if ((rs = uc_malloc(size,&sp)) >= 0) {

	        rs = store_start(sp,kp,kl) ;

#if	CF_DEBUGS
	        debugprintf("citedb_add: store_start() rs=%d\n",rs) ;
#endif

	        key.buf = (void *) sp->citekey ;

	        value.buf = sp ;
	        value.len = size ;

	        if (rs >= 0) {

	            rs = hdb_store(&op->store,key,value) ;

	            if (rs < 0)
	                store_finish(sp) ;

	        }

	        if (rs < 0)
	            uc_free(sp) ;

	    } /* end if (memory-allocation) */

	} /* end if */

/* now check if it is in the "offset" DB */

	if (rs >= 0) {
	    CITEDB_OFF	offe ;

#if	CF_DEBUGS
	    debugprintf("citedb_add: store cn=%u\n",sp->n) ;
#endif

	    memset(&offe,0,sizeof(CITEDB_OFF)) ;
	    offe.sp = sp ;
	    offe.coff = coff ;
	    offe.fi = fi ;
	    offe.ci = sp->n ;		/* index at time seen */

#if	CF_DEBUGS
	    debugprintf("citedb_add: vecobj_add() ci=%u \n",
		offe.ci) ;
#endif

	    rs = vecobj_add(&op->list,&offe) ;

#if	CF_DEBUGS
	    debugprintf("citedb_add: vecobj_add() rs=%d\n",rs) ;
#endif

	} /* end if (offset DB) */

#if	CF_DEBUGS
	debugprintf("citedb_add: store cn=%u\n",sp->n) ;
	debugprintf("citedb_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (citedb_add) */


int citedb_curbegin(CITEDB *op,CITEDB_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("citedb_curbegin: ent \n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (citedb_curbegin) */


int citedb_curend(CITEDB *op,CITEDB_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("citedb_curend: ent \n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (citedb_curend) */


int citedb_enum(CITEDB *op,CITEDB_CUR *curp,CITEDB_ENT *ep)
{
	CITEDB_OFF	*offp ;
	CITEDB_STORE	*sp ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("citedb_enum: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

/* do the lookup */

	while ((rs = vecobj_get(&op->list,i,&offp)) >= 0) {
	    if (offp != NULL) break ;
	    i += 1 ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("citedb_enum: vecobj_get() rs=%d\n",rs) ;
	if (rs >= 0)
	debugprintf("citedb_enum: ci=%u\n",offp->ci) ;
#endif

	if ((rs >= 0) && (offp != NULL)) {
	    sp = offp->sp ;
	    if (ep != NULL) {
	        rs = entry_load(ep,sp,offp) ;
	    }
	    curp->i = i ;
	}

#if	CF_DEBUGS
	debugprintf("citedb_enum: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (citedb_enum) */


int citedb_fetch(CITEDB *op,cchar *citekey,CITEDB_CUR *curp,CITEDB_ENT *ep)
{
	CITEDB_OFF	*offp ;
	CITEDB_STORE	*sp ;
	CITEDB_CUR	cur ;
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (citekey == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	if (citekey[0] == '\0') return SR_INVALID ;

	if (curp == NULL) {
	    curp = &cur ;
	    curp->i = -1 ;
	}

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

/* do the lookup */

	while ((rs = vecobj_get(&op->list,i,&offp)) >= 0) {
	    if (offp != NULL) {
	        sp = offp->sp ;
	        if (strcmp(sp->citekey,citekey) == 0) break ;
	        i += 1 ;
	    }
	} /* end while */

	if ((rs >= 0) && (offp != NULL)) {
	    sp = offp->sp ;
	    if (ep != NULL) {
	        rs = entry_load(ep,sp,offp) ;
	    }
	    curp->i = i ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (citedb_fetch) */


int citedb_finish(CITEDB *op)
{
	CITEDB_STORE	*sp ;
	HDB_CUR		keycursor ;
	HDB_DATUM	key, value ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

/* clean out the "store" DB */

	if (hdb_curbegin(&op->store,&keycursor) >= 0) {
	    while (hdb_enum(&op->store,&keycursor,&key,&value) >= 0) {
	        sp = (CITEDB_STORE *) value.buf ;
	        rs1 = store_finish(sp) ;
	        if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(sp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    rs1 = hdb_curend(&op->store,&keycursor) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

/* ready to drop the whole object */

	rs1 = hdb_finish(&op->store) ;
	if (rs >= 0) rs = rs1 ;

/* clean out the "offset" DB list */

	rs1 = vecobj_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (citedb_finish) */


int citedb_audit(CITEDB *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CITEDB_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_audit(&op->list) ;

#if	CF_DEBUGS
	debugprintf("citedb_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (citedb_audit) */


/* private subroutines */


static int store_start(CITEDB_STORE *sp,cchar *kp,int kl)
{
	int		rs ;
	cchar		*cp ;

	if (sp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	memset(sp,0,sizeof(CITEDB_STORE)) ;

	if ((rs = uc_mallocstrw(kp,kl,&cp)) >= 0) {
	    sp->citekey = cp ;
	    sp->n = 1 ;
	}

	return rs ;
}
/* end subroutine (store_start) */


static int store_update(CITEDB_STORE *sp,int n)
{
	int		rs = SR_OK ;

	if (sp == NULL) return SR_FAULT ;

	if (sp->n == 1) {
	    rs = mkcitestr(sp->citestr,n) ;
	}

	if (rs >= 0)
	    sp->n += 1 ;

	return (rs >= 0) ? sp->n : rs ;
}
/* end subroutine (store_update) */


static int store_finish(CITEDB_STORE *sp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sp == NULL) return SR_FAULT ;

	if (sp->citekey != NULL) {
	    rs1 = uc_free(sp->citekey) ;
	    if (rs >= 0) rs = rs1 ;
	    sp->citekey = NULL ;
	}

	return rs ;
}
/* end subroutine (store_finish) */


static int entry_load(CITEDB_ENT *ep,CITEDB_STORE *sp,CITEDB_OFF *offp)
{
	int		rs ;

	ep->coff = 0 ;
	ep->fi = 0 ;
	ep->ci = 0 ;

	if ((rs = sncpy1(ep->citekey,CITEDB_CITEKEYLEN,sp->citekey)) >= 0) {
	    ep->n = sp->n ;
	    strwcpy(ep->citestr,sp->citestr,CITEDB_CITESTRLEN) ;
	    if (offp != NULL) {
	        ep->coff = offp->coff ;
	        ep->fi = offp->fi ;
	        ep->ci = offp->ci ;
	    }
	}

	return rs ;
}
/* end subroutine (entry_load) */


/* make a TROFF citation reference string */
static int mkcitestr(char *buf,int index)
{

	buf[0] = '\0' ;
	if (index > (2 * 26))
	    return SR_OVERFLOW ;

	buf[0] = 'r' ;
	buf[1] = (index <= 26) ? ('a' + index) : ('A' + index) ;
	buf[2] = '\0' ;
	buf[3] = '\0' ;
	return SR_OK ;
}
/* end subroutine (mkcitestr) */


