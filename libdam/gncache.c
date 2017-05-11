/* gncache */

/* group-name cache */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


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
#include	<vechand.h>
#include	<cq.h>
#include	<localmisc.h>

#include	"gncache.h"


/* local defines */

#define	GNCACHE_REC	struct gncache_r

#define	TO_CHECK	5


/* external subroutines */

extern int	getgroupname(char *,int,gid_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

enum cts {
	ct_miss,
	ct_hit,
	ct_overlast
} ;

struct gncache_r {
	time_t		ti_create ;		/* creation time */
	time_t		ti_access ;		/* last access time */
	gid_t		gid ;
	char		gn[GROUPNAMELEN + 1] ;
} ;


/* forward references */

static int gncache_searchgid(GNCACHE *,GNCACHE_REC **,gid_t) ;
static int gncache_newrec(GNCACHE *,time_t,GNCACHE_REC **,gid_t,cchar *) ;
static int gncache_recaccess(GNCACHE *,GNCACHE_REC *,time_t) ;
static int gncache_allocrec(GNCACHE *,GNCACHE_REC **) ;
static int gncache_recfree(GNCACHE *,GNCACHE_REC *) ;
static int gncache_maintenance(GNCACHE *,time_t) ;
static int gncache_record(GNCACHE *,int,int) ;

#ifdef	COMMENT
static int gncache_recdel(GNCACHE *,GNCACHE_REC *) ;
#endif

static int record_start(GNCACHE_REC *,time_t,gid_t,const char *) ;
static int record_old(GNCACHE_REC *,time_t,int) ;
static int record_refresh(GNCACHE_REC *,time_t) ;
static int record_update(GNCACHE_REC *,time_t,const char *) ;
static int record_access(GNCACHE_REC *,time_t) ;
static int record_finish(GNCACHE_REC *) ;

static int	entry_load(GNCACHE_ENT *,GNCACHE_REC *) ;


/* local variables */


/* exported subroutines */


int gncache_start(GNCACHE *op,int max,int to)
{
	const int	defnum = GNCACHE_DEFENT ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < 4) max = GNCACHE_DEFMAX ;

	if (to < 1) to = GNCACHE_DEFTTL ;

	memset(op,0,sizeof(GNCACHE)) ;

	if ((rs = cq_start(&op->recsfree)) >= 0) {
	    if ((rs = vechand_start(&op->recs,defnum,0)) >= 0) {
	        op->max = max ;
	        op->ttl = to ;
	        op->ti_check = time(NULL) ;
	        op->magic = GNCACHE_MAGIC ;
	    }
	    if (rs < 0)
	        cq_finish(&op->recsfree) ;
	} /* end if (cq-start) */

	return rs ;
}
/* end subroutine (gncache_start) */


int gncache_finish(GNCACHE *op)
{
	GNCACHE_REC	*rp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	void		*vp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GNCACHE_MAGIC) return SR_NOTOPEN ;

/* loop freeing up all cache entries */

	for (i = 0 ; vechand_get(&op->recs,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;
	    rs1 = record_finish(rp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(rp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end while */
	if (rs >= 0) rs = rs1 ;

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
/* end subroutine (gncache_finish) */


int gncache_add(GNCACHE *op,gid_t gid,cchar gn[])
{
	GNCACHE_REC	*rp ;
	time_t		dt = time(NULL) ;
	int		rs = SR_OK ;
	int		gl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (gn == NULL) return SR_FAULT ;

	if (op->magic != GNCACHE_MAGIC) return SR_NOTOPEN ;

	if (gid < 0) return SR_INVALID ;

	if (gn[0] == '\0') return SR_INVALID ;

	if ((rs = gncache_searchgid(op,&rp,gid)) >= 0) {
	    gl = rs ;
	    rs = record_update(rp,dt,gn) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = gncache_newrec(op,dt,NULL,gid,gn) ;
	    gl = rs ;
	}

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (gncache_add) */


int gncache_lookgid(GNCACHE *op,GNCACHE_ENT *ep,gid_t gid)
{
	GNCACHE_REC	*rp ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		ct ;
	int		gl = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GNCACHE_MAGIC) return SR_NOTOPEN ;

	if (gid < 0) return SR_INVALID ;

	if ((rs = gncache_searchgid(op,&rp,gid)) >= 0) {
	    ct = ct_hit ;
	    rs = gncache_recaccess(op,rp,dt) ;
	    gl = rs ;
	} else if (rs == SR_NOTFOUND) {
	    char	gn[GROUPNAMELEN + 1] ;
	    ct = ct_miss ;
	    if ((rs = getgroupname(gn,GROUPNAMELEN,gid)) >= 0) {
	        rs = gncache_newrec(op,dt,&rp,gid,gn) ;
	        gl = rs ;
	    }
	} /* end if (search-gid) */

	gncache_record(op,ct,rs) ;
	if (rs == 0) rs = SR_NOTFOUND ;

	if (rs >= 0) {
	    if (ep != NULL) rs = entry_load(ep,rp) ;
	    gncache_maintenance(op,dt) ;
	} /* end if */

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (gncache_lookgid) */


int gncache_stats(GNCACHE *op,GNCACHE_STATS *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != GNCACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = vechand_count(&op->recs)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}
	return rs ;
}
/* end subroutine (gncache_stats) */


int gncache_check(GNCACHE *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GNCACHE_MAGIC) return SR_NOTOPEN ;

	if (dt == 0)
	    dt = time(NULL) ;

	if ((dt - op->ti_check) >= TO_CHECK) {
	    f = TRUE ;
	    op->ti_check = dt ;
	    rs = gncache_maintenance(op,dt) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (gncache_check) */


/* private subroutines */


static int gncache_newrec(GNCACHE *op,time_t dt,GNCACHE_REC **rpp,gid_t gid,
		cchar *gn)
{
	GNCACHE_REC	*rp ;
	int		rs ;
	int		gl = 0 ;

	if ((rs = gncache_allocrec(op,&rp)) >= 0) {
	    if ((rs = record_start(rp,dt,gid,gn)) >= 0) {
	        gl = rs ;
	        rs = vechand_add(&op->recs,rp) ;
	        if (rs < 0)
	            record_finish(rp) ;
	    } /* end if (record-start) */
	    if (rs < 0)
	        uc_free(rp) ;
	} /* end if */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? rp : NULL ;

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (gncache_newrec) */


static int gncache_recaccess(GNCACHE *op,GNCACHE_REC *rp,time_t dt)
{
	int		rs ;
	int		gl = 0 ;

	if ((rs = record_old(rp,dt,op->ttl)) > 0) {
	    rs = record_refresh(rp,dt) ;
	    gl = rs ;
	} else {
	    rs = record_access(rp,dt) ;
	    gl = rs ;
	}

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (gncache_recaccess) */


static int gncache_searchgid(GNCACHE *op,GNCACHE_REC **rpp,gid_t gid)
{
	int		rs = SR_OK ;
	int		i ;
	int		gl = 0 ;

	for (i = 0 ; (rs = vechand_get(&op->recs,i,rpp)) >= 0 ; i += 1) {
	    if (*rpp == NULL) continue ;
	    if ((*rpp)->gid == gid)
	        break ;
	} /* end for */

	if (rs >= 0)
	    gl = strlen((*rpp)->gn) ;

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (gncache_searchgid) */


static int gncache_maintenance(GNCACHE *op,time_t dt)
{
	GNCACHE_REC	*rp ;
	time_t		ti_oldest = LONG_MAX ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		n ;
	int		iold = -1 ; /* the oldest one */
	int		i ;

/* delete expired entries */

	for (i = 0 ; vechand_get(&op->recs,i,&rp) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;
	    if ((dt - rp->ti_create) >= op->ttl) {
	        vechand_del(&op->recs,i) ;
	        record_finish(rp) ;
	        gncache_recfree(op,rp) ;
	    } else {
	        if (rp->ti_access < ti_oldest) {
	            ti_oldest = rp->ti_access ;
	            iold = i ;
	        }
	    }
	} /* end for */

/* delete entries (at least one) if we are too big */

	if ((rs >= 0) && (iold >= 0)) {
	    n = vechand_count(&op->recs) ;
	    if (n > op->max) {
	        rs1 = vechand_get(&op->recs,iold,&rp) ;
	        if ((rs1 >= 0) && (rp != NULL)) {
	            vechand_del(&op->recs,iold) ;
	            record_finish(rp) ;
	            gncache_recfree(op,rp) ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (gncache_maintenance) */


static int gncache_allocrec(GNCACHE *op,GNCACHE_REC **rpp)
{
	const int	size = sizeof(GNCACHE_REC) ;
	int		rs ;

	if ((rs = cq_rem(&op->recsfree,rpp)) == SR_NOTFOUND) {
	    void	*vp ;
	    rs = uc_malloc(size,&vp) ;
	    if (rs >= 0) *rpp = vp ;
	}

	return rs ;
}
/* end subroutine (gncache_allocrec) */


#ifdef	COMMENT
static int gncache_recdel(GNCACHE *op,GNCACHE_REC *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs1 = vechand_ent(&op->db,ep)) >= 0) {
	    rs1 = vechand_del(rs1) ;
	}
	if (rs >= 0) rs = rs1 ;

	rs1 = record_finish(ep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (gncache_recdel) */
#endif /* COMMENT */


static int gncache_recfree(GNCACHE *op,GNCACHE_REC *rp)
{
	int		rs = SR_OK ;
	int		n = cq_count(&op->recsfree) ;

	if (n < GNCACHE_MAXFREE) {
	    rs = cq_ins(&op->recsfree,rp) ;
	    if (rs < 0)
	        uc_free(rp) ;
	} else
	    uc_free(rp) ;

	return rs ;
}
/* end subroutine (gncache_recfree) */


static int gncache_record(GNCACHE *op,int ct,int rs)
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
/* end subroutine (gncache_record) */


static int record_start(GNCACHE_REC *rp,time_t dt,gid_t gid,cchar gn[])
{
	int		rs = SR_OK ;
	int		gl = 0 ;

	if (rp == NULL) return SR_FAULT ;
	if (gn == NULL) return SR_FAULT ;

	if (gn[0] == '\0') return SR_INVALID ;

	if (dt == 0)
	    dt = time(NULL) ;

	memset(rp,0,sizeof(GNCACHE_REC)) ;

	rp->gid = gid ;
	rp->ti_create = dt ;
	rp->ti_access = dt ;
	gl = strwcpy(rp->gn,gn,GROUPNAMELEN) - rp->gn ;

	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (record_start) */


static int record_finish(GNCACHE_REC *rp)
{

	if (rp == NULL) return SR_FAULT ;

	rp->gid = -1 ;
	rp->gn[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (record_finish) */


static int record_old(GNCACHE_REC *rp,time_t dt,int ttl)
{
	int		f = FALSE ;

	if (rp == NULL) return SR_FAULT ;

	f = ((dt - rp->ti_create) >= ttl) ;
	return f ;
}
/* end subroutine (record_old) */


static int record_refresh(GNCACHE_REC *rp,time_t dt)
{
	int		rs ;
	int		gl = 0 ;
	char		gn[GROUPNAMELEN + 1] ;
	if ((rs = getgroupname(gn,GROUPNAMELEN,rp->gid)) >= 0) {
	    gl = rs ;
	    rs = record_update(rp,dt,gn) ;
	}
	return (rs >= 0) ? gl : rs ;
}
/* end subroutine (record_refresh) */


static int record_update(GNCACHE_REC *rp,time_t dt,cchar gn[])
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (rp == NULL) return SR_FAULT ;

	rp->ti_create = dt ;
	rp->ti_access = dt ;
	f_changed = (strcmp(rp->gn,gn) != 0) ;

	if (f_changed)
	    strwcpy(rp->gn,gn,GROUPNAMELEN) ;

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (record_update) */


static int record_access(GNCACHE_REC *rp,time_t dt)
{
	int		gl ;

	rp->ti_access = dt ;
	gl = strlen(rp->gn) ;

	return gl ;
}
/* end subroutine (record_access) */


static int entry_load(GNCACHE_ENT *ep,GNCACHE_REC *rp)
{
	const int	gnl = GROUPNAMELEN ;
	int		gl ;

	ep->gid = rp->gid ;
	gl = strwcpy(ep->groupname,rp->gn,gnl) - ep->groupname ;
	return gl ;
}
/* end subroutine (entry_load) */


