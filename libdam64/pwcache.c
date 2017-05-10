/* pwcache */

/* PASSWD cache */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides a crude cache for PASSWD-DB entries.


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
#include	<getax.h>
#include	<ugetpw.h>
#include	<passwdent.h>
#include	<localmisc.h>

#include	"pwcache.h"


/* local defines */

#ifndef	PWCACHE_REC
#define	PWCACHE_REC	struct pwcache_r
#endif

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

enum cts {
	ct_miss,
	ct_hit,
	ct_overlast
} ;

struct pwcache_r {
	PQ_ENT		linkage ;
	char		*pwbuf ;
	struct passwd	pw ;
	time_t		ti_create ;
	time_t		ti_access ;
	uint		wcount ;
	int		pwl ;
	char		un[USERNAMELEN+1] ;
} ;


/* forward references */

static int	pwcache_mkrec(PWCACHE *,time_t,PWCACHE_REC **,
			const char *) ;
static int	pwcache_newrec(PWCACHE *,time_t,PWCACHE_REC **,
			const char *) ;
static int	pwcache_recstart(PWCACHE *,time_t,PWCACHE_REC *,
			const char *) ;
static int	pwcache_recdel(PWCACHE *,PWCACHE_REC *) ;
static int	pwcache_recaccess(PWCACHE *,time_t,PWCACHE_REC *) ;
static int	pwcache_recrear(PWCACHE *,PWCACHE_REC *) ;
static int	pwcache_recfins(PWCACHE *) ;
static int	pwcache_record(PWCACHE *,int,int) ;

static int record_start(PWCACHE_REC *,time_t,int,const char *) ;
static int record_access(PWCACHE_REC *,time_t) ;
static int record_refresh(PWCACHE_REC *,time_t,int) ;
static int record_old(PWCACHE_REC *,time_t,int) ;
static int record_finish(PWCACHE_REC *) ;


/* local variables */


/* exported subroutines */


int pwcache_start(PWCACHE *op,int max,int ttl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < PWCACHE_DEFMAX)
	    max = PWCACHE_DEFMAX ;

	if (ttl < PWCACHE_DEFTTL)
	    ttl = PWCACHE_DEFTTL ;

	memset(op,0,sizeof(PWCACHE)) ;

	if ((rs = hdb_start(&op->db,max,1,NULL,NULL)) >= 0) {
	    if ((rs = pq_start(&op->lru)) >= 0) {
		op->max = max ;
		op->ttl = ttl ;
		op->ti_check = time(NULL) ;
		op->magic = PWCACHE_MAGIC ;
	    }
	    if (rs < 0)
		hdb_finish(&op->db) ;
	} /* end if (hdb-start) */

	return rs ;
}
/* end subroutine (pwcache_start) */


int pwcache_finish(PWCACHE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PWCACHE_MAGIC) return SR_NOTOPEN ;

/* finish up the LRU queue */

	rs1 = pq_finish(&op->lru) ;
	if (rs >= 0) rs = rs1 ;

/* loop freeing up all cache entries */

	rs1 = pwcache_recfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up everyting else */

	rs1 = hdb_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pwcache_finish) */


int pwcache_lookup(PWCACHE *op,PASSWDENT *pwp,char *pwbuf,int pwlen,cchar *un)
{
	HDB_DATUM	key, val ;
	PWCACHE_REC	*ep = NULL ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		ct ;

	if (op == NULL) return SR_FAULT ;
	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != PWCACHE_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pwcache_lookup: u=>%s<\n",un) ;
#endif

	op->s.total += 1 ;

	key.buf = un ;
	key.len = strlen(un) ;

	if ((rs = hdb_fetch(&op->db,key,NULL,&val)) >= 0) {
	    ct = ct_hit ;
	    ep = (PWCACHE_REC *) val.buf ;
	    rs = pwcache_recaccess(op,dt,ep) ;
	} else if (rs == SR_NOTFOUND) {
	    ct = ct_miss ;
	    rs = pwcache_mkrec(op,dt,&ep,un) ;
	} /* end if (hit or miss) */

	pwcache_record(op,ct,rs) ;

	if (rs > 0) { /* not '>=' */
	    rs = passwdent_load(pwp,pwbuf,pwlen,&ep->pw) ;
	} else if (rs == 0) {
	    rs = SR_NOTFOUND ;
	}

	if (rs <= 0)
	    memset(pwp,0,sizeof(struct passwd)) ;

	return rs ;
}
/* end subroutine (pwcache_lookup) */


int pwcache_invalidate(PWCACHE *op,cchar *un)
{
	HDB_DATUM	key, val ;
	PWCACHE_REC	*ep ;
	int		rs ;
	int		rs1 ;
	int		f_found = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != PWCACHE_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

	key.buf = un ;
	key.len = strlen(un) ;

	if ((rs = hdb_fetch(&op->db,key,NULL,&val)) >= 0) {
	    PQ_ENT	*pep ;

	    f_found = TRUE ;

	    ep = (PWCACHE_REC *) val.buf ;

	    pep = (PQ_ENT *) ep ;
	    rs1 = pq_unlink(&op->lru,pep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_delkey(&op->db,key) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = record_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;

	    uc_free(ep) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (pwcache_invalidate) */


int pwcache_check(PWCACHE *op,time_t dt)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	PWCACHE_REC	*ep ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PWCACHE_MAGIC) return SR_NOTOPEN ;

/* loop checking all cache entries */

	if ((rs = hdb_curbegin(&op->db,&cur)) >= 0) {
	    if (dt == 0) dt = time(NULL) ;

	    while (rs >= 0) {
	        rs1 = hdb_enum(&op->db,&cur,&key,&val) ;
		if (rs1 == SR_NOTFOUND) break ;

		rs = rs1 ;
		if (rs >= 0) {
	            ep = (PWCACHE_REC *) val.buf ;
	            if ((rs = record_old(ep,dt,op->ttl)) > 0) {
		        f = TRUE ;
		        if ((rs = pwcache_recdel(op,ep)) >= 0) {
		            PQ_ENT	*pep = (PQ_ENT *) ep ;
	                    rs = pq_unlink(&op->lru,pep) ;
			    uc_free(ep) ;
			}
		    } /* end if (entry-old) */
		}

	    } /* end while */

	    hdb_curend(&op->db,&cur) ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pwcache_check) */


int pwcache_stats(PWCACHE *op,PWCACHE_STATS *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != PWCACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = hdb_count(&op->db)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}

	return rs ;
}
/* end subroutine (pwcache_stats) */


/* private subroutines */


static int pwcache_mkrec(PWCACHE *op,time_t dt,PWCACHE_REC **epp,cchar *un)
{
	PQ_ENT		*pep ;
	int		rs ;
	int		rs1 ;
	int		pwl = 0 ;

	*epp = NULL ;
	if ((rs = hdb_count(&op->db)) >= 0) {
	    int	n = rs ;

	    if (n >= op->max) {

	        if ((rs = pq_rem(&op->lru,&pep)) >= 0) {
		    PWCACHE_REC	*ep = (PWCACHE_REC *) pep ;

		    if ((rs = pwcache_recdel(op,ep)) >= 0) {
		        rs = pwcache_recstart(op,dt,ep,un) ;
		        pwl = rs ;
	            }

	            rs1 = pq_ins(&op->lru,pep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (removed entry) */

	        if (rs >= 0) *epp = (PWCACHE_REC *) pep ;

	    } else {

	        if ((rs = pwcache_newrec(op,dt,epp,un)) >= 0) {
	            pwl = rs ;
	            if (*epp != NULL) {
		        pep = (PQ_ENT *) *epp ;
	                rs = pq_ins(&op->lru,pep) ;
		    }
	        } /* end if (new-entry) */

	    } /* end if */

	} /* end if */

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (pwcache_mkrec) */


static int pwcache_newrec(PWCACHE *op,time_t dt,PWCACHE_REC **epp,cchar *un)
{
	PWCACHE_REC	*ep ;
	const int	size = sizeof(PWCACHE_REC) ;
	int		rs ;

	if (epp == NULL) return SR_NOANODE ;

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    rs = pwcache_recstart(op,dt,ep,un) ;
	    if (rs < 0) uc_free(ep) ;
	}

	*epp = (rs >= 0) ? ep : NULL ;
	return rs ;
}
/* end subroutine (pwcache_newrec) */


static int pwcache_recstart(PWCACHE *op,time_t dt,PWCACHE_REC *ep,cchar	*un)
{
	const int	wc = op->wcount++ ;
	int		rs ;
	int		pwl = 0 ;

	if ((rs = record_start(ep,dt,wc,un)) >= 0) {
	    HDB_DATUM	key, val ;
	    const int	size = sizeof(PWCACHE_REC) ;
	    pwl = rs ;
	    key.buf = ep->un ;
	    key.len = strlen(ep->un) ;
	    val.buf = ep ;
	    val.len = size ;
	    rs = hdb_store(&op->db,key,val) ;
	    if (rs < 0)
	        record_finish(ep) ;
	} /* end if (entry-start) */

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (pwcache_recstart) */


static int pwcache_recaccess(PWCACHE *op,time_t dt,PWCACHE_REC *ep)
{
	int		rs ;

	if ((rs = pwcache_recrear(op,ep)) >= 0) {
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
/* end subroutine (pwcache_recaccess) */


static int pwcache_recrear(PWCACHE *op,PWCACHE_REC *ep)
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
			PWCACHE_REC	*ep = (PWCACHE_REC *) pep ;
			record_finish(ep) ;
			uc_free(pep) ;
		    }
		}
	    }
	} /* end if (pq-gettail) */

	return rs ;
}
/* end subroutine (pwcache_recrear) */


static int pwcache_recdel(PWCACHE *op,PWCACHE_REC *ep)
{
	HDB_DATUM	key ;
	int		rs = SR_OK ;
	int		rs1 ;

	key.buf = ep->un ;
	key.len = strlen(ep->un) ;

	rs1 = hdb_delkey(&op->db,key) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = record_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pwcache_recdel) */


static int pwcache_recfins(PWCACHE *op)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	PWCACHE_REC	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs1 = hdb_curbegin(&op->db,&cur)) >= 0) {
	    while (hdb_enum(&op->db,&cur,&key,&val) >= 0) {
	        ep = (PWCACHE_REC *) val.buf ;
	        rs1 = record_finish(ep) ;
		if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    hdb_curend(&op->db,&cur) ;
	} /* end if */
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pwcache_recfins) */


static int pwcache_record(PWCACHE *op,int ct,int rs)
{
	int		f_got = (rs > 0) ;
#if	CF_DEBUGS
	debugprintf("pwcache_record: ct=%u rs=%d\n",ct,rs) ;
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
/* end subroutine (pwcache_record) */


static int record_start(ep,dt,wc,un)
PWCACHE_REC	*ep ;
time_t		dt ;
int		wc ;
const char	un[] ;
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pwl = 0 ;
	char		*pwbuf ;

	if (ep == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	memset(ep,0,sizeof(PWCACHE_REC)) ;

	strwcpy(ep->un,un,USERNAMELEN) ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	if ((rs1 = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
	    void	*p ;
	    pwl = rs1 ;

	    if ((rs = uc_malloc((pwl+1),&p)) >= 0) {
		char	*pwbuf = (char *) p ;
		if ((rs = passwdent_load(&ep->pw,pwbuf,pwl,&pw)) >= 0) {
	            ep->pwbuf = pwbuf ;
	    	    ep->pwl = pwl ;
		}
		if (rs < 0) uc_free(p) ;
	    } /* end if (memory-allocation) */

	} else if (rs1 == SR_NOTFOUND) {
	    ep->pwl = 0 ; /* optional */
	    pwl = 0 ; /* indicates an empty (not-found) entry */
	} else {
	    rs = rs1 ;
	}
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	}

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_start) */


static int record_finish(ep)
PWCACHE_REC	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->pwbuf != NULL) {
	    rs1 = uc_free(ep->pwbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->pwbuf = NULL ;
	}

	ep->pwl = 0 ;
	ep->un[0] = '\0' ;
	return rs ;
}
/* end subroutine (record_finish) */


static int record_access(ep,dt)
PWCACHE_REC	*ep ;
time_t		dt ;
{
	int		rs = SR_OK ;
	int		pwl = 0 ;

	if (ep == NULL) return SR_FAULT ;

	ep->ti_access = dt ;
	pwl  = ep->pwl ;
	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_access) */


static int record_refresh(ep,dt,wc)
PWCACHE_REC	*ep ;
time_t		dt ;
int		wc ;
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pwl = 0 ;
	char		*pwbuf ;

	if (ep == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	if ((rs1 = GETPW_NAME(&pw,pwbuf,pwlen,ep->un)) >= 0) {
	    void	*p ;
	    pwl = rs1 ;

	    ep->pwl = pwl ;
	    if (ep->pwbuf != NULL) {
	        rs = uc_realloc(ep->pwbuf,(pwl+1),&p) ;
	    } else
	        rs = uc_malloc((pwl+1),&p) ;

	    if (rs >= 0) {
		char	*pwbuf = (char *) p ;
	        ep->pwbuf = (char *) p ;
		rs = passwdent_load(&ep->pw,pwbuf,pwl,&pw) ;
		if (rs < 0) uc_free(p) ;
	    }

	} else if (rs1 == SR_NOTFOUND) {
	    if (ep->pwbuf != NULL) {
		uc_free(ep->pwbuf) ;
		ep->pwbuf = NULL ;
	    }
	    ep->pwl = 0 ;
	    pwl = 0 ; /* indicates an empty (not-found) entry */
	} else {
	    rs = rs1 ;
	}
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	}

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (record_refresh) */


static int record_old(PWCACHE_REC *ep,time_t dt,int ttl)
{
	int		f_old ;

	if (ep == NULL) return SR_FAULT ;

	f_old = ((dt - ep->ti_create) >= ttl) ;
	return f_old ;
}
/* end subroutine (record_old) */


