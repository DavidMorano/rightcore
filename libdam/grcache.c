/* grcache */

/* GROUP cache */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SEARCHGID	0		/* compile in 'searchgid()' */
#define	CF_MAINTEXTRA	0		/* perform extra maintenance */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides a crude cache for GROUP-DB entries.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vechand.h>
#include	<cq.h>
#include	<getax.h>
#include	<groupent.h>
#include	<localmisc.h>

#include	"grcache.h"


/* local defines */

#define	GRCACHE_RECMAGIC	0x98643163
#define	GRCACHE_REC		struct grcache_r

#define	TO_CHECK		5


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

enum cts {
	ct_miss,
	ct_hit,
	ct_overlast
} ;

struct grcache_r {
	uint		magic ;
	char		*grbuf ;
	struct group	gr ;
	time_t		ti_create ;		/* creation time */
	time_t		ti_access ;		/* access time (last) */
	gid_t		gid ;
	int		wcount ;
	int		grl ;
	char		gn[GROUPNAMELEN+1] ;
} ;


/* forward references */

static int grcache_searchname(GRCACHE *,GRCACHE_REC **,const char *) ;
static int grcache_mkrec(GRCACHE *,time_t,GRCACHE_REC **,const char *) ;
static int grcache_getrec(GRCACHE *,time_t,GRCACHE_REC **) ;
static int grcache_allocrec(GRCACHE *,GRCACHE_REC **) ;
static int grcache_recstart(GRCACHE *,time_t,GRCACHE_REC *,const char *) ;
static int grcache_recrear(GRCACHE *,GRCACHE_REC *) ;
static int grcache_recaccess(GRCACHE *,time_t,GRCACHE_REC *) ;
static int grcache_recdel(GRCACHE *,int,GRCACHE_REC *) ;
static int grcache_recfree(GRCACHE *,GRCACHE_REC *) ;
static int grcache_maintenance(GRCACHE *,time_t) ;
static int grcache_record(GRCACHE *,int,int) ;

#if	CF_SEARCHGID
static int grcache_searchgid(GRCACHE *,GRCACHE_REC **,gid_t) ;
#endif /* CF_SEARCHGID */

static int record_start(GRCACHE_REC *,time_t,int,const char *) ;
static int record_access(GRCACHE_REC *,time_t) ;
static int record_refresh(GRCACHE_REC *,time_t,int) ;
static int record_old(GRCACHE_REC *,time_t,int) ;
static int record_finish(GRCACHE_REC *) ;


/* local variables */


/* exported subroutines */


int grcache_start(GRCACHE *op,int max,int ttl)
{
	const int	defnum = GRCACHE_DEFENTS ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < 4) max = GRCACHE_DEFMAX ;

	if (ttl < 1) ttl = GRCACHE_DEFTTL ;

	memset(op,0,sizeof(GRCACHE)) ;

	if ((rs = cq_start(&op->recsfree)) >= 0) {
	    if ((rs = vechand_start(&op->recs,defnum,0)) >= 0)
		op->max = max ;
		op->ttl = ttl ;
		op->ti_check = time(NULL) ;
		op->magic = GRCACHE_MAGIC ;
	    if (rs < 0)
		cq_finish(&op->recsfree) ;
	} /* end if (cq-start) */

	return rs ;
}
/* end subroutine (grcache_start) */


int grcache_finish(GRCACHE *op)
{
	GRCACHE_REC	*rp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	void		*vp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GRCACHE_MAGIC) return SR_NOTOPEN ;

/* loop freeing up all cache entries */

	for (i = 0 ; vechand_get(&op->recs,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;
	    record_finish(rp) ;
	    uc_free(rp) ;
	} /* end while */

	rs1 = vechand_finish(&op->recs) ;
	if (rs >= 0) rs = rs1 ;

	while (cq_rem(&op->recsfree,&vp) >= 0) {
	    rs1 = uc_free(vp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = cq_finish(&op->recsfree) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (grcache_finish) */


int grcache_lookname(GRCACHE *op,GROUPENT *grp,char *grbuf,int grlen,cchar *gn)
{
	GRCACHE_REC	*rp ;
	time_t		dt = time(NULL) ;
	int		rs ;
	int		ct ;

	if (op == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;
	if (grbuf == NULL) return SR_FAULT ;
	if (gn == NULL) return SR_FAULT ;

	if (op->magic != GRCACHE_MAGIC) return SR_NOTOPEN ;

	if (gn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("grcache_lookname: gn=%s\n",gn) ;
#endif

	op->s.total += 1 ;

	if ((rs = grcache_searchname(op,&rp,gn)) >= 0) {
	    ct = ct_hit ;
	    rs = grcache_recaccess(op,dt,rp) ;
	} else if (rs == SR_NOTFOUND) {
	    ct = ct_miss ;
	    rs = grcache_mkrec(op,dt,&rp,gn) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("grcache_lookname: mid rs=%d\n",rs) ;
#endif

	grcache_record(op,ct,rs) ;

	if (rs > 0) {
	    rs = group_load(grp,grbuf,grlen,&rp->gr) ;
#if	CF_MAINTEXTRA
	    if ((dt - op->ti_check) >= TO_CHECK) {
	        op->ti_check = dt ;
	        grcache_maintenance(op,dt) ;
	    }
#endif /* CF_MAINTEXTRA */
	} else if (rs == 0) {
	    rs = SR_NOTFOUND ;
	} /* end if */

	if (rs <= 0)
	    memset(grp,0,sizeof(struct group)) ;

#if	CF_DEBUGS
	debugprintf("grcache_lookname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (grcache_lookname) */


#ifdef	COMMENT
int grcache_lookgid(GRCACHE *op,GROUPENT *grp,char *grbuf,int grlen,gid_t gid)
{
	GRCACHE_REC	*rp ;
	time_t		dt = time(NULL) ;
	int		rs = SR_OK ;
	int		grl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;
	if (grbuf == NULL) return SR_FAULT ;

	if (op->magic != GRCACHE_MAGIC) return SR_NOTOPEN ;

	if (gid < 0) return SR_INVALID ;

	op->s.total += 1 ;

	if ((rs = grcache_searchgid(op,&rp,gid)) >= 0) {
	    rs = grcache_recaccess(op,dt,rp) ;
	    grl = rs ;
	} else if (rs == SR_NOTFOUND) {
	    char	groupname[GROUPNAMELEN + 1] ;

	    if ((rs = getgroupname(groupname,GROUPNAMELEN,gid)) >= 0) {
	        rs = grcache_newrec(op,dt,&rp,gid,groupname) ;
	        grl = rs ;
	    }

	} /* end if */

	if (rs > 0) {
	    if (grp != NULL) rs = group_load(grp,grbuf,grlen,&rp->gr) ;
	    grcache_maintenance(op,dt) ;
	} /* end if */

	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (grcache_lookgid) */
#endif /* COMMENT */


int grcache_stats(GRCACHE *op,GRCACHE_STATS *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != GRCACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = vechand_count(&op->recs)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}

	return rs ;
}
/* end subroutine (grcache_stats) */


int grcache_check(GRCACHE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GRCACHE_MAGIC) return SR_NOTOPEN ;

	if (dt == 0)
	    dt = time(NULL) ;

	if ((dt - op->ti_check) >= TO_CHECK) {
	    f = TRUE ;
	    op->ti_check = dt ;
	    rs = grcache_maintenance(op,dt) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (grcache_check) */


/* private subroutines */


static int grcache_mkrec(GRCACHE *op,time_t dt,GRCACHE_REC **epp,cchar *gn)
{
	int		rs ;
	int		grl = 0 ;

	*epp = NULL ;
	if ((rs = vechand_count(&op->recs)) >= 0) {
	    int	n = rs ;
	    if (n >= op->max) {
	        rs = grcache_getrec(op,dt,epp) ;
	    } else {
	        rs = grcache_allocrec(op,epp) ;
	    } /* end if */
	    if (rs >= 0) {
	        rs = grcache_recstart(op,dt,*epp,gn) ;
		grl = rs ;
	    }
	} /* end if */

	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (grcache_mkrec) */


static int grcache_recstart(GRCACHE *op,time_t dt,GRCACHE_REC *ep,cchar *gn)
{
	const int	wc = op->wcount++ ;
	int		rs ;
	int		grl = 0 ;

	if ((rs = record_start(ep,dt,wc,gn)) >= 0) {
	    grl = rs ;
	    rs = vechand_add(&op->recs,ep) ;
	    if (rs < 0)
	        record_finish(ep) ;
	} /* end if (record-start) */

	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (grcache_recstart) */


static int grcache_recaccess(GRCACHE *op,time_t dt,GRCACHE_REC *ep)
{
	int		rs ;

	if ((rs = grcache_recrear(op,ep)) >= 0) {
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
/* end subroutine (grcache_recaccess) */


static int grcache_recrear(GRCACHE *op,GRCACHE_REC *ep)
{
	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (grcache_recrear) */


static int grcache_recdel(GRCACHE *op,int ri,GRCACHE_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ri >= 0) {
	    rs1 = vechand_del(&op->recs,ri) ;
	} else {
	    if ((rs1 = vechand_ent(&op->recs,ep)) >= 0) {
	        rs1 = vechand_del(&op->recs,rs1) ;
	    }
	}
	if (rs >= 0) rs = rs1 ;

	rs1 = record_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (grcache_recdel) */


static int grcache_searchname(GRCACHE *op,GRCACHE_REC **rpp,cchar *gn)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; (rs = vechand_get(&op->recs,i,rpp)) >= 0 ; i += 1) {
	    if (*rpp == NULL) continue ;
	    if ((*rpp)->gn[0] == gn[0]) {
		if (strcmp((*rpp)->gn,gn) == 0) break ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (grcache_searchname) */


#if	CF_SEARCHGID
static int grcache_searchgid(GRCACHE *op,GRCACHE_REC **rpp,gid_t gid)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; (rs = vechand_get(&op->recs,i,rpp)) >= 0 ; i += 1) {
	    if (*rpp == NULL) continue ;
	    if ((*rpp)->gid == gid) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (grcache_searchgid) */
#endif /* CF_SEARCHGID */


static int grcache_getrec(GRCACHE *op,time_t dt,GRCACHE_REC **rpp)
{
	GRCACHE_REC	*rp ;
	VECHAND		*rlp = &op->recs ;
	time_t		ti_oldest = LONG_MAX ;
	int		rs = SR_OK ;
	int		iold = -1 ;
	int		i ;
	int		f_exp = FALSE ;

	for (i = 0 ; vechand_get(rlp,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;

	    if (record_old(rp,dt,op->ttl) > 0) {
	        grcache_recdel(op,i,rp) ;
	        grcache_recfree(op,rp) ;
		f_exp = TRUE ;
	    } else {
	        if (rp->ti_access < ti_oldest) {
	            ti_oldest = rp->ti_access ;
	            iold = i ;
	        }
	    }

	    if (rs < 0) break ;
	} /* end for */

	if (rs >= 0) {
	    if (f_exp || (iold < 0)) {
		rs = grcache_allocrec(op,rpp) ;
	    } else {
	        if ((rs = vechand_get(rlp,iold,rpp)) >= 0) {
		    vechand_del(rlp,iold) ;
		    record_finish(*rpp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (grcache_getrec) */


static int grcache_maintenance(GRCACHE *op,time_t dt)
{
	GRCACHE_REC	*rp ;
	time_t		ti_oldest = LONG_MAX ;
	int		rs = SR_OK ;
	int		iold = -1 ;
	int		i ;

/* delete expired entries */

	for (i = 0 ; vechand_get(&op->recs,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;
	    if ((dt - rp->ti_create) >= op->ttl) {
	        grcache_recdel(op,i,rp) ;
	        grcache_recfree(op,rp) ;
	    } else {
	        if (rp->ti_access < ti_oldest) {
	            ti_oldest = rp->ti_access ;
	            iold = i ;
	        }
	    }
	} /* end for */

/* delete entries (at least one) if we are too big */

	if ((rs >= 0) && (iold >= 0)) {
	    int	n = vechand_count(&op->recs) ;
	    if (n > op->max) {
	        rs = vechand_get(&op->recs,iold,&rp) ;
	        if ((rs >= 0) && (rp != NULL)) {
	            grcache_recdel(op,iold,rp) ;
	            grcache_recfree(op,rp) ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (grcache_maintenance) */


static int grcache_allocrec(GRCACHE *op,GRCACHE_REC **rpp)
{
	int		rs ;

	if ((rs = cq_rem(&op->recsfree,rpp)) == SR_NOTFOUND) {
	    const int	size = sizeof(GRCACHE_REC) ;
	    void	*vp ;
	    if ((rs = uc_malloc(size,&vp)) >= 0) {
	        *rpp = vp ;
	    }
	}

	return rs ;
}
/* end subroutine (grcache_allocrec) */


static int grcache_recfree(GRCACHE *op,GRCACHE_REC *rp)
{
	int		rs ;

	if ((rs = cq_count(&op->recsfree)) >= 0) {
	    const int	n = rs ;
	    if (n < GRCACHE_MAXFREE) {
	        rs = cq_ins(&op->recsfree,rp) ;
	    } else {
	        uc_free(rp) ;
	    }
	}

	return rs ;
}
/* end subroutine (grcache_recfree) */


static int grcache_record(GRCACHE *op,int ct,int rs)
{
	int		f_got = (rs > 0) ;
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
/* end subroutine (grcache_record) */


static int record_start(rp,dt,wc,gn)
GRCACHE_REC	*rp ;
time_t		dt ;
int		wc ;
const char	gn[] ;
{
	const int	grlen = getbufsize(getbufsize_gr) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		grl = 0 ;
	char		*grbuf ;

	if (rp == NULL) return SR_FAULT ;
	if (gn == NULL) return SR_FAULT ;

	if (gn[0] == '\0') return SR_INVALID ;

	if (dt == 0) dt = time(NULL) ;

	memset(rp,0,sizeof(GRCACHE_REC)) ;

	if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	    struct group	gr ;
	    if ((rs1 = getgr_name(&gr,grbuf,grlen,gn)) >= 0) {
	        const int	size = (rs1+1) ;
	        void	*p ;
	        grl = rs1 ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
		    char	*grbuf = (char *) p ;
		    if ((rs = group_load(&rp->gr,grbuf,grl,&gr)) >= 0) {
	                rp->grbuf = grbuf ;
	    	        rp->grl = grl ;
		    }
		    if (rs < 0) uc_free(p) ;
	        } /* end if (memory-allocation) */
	    } else if (rs1 == SR_NOTFOUND) {
	        rp->grl = 0 ; /* optional */
	        grl = 0 ; /* indicates an empty (not-found) entry */
	    } else
	        rs = rs1 ;
	    uc_free(grbuf) ;
	} /* end if (memory-allocation) */

	if (rs >= 0) {
	    strwcpy(rp->gn,gn,GROUPNAMELEN) ;
	    rp->ti_create = dt ;
	    rp->ti_access = dt ;
	    rp->wcount = wc ;
	    rp->magic = GRCACHE_RECMAGIC ;
	}

	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (record_start) */


static int record_finish(rp)
GRCACHE_REC	*rp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (rp == NULL) return SR_FAULT ;

	if (rp->magic != GRCACHE_RECMAGIC) return SR_NOTFOUND ;

	if (rp->grbuf != NULL) {
	    rs1 = uc_free(rp->grbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    rp->grbuf = NULL ;
	}

	rp->grl = 0 ;
	rp->gid = -1 ;
	rp->gn[0] = '\0' ;
	rp->magic = 0 ;
	return rs ;
}
/* end subroutine (record_finish) */


static int record_access(ep,dt)
GRCACHE_REC	*ep ;
time_t		dt ;
{
	int		rs = SR_OK ;
	int		grl ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->magic != GRCACHE_RECMAGIC) return SR_NOTFOUND ;

	ep->ti_access = dt ;
	grl  = ep->grl ;
	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (record_access) */


static int record_refresh(ep,dt,wc)
GRCACHE_REC	*ep ;
time_t		dt ;
int		wc ;
{
	struct group	gr ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		grl = 0 ;
	char		*grbuf ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->magic != GRCACHE_RECMAGIC) return SR_NOTFOUND ;

	if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	    if ((rs1 = getgr_name(&gr,grbuf,grlen,ep->gn)) >= 0) {
	        void	*p ;
	        grl = rs1 ;
	        if (ep->grbuf != NULL) {
	            rs = uc_realloc(ep->grbuf,(grl+1),&p) ;
	        } else
	            rs = uc_malloc((grl+1),&p) ;
	        if (rs >= 0) {
		    char	*grbuf = (char *) p ;
	            ep->grbuf = (char *) p ;
	            ep->grl = grl ;
		    rs = group_load(&ep->gr,grbuf,grl,&gr) ;
		    if (rs < 0) uc_free(p) ;
	        }
	    } else if (rs1 == SR_NOTFOUND) {
	        if (ep->grbuf != NULL) {
		    uc_free(ep->grbuf) ;
		    ep->grbuf = NULL ;
	        }
	        ep->grl = 0 ; /* signal whatever? */
	        grl = 0 ; /* indicates an empty (not-found) entry */
	    } else
	        rs = rs1 ;
	    uc_free(grbuf) ;
	} /* end if (memory-allocation) */

	if (rs >= 0) {
	    ep->ti_create = dt ;
	    ep->ti_access = dt ;
	    ep->wcount = wc ;
	}

	return (rs >= 0) ? grl : rs ;
}
/* end subroutine (record_refresh) */


/* is it old? */
static int record_old(GRCACHE_REC *ep,time_t dt,int ttl)
{
	int		f_old ;
	if (ep == NULL) return SR_FAULT ;
	if (ep->magic != GRCACHE_RECMAGIC) return SR_NOTFOUND ;
	f_old = ((dt - ep->ti_create) >= ttl) ;
	return f_old ;
}
/* end subroutine (record_old) */


