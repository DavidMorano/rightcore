/* pcsnsrecs */

/* PCS Name-Server Records */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides storage for PCS Name-Server records.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<recarr.h>
#include	<localmisc.h>

#include	"pcsnsrecs.h"


/* local defines */

#ifndef	PCSNSRECS_REC
#define	PCSNSRECS_REC	struct pcsnsrecs_r
#endif

#define	RECINFO		struct recinfo


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct recinfo {
	cchar		*un ;
	cchar		*vbuf ;
	int		vlen ;
	int		w ;
	int		ttl ;		/* time-to-live */
} ;

enum cts {
	ct_null,
	ct_miss,
	ct_hit,
	ct_overlast
} ;

struct pcsnsrecs_r {
	PQ_ENT		linkage ;
	char		*vbuf ;		/* value buffer */
	time_t		ti_create ;
	time_t		ti_access ;
	uint		wcount ;	/* write count */
	int		vlen ;		/* value length */
	int		vl ;
	int		recidx ;	/* record index */
	int		w ;		/* "which" number */
	int		ttl ;
	char		un[USERNAMELEN+1] ; /* key */
} ;


/* forward references */

static int	pcsnsrecs_fetch(PCSNSRECS *,PCSNSRECS_REC **,cchar *,int) ;
static int	pcsnsrecs_mkrec(PCSNSRECS *,time_t,RECINFO *) ;
static int	pcsnsrecs_newrec(PCSNSRECS *,time_t,
			PCSNSRECS_REC **,RECINFO *) ;
static int	pcsnsrecs_recstart(PCSNSRECS *,time_t,
			PCSNSRECS_REC *,RECINFO *) ;
static int	pcsnsrecs_recdel(PCSNSRECS *,PCSNSRECS_REC *) ;
static int	pcsnsrecs_recaccess(PCSNSRECS *,time_t,PCSNSRECS_REC *) ;
static int	pcsnsrecs_recrear(PCSNSRECS *,PCSNSRECS_REC *) ;
static int	pcsnsrecs_recfins(PCSNSRECS *op) ;
static int	pcsnsrecs_upstats(PCSNSRECS *,int,int) ;

static int record_start(PCSNSRECS_REC *,time_t,int,RECINFO *) ;
static int record_access(PCSNSRECS_REC *,time_t) ;
static int record_old(PCSNSRECS_REC *,time_t,int) ;
static int record_finish(PCSNSRECS_REC *) ;
static int record_get(PCSNSRECS_REC *,char *,int) ;


/* local variables */


/* exported subroutines */


int pcsnsrecs_start(PCSNSRECS *op,int max,int ttl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < PCSNSRECS_DEFMAX)
	    max = PCSNSRECS_DEFMAX ;

	if (ttl < PCSNSRECS_DEFTTL)
	    ttl = PCSNSRECS_DEFTTL ;

	memset(op,0,sizeof(PCSNSRECS)) ;

	{
	    const int	size = sizeof(RECARR) ;
	    char	*p ;
	    if ((rs = uc_libmalloc(size,&p)) >= 0) {
	        int	ro = 0 ;
	        ro |= RECARR_OSTATIONARY ;
	        ro |= RECARR_OREUSE ;
	        ro |= RECARR_OCONSERVE ;
	        op->recs = p ;
	        if ((rs = recarr_start(op->recs,max,ro)) >= 0) {
	            if ((rs = pq_start(&op->lru)) >= 0) {
	                op->max = max ;
	                op->ttl = ttl ;
	                op->ti_check = time(NULL) ;
	                op->magic = PCSNSRECS_MAGIC ;
	            }
	            if (rs < 0)
	                recarr_finish(op->recs) ;
	        }
	        if (rs < 0) {
	            uc_libfree(op->recs) ;
	            op->recs = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end block */

	return rs ;
}
/* end subroutine (pcsnsrecs_start) */


int pcsnsrecs_finish(PCSNSRECS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pcsnsrecs_finish: ent\n") ;
#endif

/* finish up the LRU queue */

	rs1 = pq_finish(&op->lru) ;
	if (rs >= 0) rs = rs1 ;

/* loop freeing up all cache entries */

	rs1 = pcsnsrecs_recfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up everyting else */

	rs1 = recarr_finish(op->recs) ;
	if (rs >= 0) rs = rs1 ;

	if (op->recs != NULL) {
	    rs1 = uc_libfree(op->recs) ;
	    if (rs >= 0) rs = rs1 ;
	    op->recs = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("pcsnsrecs_finish: rer rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsnsrecs_finish) */


int pcsnsrecs_store(PCSNSRECS *op,cchar *vbuf,int vlen,cchar *un,int w,int ttl)
{
	RECINFO		ri ;
	const time_t	dt = time(NULL) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

	if (vlen < 1) return SR_INVALID ;
	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsnsrecs_store: u=%s\n",un) ;
#endif

	memset(&ri,0,sizeof(RECINFO)) ;
	ri.un = un ;
	ri.vbuf = vbuf ;
	ri.vlen = vlen ;
	ri.w = w ;
	ri.ttl = ttl ;
	rs = pcsnsrecs_mkrec(op,dt,&ri) ;


#if	CF_DEBUGS
	debugprintf("pcsnsrecs_store: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsrecs_store) */


int pcsnsrecs_lookup(PCSNSRECS *op,char *rbuf,int rlen,cchar *un,int w)
{
	PCSNSRECS_REC	*ep = NULL ;
	int		rs ;
	int		ct = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

	if (rlen < 1) return SR_INVALID ;
	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsnsrecs_lookup: u=%s\n",un) ;
#endif

	op->s.total += 1 ;

	if ((rs = pcsnsrecs_fetch(op,&ep,un,w)) >= 0) {
	    time_t	dt = time(NULL) ;
	    if ((rs = record_old(ep,dt,op->ttl)) > 0) {
	        ct = ct_miss ;
	        if ((rs = pcsnsrecs_recdel(op,ep)) >= 0) {
	            PQ_ENT	*pep = (PQ_ENT *) ep ;
	            rs = pq_unlink(&op->lru,pep) ;
	            uc_libfree(ep) ;
		    if (rs >= 0) rs = 0 ;
	        }
	    } else {
	        ct = ct_hit ;
	        rs = pcsnsrecs_recaccess(op,dt,ep) ;
	    }
	} else if (rs == SR_NOTFOUND) {
	    ct = ct_miss ;
	} /* end if (hit or miss) */

	pcsnsrecs_upstats(op,ct,rs) ;

	if (rs > 0) { /* not '>=' */
	    rs = record_get(ep,rbuf,rlen) ;
	} else if (rs == 0) {
	    rs = SR_NOTFOUND ;
	}

	if (rs <= 0) rbuf[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("pcsnsrecs_lookup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsnsrecs_lookup) */


int pcsnsrecs_invalidate(PCSNSRECS *op,cchar un[],int w)
{
	PCSNSRECS_REC	*ep ;
	int		rs ;
	int		rs1 ;
	int		f_found = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

	if ((rs = pcsnsrecs_fetch(op,&ep,un,w)) >= 0) {
	    const int	ri = rs ;
	    PQ_ENT	*pep = (PQ_ENT *) ep ;

	    f_found = TRUE ;

	    rs1 = pq_unlink(&op->lru,pep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = recarr_del(op->recs,ri) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = record_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = uc_libfree(ep) ;
	    if (rs >= 0) rs = rs1 ;

	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (pcsnsrecs_invalidate) */


int pcsnsrecs_check(PCSNSRECS *op,time_t dt)
{
	PCSNSRECS_REC	*ep ;
	RECARR		*rlp = op->recs ;
	int		rs ;
	int		i ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

/* loop checking all cache entries */

	for (i = 0 ; recarr_get(rlp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (dt == 0) dt = time(NULL) ;
	        if ((rs = record_old(ep,dt,op->ttl)) > 0) {
	            f = TRUE ;
	            if ((rs = pcsnsrecs_recdel(op,ep)) >= 0) {
	                PQ_ENT	*pep = (PQ_ENT *) ep ;
	                rs = pq_unlink(&op->lru,pep) ;
	                uc_libfree(ep) ;
	            }
	        } /* end if (entry-old) */
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pcsnsrecs_check) */


int pcsnsrecs_stats(PCSNSRECS *op,PCSNSRECS_ST *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != PCSNSRECS_MAGIC) return SR_NOTOPEN ;

	if ((rs = recarr_count(op->recs)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}

	return rs ;
}
/* end subroutine (pcsnsrecs_stats) */


/* private subroutines */


static int pcsnsrecs_fetch(PCSNSRECS *op,PCSNSRECS_REC **epp,cchar *un,int w)
{
	RECARR		*rlp = op->recs ;
	PCSNSRECS_REC	*ep ;
	int		rs ;
	int		i ;

	for (i = 0 ; (rs = recarr_get(rlp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if ((un[0] == ep->un[0]) && (w == ep->w)) {
	            if (strcmp(un,ep->un) == 0) break ;
	        }
	    }
	} /* end for */

	if (epp != NULL)
	    *epp = (rs >= 0) ? ep : NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (pcsnsrecs_fetch) */


static int pcsnsrecs_mkrec(PCSNSRECS *op,time_t dt,RECINFO *rip)
{
	int		rs ;
	int		rs1 ;
	int		vl = 0 ;

	if ((rs = recarr_count(op->recs)) >= 0) {
	    PQ_ENT	*pep ;
	    const int	n = rs ;

	    if (n >= op->max) {

	        if ((rs = pq_rem(&op->lru,&pep)) >= 0) {
	            PCSNSRECS_REC	*ep = (PCSNSRECS_REC *) pep ;

	            if ((rs = pcsnsrecs_recdel(op,ep)) >= 0) {
	                rs = pcsnsrecs_recstart(op,dt,ep,rip) ;
	                vl = rs ;
	            }

	            rs1 = pq_ins(&op->lru,pep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (removed entry) */

	    } else {
	        PCSNSRECS_REC	*ep ;

	        if ((rs = pcsnsrecs_newrec(op,dt,&ep,rip)) >= 0) {
	            vl = rs ;
	            if (ep != NULL) {
	                pep = (PQ_ENT *) ep ;
	                rs = pq_ins(&op->lru,pep) ;
	            }
	        } /* end if (new-entry) */

	    } /* end if */

	} /* end if (recarr_count) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (pcsnsrecs_mkrec) */


static int pcsnsrecs_newrec(PCSNSRECS *op,time_t dt,
		PCSNSRECS_REC **epp,RECINFO *rip)
{
	PCSNSRECS_REC	*ep ;
	const int	size = sizeof(PCSNSRECS_REC) ;
	int		rs ;

	if ((rs = uc_libmalloc(size,&ep)) >= 0) {
	    rs = pcsnsrecs_recstart(op,dt,ep,rip) ;
	    if (rs < 0)
	        uc_libfree(ep) ;
	} /* end if (m-a) */

	*epp = (rs >= 0) ? ep : NULL ;
	return rs ;
}
/* end subroutine (pcsnsrecs_newrec) */


static int pcsnsrecs_recstart(PCSNSRECS *op,time_t dt,PCSNSRECS_REC *ep,
		RECINFO *rip)
{
	const int	wc = op->wcount++ ;
	int		rs ;
	int		rl = 0 ;

	if ((rs = record_start(ep,dt,wc,rip)) >= 0) {
	    rl = rs ;
	    rs = recarr_add(op->recs,ep) ;
	    ep->recidx = rs ;
	    if (rs < 0)
	        record_finish(ep) ;
	} /* end if (entry-start) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsnsrecs_recstart) */


static int pcsnsrecs_recaccess(PCSNSRECS *op,time_t dt,PCSNSRECS_REC *ep)
{
	int		rs ;

	if ((rs = pcsnsrecs_recrear(op,ep)) >= 0) {
	    rs = record_access(ep,dt) ;
	} /* end if */

	return rs ;
}
/* end subroutine (pcsnsrecs_recaccess) */


static int pcsnsrecs_recrear(PCSNSRECS *op,PCSNSRECS_REC *ep)
{
	PQ_ENT		*pcp = (PQ_ENT *) ep ;
	PQ_ENT		*pep ;
	int		rs ;

	if ((rs = pq_gettail(&op->lru,&pep)) >= 0) {
	    if (pcp != pep) {
	        pep = (PQ_ENT *) ep ;
	        if ((rs = pq_unlink(&op->lru,pep)) >= 0) {
	            rs = pq_ins(&op->lru,pep) ;
	            if (rs < 0) {
	                PCSNSRECS_REC	*ep = (PCSNSRECS_REC *) pep ;
	                record_finish(ep) ;
	                uc_libfree(pep) ;
	            }
	        }
	    }
	} /* end if (pq-gettail) */

	return rs ;
}
/* end subroutine (pcsnsrecs_recrear) */


static int pcsnsrecs_recdel(PCSNSRECS *op,PCSNSRECS_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs1 = recarr_ent(op->recs,ep)) >= 0) {
	    rs1 = recarr_del(op->recs,rs1) ;
	}
	if (rs >= 0) rs = rs1 ;

	rs1 = record_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pcsnsrecs_recdel) */


static int pcsnsrecs_recfins(PCSNSRECS *op)
{
	RECARR		*rlp = op->recs ;
	PCSNSRECS_REC	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; recarr_get(rlp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = record_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_libfree(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (pcsnsrecs_recfins) */


static int pcsnsrecs_upstats(PCSNSRECS *op,int ct,int rs)
{
	int		f_got = (rs > 0) ;
#if	CF_DEBUGS
	debugprintf("pcsnsrecs_upstats: ct=%u rs=%d\n",ct,rs) ;
#endif
	switch (ct) {
	case ct_hit:
	    if (f_got) op->s.phits += 1 ;
	    else op->s.nhits += 1 ;
	    break ;
	case ct_miss:
	    if (f_got) op->s.pmisses += 1 ;
	    else op->s.nmisses += 1 ;
	    break ;
	} /* end switch */
	return SR_OK ;
}
/* end subroutine (pcsnsrecs_upstats) */


static int record_start(PCSNSRECS_REC *ep,time_t dt,int wc,RECINFO *rip)
{
	int		rs ;
	int		vlen ;
	int		vl = 0 ;
	char		*vbuf ;

	if (ep == NULL) return SR_FAULT ;
	if (rip == NULL) return SR_FAULT ;

	if (rip->un[0] == '\0') return SR_INVALID ;

	vlen = rip->vlen ;

	memset(ep,0,sizeof(PCSNSRECS_REC)) ;
	ep->w = rip->w ;
	strwcpy(ep->un,rip->un,USERNAMELEN) ;

	if ((rs = uc_libmalloc((vlen+1),&vbuf)) >= 0) {
	    vl = (strwcpy(vbuf,rip->vbuf,vlen) - vbuf) ;
	    ep->vbuf = vbuf ;
	    ep->vlen = vlen ;
	    ep->vl = vl ;
	} /* end if (m-a) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	    ep->ttl = rip->ttl ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (record_start) */


static int record_finish(PCSNSRECS_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("record_finish: ent u=%s w=%u\n",ep->un,ep->w) ;
	debugprintf("record_finish: v=>%t<\n",
		ep->vbuf,strlinelen(ep->vbuf,ep->vl,40)) ;
#endif

	if (ep->vbuf != NULL) {
	    rs1 = uc_libfree(ep->vbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->vbuf = NULL ;
	    ep->vlen = 0 ;
	}

	ep->un[0] = '\0' ;
	return rs ;
}
/* end subroutine (record_finish) */


static int record_access(PCSNSRECS_REC *ep,time_t dt)
{
	int		rs = SR_OK ;
	int		vl = 0 ;

	if (ep == NULL) return SR_FAULT ;

	ep->ti_access = dt ;
	vl = ep->vl ;
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (record_access) */


static int record_old(PCSNSRECS_REC *ep,time_t dt,int ttl)
{
	int		f_old ;
	if (ep == NULL) return SR_FAULT ;
	if ((ep->ttl > 0) && (ep->ttl < ttl)) ttl = ep->ttl ;
	f_old = ((dt - ep->ti_create) >= ttl) ;
	return f_old ;
}
/* end subroutine (record_old) */


static int record_get(PCSNSRECS_REC *ep,char *rbuf,int rlen)
{
	int		rs ;
	rs = snwcpy(rbuf,rlen,ep->vbuf,ep->vl) ;
	return rs ;
}
/* end subroutine (record_get) */


