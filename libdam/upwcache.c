/* upwcache */

/* low-level UNIX® PASSWD cache */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides a crude low-level cache for PASSWD-DB entries.

	Notes:

	Q. Why do we not have a real implementation for |upwcache_uid()|?
	A. Two reasons:
	a. we are really lazy
	b. we do not feel that we actually need a better implementation
	at this time


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<passwdent.h>
#include	<recarr.h>
#include	<localmisc.h>

#include	"upwcache.h"


/* local defines */

#ifndef	UPWCACHE_REC
#define	UPWCACHE_REC	struct upwcache_r
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

enum cts {
	ct_miss,
	ct_hit,
	ct_overlast

} ;

struct upwcache_r {
	PQ_ENT		linkage ;
	char		*pwbuf ;
	struct passwd	pw ;
	time_t		ti_create ;
	time_t		ti_access ;
	uint		wcount ;
	int		pwlen ;		/* buffer size */
	int		pwl ;		/* currently used amount (of buffer) */
	int		recidx ;
	char		un[USERNAMELEN+1] ;
} ;


/* forward references */

static int	upwcache_fetch(UPWCACHE *,UPWCACHE_REC **,const char *) ;
static int	upwcache_mkrec(UPWCACHE *,time_t,UPWCACHE_REC **,cchar *) ;
static int	upwcache_newrec(UPWCACHE *,time_t,UPWCACHE_REC **,cchar *) ;
static int	upwcache_recstart(UPWCACHE *,time_t,UPWCACHE_REC *,cchar *) ;
static int	upwcache_recdel(UPWCACHE *,UPWCACHE_REC *) ;
static int	upwcache_recaccess(UPWCACHE *,time_t,UPWCACHE_REC *) ;
static int	upwcache_recrear(UPWCACHE *,UPWCACHE_REC *) ;
static int	upwcache_recfins(UPWCACHE *op) ;
static int	upwcache_upstats(UPWCACHE *,int,int) ;

static int record_start(UPWCACHE_REC *,time_t,int,const char *) ;
static int record_access(UPWCACHE_REC *,time_t) ;
static int record_refresh(UPWCACHE_REC *,time_t,int) ;
static int record_old(UPWCACHE_REC *,time_t,int) ;
static int record_finish(UPWCACHE_REC *) ;


/* local variables */


/* exported subroutines */


int upwcache_start(UPWCACHE *op,int max,int ttl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < UPWCACHE_DEFMAX)
	    max = UPWCACHE_DEFMAX ;

	if (ttl < UPWCACHE_DEFTTL)
	    ttl = UPWCACHE_DEFTTL ;

	memset(op,0,sizeof(UPWCACHE)) ;

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
	                op->magic = UPWCACHE_MAGIC ;
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
/* end subroutine (upwcache_start) */


int upwcache_finish(UPWCACHE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

/* finish up the LRU queue */

	rs1 = pq_finish(&op->lru) ;
	if (rs >= 0) rs = rs1 ;

/* loop freeing up all cache entries */

	rs1 = upwcache_recfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up everything else */

	rs1 = recarr_finish(op->recs) ;
	if (rs >= 0) rs = rs1 ;

	if (op->recs != NULL) {
	    rs1 = uc_libfree(op->recs) ;
	    if (rs >= 0) rs = rs1 ;
	    op->recs = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (upwcache_finish) */


int upwcache_lookup(UPWCACHE *op,struct passwd *pwp,char *pwbuf,int pwlen,
		cchar un[])
{
	UPWCACHE_REC	*ep = NULL ;
	time_t		dt = time(NULL) ;
	int		rs ;
	int		ct ;

	if (op == NULL) return SR_FAULT ;
	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

	if (pwlen < 1) return SR_INVALID ;
	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("upwcache_lookup: u=%s\n",un) ;
#endif

	op->s.total += 1 ;

	if ((rs = upwcache_fetch(op,&ep,un)) >= 0) {
	    ct = ct_hit ;
	    rs = upwcache_recaccess(op,dt,ep) ;
	} else if (rs == SR_NOTFOUND) {
	    ct = ct_miss ;
	    rs = upwcache_mkrec(op,dt,&ep,un) ;
	} /* end if (hit or miss) */

	upwcache_upstats(op,ct,rs) ;

	if (rs > 0) { /* not '>=' */
	    rs = passwdent_load(pwp,pwbuf,pwlen,&ep->pw) ;
	} else if (rs == 0) {
	    rs = SR_NOTFOUND ;
	}

	if (rs <= 0)
	    memset(pwp,0,sizeof(struct passwd)) ;

#if	CF_DEBUGS
	debugprintf("upwcache_lookup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (upwcache_lookup) */


int upwcache_uid(UPWCACHE *op,struct passwd *pwp,char *pwbuf,int pwlen,
		uid_t uid)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

	rs = uc_getpwuid(uid,pwp,pwbuf,pwlen) ;

	return rs ;
}
/* end subroutine (upwcache_uid) */


int upwcache_invalidate(UPWCACHE *op,cchar un[])
{
	UPWCACHE_REC	*ep ;
	int		rs ;
	int		rs1 ;
	int		f_found = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

	if ((rs = upwcache_fetch(op,&ep,un)) >= 0) {
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

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (upwcache_invalidate) */


int upwcache_check(UPWCACHE *op,time_t dt)
{
	UPWCACHE_REC	*ep ;
	RECARR		*rlp = op->recs ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

/* loop checking all cache entries */

	for (i = 0 ; recarr_get(rlp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (dt == 0) dt = time(NULL) ;
	        if ((rs = record_old(ep,dt,op->ttl)) > 0) {
	            f = TRUE ;
	            if ((rs = upwcache_recdel(op,ep)) >= 0) {
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
/* end subroutine (upwcache_check) */


int upwcache_stats(UPWCACHE *op,UPWCACHE_STATS *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != UPWCACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = recarr_count(op->recs)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}

	return rs ;
}
/* end subroutine (upwcache_stats) */


/* private subroutines */


static int upwcache_fetch(UPWCACHE *op,UPWCACHE_REC **epp,const char *un)
{
	RECARR		*rlp = op->recs ;
	UPWCACHE_REC	*ep ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("upwcache_fetch: u=%s\n",un) ;
#endif

	for (i = 0 ; (rs = recarr_get(rlp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (un[0] == ep->un[0]) {
	            if (strcmp(un,ep->un) == 0) break ;
	        }
	    }
	} /* end for */

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ep : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("upwcache_fetch: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (upwcache_fetch) */


static int upwcache_mkrec(UPWCACHE *op,time_t dt,UPWCACHE_REC **epp,cchar un[])
{
	PQ_ENT		*pep ;
	int		rs ;
	int		rs1 ;
	int		pwl = 0 ;

#if	CF_DEBUGS
	debugprintf("upwcache_mkrec: u=%s\n",un) ;
#endif

	*epp = NULL ;
	if ((rs = recarr_count(op->recs)) >= 0) {
	    int	n = rs ;

	    if (n >= op->max) {

#if	CF_DEBUGS
		debugprintf("upwcache_mkrec: at max\n") ;
#endif

	        if ((rs = pq_rem(&op->lru,&pep)) >= 0) {
	            UPWCACHE_REC	*ep = (UPWCACHE_REC *) pep ;

	            if ((rs = upwcache_recdel(op,ep)) >= 0) {
	                rs = upwcache_recstart(op,dt,ep,un) ;
	                pwl = rs ;
	            }

	            rs1 = pq_ins(&op->lru,pep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (removed entry) */

	        if (rs >= 0) *epp = (UPWCACHE_REC *) pep ;

	    } else {

#if	CF_DEBUGS
		debugprintf("upwcache_mkrec: < max\n") ;
#endif

	        if ((rs = upwcache_newrec(op,dt,epp,un)) >= 0) {
	            pwl = rs ;
	            if (*epp != NULL) {
	                pep = (PQ_ENT *) *epp ;
	                rs = pq_ins(&op->lru,pep) ;
	            }
	        } /* end if (new-entry) */

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("upwcache_mkrec: ret rs=%d pwl=%u\n",rs,pwl) ;
#endif

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (upwcache_mkrec) */


static int upwcache_newrec(UPWCACHE *op,time_t dt,UPWCACHE_REC **epp,cchar un[])
{
	UPWCACHE_REC	*ep ;
	const int	size = sizeof(UPWCACHE_REC) ;
	int		rs ;

	if (epp == NULL) return SR_NOANODE ;

#if	CF_DEBUGS
	debugprintf("upwcache_newrec: ent\n") ;
#endif

	if ((rs = uc_libmalloc(size,&ep)) >= 0) {
	    rs = upwcache_recstart(op,dt,ep,un) ;
	    if (rs < 0)
	        uc_libfree(ep) ;
	} /* end if (m-a) */

	*epp = (rs >= 0) ? ep : NULL ;

#if	CF_DEBUGS
	debugprintf("upwcache_newrec: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (upwcache_newrec) */


static int upwcache_recstart(UPWCACHE *op,time_t dt,UPWCACHE_REC *ep,cchar un[])
{
	const int	wc = op->wcount++ ;
	int		rs ;
	int		pwl = 0 ;

#if	CF_DEBUGS
	debugprintf("upwcache_recstart: ent\n") ;
#endif

	if ((rs = record_start(ep,dt,wc,un)) >= 0) {
	    pwl = rs ;
	    rs = recarr_add(op->recs,ep) ;
	    ep->recidx = rs ;
	    if (rs < 0)
	        record_finish(ep) ;
	} /* end if (entry-start) */

#if	CF_DEBUGS
	debugprintf("upwcache_recstart: ret rs=%d pwl=%u\n",rs,pwl) ;
#endif

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (upwcache_recstart) */


static int upwcache_recaccess(UPWCACHE *op,time_t dt,UPWCACHE_REC *ep)
{
	int		rs ;

	if ((rs = upwcache_recrear(op,ep)) >= 0) {
	    if ((rs = record_old(ep,dt,op->ttl)) > 0) {
	        int	wc = op->wcount++ ;
	        op->s.refreshes += 1 ;
	        rs = record_refresh(ep,dt,wc) ;
	    } else {
	        rs = record_access(ep,dt) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (upwcache_recaccess) */


static int upwcache_recrear(UPWCACHE *op,UPWCACHE_REC *ep)
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
	                UPWCACHE_REC	*ep = (UPWCACHE_REC *) pep ;
	                record_finish(ep) ;
	                uc_libfree(pep) ;
	            }
	        }
	    }
	} /* end if (pq-gettail) */

	return rs ;
}
/* end subroutine (upwcache_recrear) */


static int upwcache_recdel(UPWCACHE *op,UPWCACHE_REC *ep)
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
/* end subroutine (upwcache_recdel) */


static int upwcache_recfins(UPWCACHE *op)
{
	RECARR		*rlp = op->recs ;
	UPWCACHE_REC	*ep ;
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
/* end subroutine (upwcache_recfins) */


static int upwcache_upstats(UPWCACHE *op,int ct,int rs)
{
	int		f_got = (rs > 0) ;
#if	CF_DEBUGS
	debugprintf("upwcache_upstats: ct=%u rs=%d\n",ct,rs) ;
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
#if	CF_DEBUGS
	debugprintf("upwcache_upstats: ret\n") ;
#endif
	return SR_OK ;
}
/* end subroutine (upwcache_upstats) */


static int record_start(UPWCACHE_REC *ep,time_t dt,int wc,cchar *un)
{
	int		rs ;
	int		rs1 ;
	int		pwl = 0 ;

	if (ep == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	memset(ep,0,sizeof(UPWCACHE_REC)) ;

	strwcpy(ep->un,un,USERNAMELEN) ;

	if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = rs ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs1 = uc_getpwnam(un,&pw,pwbuf,pwlen)) >= 0) {
	            char	*pwb ;
	            pwl = rs1 ;
	            if ((rs = uc_libmalloc((pwl+1),&pwb)) >= 0) {
	                if ((rs = passwdent_load(&ep->pw,pwb,pwl,&pw)) >= 0) {
	                    ep->pwbuf = pwb ;
	                    ep->pwlen = pwl ;
	                    ep->pwl = pwl ;
	                }
	                if (rs < 0)
	                    uc_libfree(pwb) ;
	            } /* end if (memory-allocation) */
	        } else if (rs1 == SR_NOTFOUND) {
	            ep->pwl = 0 ; /* optional */
	            pwl = 0 ; /* indicates an empty (not-found) entry */
	        } else {
	            rs = rs1 ;
	        }
	        rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs ;
	    } /* end if (m-a) */
	} /* end if (getbufsize) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	}

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_start) */


static int record_finish(UPWCACHE_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->pwbuf != NULL) {
	    rs1 = uc_libfree(ep->pwbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->pwbuf = NULL ;
	    ep->pwlen = 0 ;
	}

	ep->pwl = 0 ;
	ep->un[0] = '\0' ;
	return rs ;
}
/* end subroutine (record_finish) */


static int record_access(UPWCACHE_REC *ep,time_t dt)
{
	int		rs = SR_OK ;
	int		pwl = 0 ;

	if (ep == NULL) return SR_FAULT ;

	ep->ti_access = dt ;
	pwl  = ep->pwl ;
	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_access) */


static int record_refresh(UPWCACHE_REC *ep,time_t dt,int wc)
{
	int		rs ;
	int		rs1 ;
	int		pwl = 0 ;

	if (ep == NULL) return SR_FAULT ;

	if ((rs = getbufsize(getbufsize_pw)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = rs ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs1 = uc_getpwnam(ep->un,&pw,pwbuf,pwlen)) >= 0) {
	            pwl = rs1 ;

	            ep->pwl = pwl ;
	            if ((ep->pwbuf == NULL) || (ep->pwlen < pwl)) {
	                void	*p ;

	                if (ep->pwbuf != NULL) {
	                    rs = uc_librealloc(ep->pwbuf,(pwl+1),&p) ;
	                } else {
	                    rs = uc_libmalloc((pwl+1),&p) ;
	                }

	                if (rs >= 0) {
	                    ep->pwbuf = (char *) p ;
	                    ep->pwlen = pwl ;
	                } /* end if (m-a) */

	            } /* end if (re-use slot or allocate slot) */

	            if (rs >= 0) {
	                rs = passwdent_load(&ep->pw,ep->pwbuf,ep->pwlen,&pw) ;
		    }

	        } else if (rs1 == SR_NOTFOUND) {
	            if (ep->pwbuf != NULL) {
	                uc_libfree(ep->pwbuf) ;
	                ep->pwbuf = NULL ;
	                ep->pwlen = 0 ;
	            }
	            ep->pwl = 0 ;
	            pwl = 0 ; /* indicates an empty (not-found) entry */
	        } else {
	            rs = rs1 ;
	        }
	        rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	} /* end if (getbufsize) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	}

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_refresh) */


static int record_old(UPWCACHE_REC *ep,time_t dt,int ttl)
{
	int		f_old ;

	if (ep == NULL) return SR_FAULT ;

	f_old = ((dt - ep->ti_create) >= ttl) ;
	return f_old ;
}
/* end subroutine (record_old) */


