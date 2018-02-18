/* grmems */

/* UNIX® group membership access and cache */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides a crude access manager and cache for UNIX® group
        membership lists.

	Implementation note:

        1. This was a rough (and somewhat crude) first attempt at this function.
        We are fairly efficient at every step but there may be one or two places
        where a sorted map might have been useful. But (so called) "n" is always
        relatively small so using structure sorted maps everywhere might have
        actually slowed things down also.

        2. The use of mapping the stored local 'passwd' data file can be changed
        to simple file reading if the need ever arises. We did not use the
        FILEMAP object (because at first I thought it too simple for the
        requirement), but this could be hacked in later (as indeed it turns out
        that object is appropriate) if you want to make the code look cleaner.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<passwdent.h>
#include	<groupent.h>
#include	<recarr.h>
#include	<vecelem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"grmems.h"


/* local defines */

#define	GRMEMS_DEFMAX		20	/* default maximum entries */
#define	GRMEMS_DEFTTL		(10*60)	/* default time-to-live */

#define	GRMEMS_REC		struct grmems_r
#define	GRMEMS_USER		struct grmems_u
#define	GRMEMS_USERGID		struct grmems_ug


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	isNotValid(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;


/* local structures */

enum cts {
	ct_miss,
	ct_hit,
	ct_overlast
} ;

struct grmems_r {
	PQ_ENT		linkage ;
	const char	**mems ;
	time_t		ti_create ;
	time_t		ti_access ;
	uint		wcount ;
	int		nmems ;
	int		recidx ;
	char		gn[GROUPNAMELEN+1] ;
} ;

struct grmems_u {
	const char	*up ;
	int		ul ;
} ;

struct grmems_ug {
	gid_t		gid ;
	char		un[USERNAMELEN] ;
} ;


/* forward references */

static int	grmems_starter(GRMEMS *) ;
static int	grmems_fetch(GRMEMS *,GRMEMS_REC **,const char *,int) ;
static int	grmems_mkrec(GRMEMS *,time_t,GRMEMS_REC **,cchar *,int) ;
static int	grmems_newrec(GRMEMS *,time_t,GRMEMS_REC **,cchar *,int) ;
static int	grmems_recstart(GRMEMS *,time_t,GRMEMS_REC *,cchar *,int) ;
static int	grmems_recrefresh(GRMEMS *,time_t,GRMEMS_REC *) ;
static int	grmems_recdel(GRMEMS *,GRMEMS_REC *) ;
static int	grmems_recaccess(GRMEMS *,time_t,GRMEMS_REC *) ;
static int	grmems_recrear(GRMEMS *,GRMEMS_REC *) ;
static int	grmems_recfins(GRMEMS *op) ;
static int	grmems_upstats(GRMEMS *,int,int) ;

static int	grmems_mkug(GRMEMS *,time_t) ;
static int	grmems_mkugload(GRMEMS *,time_t,vecelem *) ;
static int	grmems_mkugstore(GRMEMS *,time_t,vecelem *) ;

static int	grmems_recusers(GRMEMS *,time_t,vecelem *,gid_t) ;

static int	grmems_pwmapbegin(GRMEMS *,time_t) ;
static int	grmems_pwmapend(GRMEMS *) ;

static int record_start(GRMEMS_REC *,time_t,int,vecelem *,struct group *) ;
static int record_loadgruns(GRMEMS_REC *,vecelem *,struct group *) ;
static int record_access(GRMEMS_REC *,time_t) ;
static int record_refresh(GRMEMS_REC *,time_t,int,vecelem *,struct group *) ;
static int record_mems(GRMEMS_REC *,time_t,int,vecelem *,struct group *) ;
static int record_isold(GRMEMS_REC *,time_t,int) ;
static int record_getgnp(GRMEMS_REC *,const char **) ;
static int record_finish(GRMEMS_REC *) ;

static int	usergid_load(GRMEMS_USERGID *,const char *,int,gid_t) ;

static int	pwentparse(const char *,int,gid_t *) ;

static int	vecelem_addouruniq(vecelem *,GRMEMS_USER *) ;

static int	ugcmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int grmems_start(GRMEMS *op,int max,int ttl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (max < GRMEMS_DEFMAX)
	    max = GRMEMS_DEFMAX ;

	if (ttl < GRMEMS_DEFTTL)
	    ttl = GRMEMS_DEFTTL ;

	memset(op,0,sizeof(GRMEMS)) ;
	op->fd = -1 ;
	op->pagesize = getpagesize() ;

	if ((rs = pq_start(&op->lru)) >= 0) {
	    op->max = max ;
	    op->ttl = ttl ;
	    op->magic = GRMEMS_MAGIC ;
	}

	return rs ;
}
/* end subroutine (grmems_start) */


int grmems_finish(GRMEMS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;

	if (op->usergids != NULL) {
	    rs1 = uc_free(op->usergids) ;
	    if (rs >= 0) rs = rs1 ;
	    op->usergids = NULL ;
	}

/* loop freeing up all cache entries */

	rs1 = grmems_recfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up everything else */

	if (op->recs != NULL) {
	    rs1 = recarr_finish(op->recs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->recs != NULL) {
	    rs1 = uc_free(op->recs) ;
	    if (rs >= 0) rs = rs1 ;
	    op->recs = NULL ;
	}

/* finish up the LRU queue */

	rs1 = pq_finish(&op->lru) ;
	if (rs >= 0) rs = rs1 ;

/* done */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (grmems_finish) */


int grmems_curbegin(GRMEMS *op,GRMEMS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(GRMEMS_CUR)) ;
	curp->ri = -1 ;
	curp->i = -1 ;

	op->cursors += 1 ;
	curp->magic = GRMEMS_CURMAGIC ;

	return rs ;
}
/* end subroutine (grmems_curbegin) */


int grmems_curend(GRMEMS *op,GRMEMS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != GRMEMS_CURMAGIC) return SR_NOTOPEN ;

	if (op->cursors > 0)
	    op->cursors -= 1 ;

	curp->ri = -1 ;
	curp->i = -1 ;
	curp->magic = 0 ;

	return rs ;
}
/* end subroutine (grmems_curend) */


int grmems_lookup(GRMEMS *op,GRMEMS_CUR *curp,cchar *gnp,int gnl)
{
	int		rs = SR_OK ;
	int		ri = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != GRMEMS_CURMAGIC) return SR_NOTOPEN ;

	if (gnp[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("grmems_lookup: gn=%t\n",gnp,gnl) ;
#endif

	if (op->recs == NULL) {
	    rs = grmems_starter(op) ;
	}

	if (rs >= 0) {
	    GRMEMS_REC	*ep = NULL ;
	    time_t	dt = time(NULL) ;
	    int		ct ;

	    op->s.total += 1 ;

	    if ((rs = grmems_fetch(op,&ep,gnp,gnl)) >= 0) {
	        ri = rs ;
	        ct = ct_hit ;
	        rs = grmems_recaccess(op,dt,ep) ;
	    } else if (rs == SR_NOTFOUND) {
	        ct = ct_miss ;
	        rs = grmems_mkrec(op,dt,&ep,gnp,gnl) ;
	        ri = rs ;
	    } /* end if (hit or miss) */

	    grmems_upstats(op,ct,rs) ;

	    if (rs >= 0) {
	        curp->ri = ri ;
	        curp->i = -1 ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("grmems_lookup: ret rs=%d ri=%u\n",rs,ri) ;
#endif

	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (grmems_lookup) */


int grmems_lookread(GRMEMS *op,GRMEMS_CUR *curp,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		ri ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != GRMEMS_CURMAGIC) return SR_NOTOPEN ;

	if (op->recs == NULL) {
	    rs = grmems_starter(op) ;
	}

	if (rs >= 0) {

	    ri = curp->ri ;

#if	CF_DEBUGS
	    debugprintf("grmems_lookread: ri=%d\n",ri) ;
#endif

	    if (ri >= 0) {
	        recarr		*rlp = (recarr *) op->recs ;
	        GRMEMS_REC	*ep ;
	        int		i = (curp->i >= 0) ? (curp->i+1) : 0 ;

#if	CF_DEBUGS
	        debugprintf("grmems_lookread: i=%d\n",i) ;
#endif
	        if ((rs = recarr_get(rlp,ri,&ep)) >= 0) {
	            if (i < ep->nmems) {
	                if ((rs = sncpy1(rbuf,rlen,ep->mems[i])) >= 0) {
	                    curp->i = i ;
	                }
	            } else {
	                rs = SR_NOTFOUND ;
		    }
	        } /* end if */

	    } else {
	        rs = SR_NOTFOUND ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	if (rs >= 0) debugprintf("grmems_lookread: ubuf=%s\n",ubuf) ;
	debugprintf("grmems_lookread: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (grmems_lookread) */


int grmems_invalidate(GRMEMS *op,cchar *gnp,int gnl)
{
	GRMEMS_REC	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_found = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;

	if (gnp[0] == '\0') return SR_INVALID ;

	if (op->recs == NULL) {
	    rs = grmems_starter(op) ;
	}

	if (rs >= 0) {
	    if ((rs = grmems_fetch(op,&ep,gnp,gnl)) >= 0) {
	        const int	ri = rs ;
	        PQ_ENT		*pep = (PQ_ENT *) ep ;

	        f_found = TRUE ;

	        rs1 = pq_unlink(&op->lru,pep) ;
	        if (rs >= 0) rs = rs1 ;

	        rs1 = recarr_del(op->recs,ri) ;
	        if (rs >= 0) rs = rs1 ;

	        rs1 = record_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;

	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;

	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (grmems_invalidate) */


int grmems_check(GRMEMS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;

/* loop checking all cache entries */

	if (op->recs != NULL) {
	    GRMEMS_REC	*ep ;
	    recarr	*rlp = op->recs ;
	    int		i ;
	    for (i = 0 ; recarr_get(rlp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            if (dt == 0) dt = time(NULL) ;
	            if ((rs = record_isold(ep,dt,op->ttl)) > 0) {
	                f = TRUE ;
	                if ((rs = grmems_recdel(op,ep)) >= 0) {
	                    PQ_ENT	*pep = (PQ_ENT *) ep ;
	                    rs = pq_unlink(&op->lru,pep) ;
	                    uc_free(ep) ;
	                }
	            } /* end if (entry-old) */
		} /* end if (non-null) */
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (non-null) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (grmems_check) */


int grmems_stats(GRMEMS *op,GRMEMS_STATS *sp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != GRMEMS_MAGIC) return SR_NOTOPEN ;

	if (op->recs != NULL) {
	    if ((rs = recarr_count(op->recs)) >= 0) {
	        *sp = op->s ;
	        sp->nentries = rs ;
	    }
	} else {
	    memset(sp,0,sizeof(GRMEMS_STATS)) ;
	}

	return rs ;
}
/* end subroutine (grmems_stats) */


/* private subroutines */


static int grmems_starter(GRMEMS *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("grmems_starter: ent %s\n",
	    ((op->recs != NULL) ? "ALREADY" : "NEED")) ;
#endif

	if (op->recs == NULL) {
	    const int	size = sizeof(recarr) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int	ro = 0 ;
	        ro |= RECARR_OSTATIONARY ;
	        ro |= RECARR_OREUSE ;
	        ro |= RECARR_OCONSERVE ;
	        op->recs = p ;
	        if ((rs = recarr_start(op->recs,op->max,ro)) >= 0) {
		    op->ti_check = time(NULL) ;
	        } /* end if (recarr_start) */
#if	CF_DEBUGS
	debugprintf("grmems_starter: recarr_start rs=%d\n",rs) ;
#endif
	        if (rs < 0) {
	            uc_free(op->recs) ;
	            op->recs = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end if (needed initialization) */

#if	CF_DEBUGS
	debugprintf("grmems_starter: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (grmems_starter) */


static int grmems_fetch(GRMEMS *op,GRMEMS_REC **epp,const char *gnp,int gnl)
{
	recarr		*rlp = op->recs ;
	GRMEMS_REC	*ep ;
	int		rs ;
	int		i ;

	for (i = 0 ; (rs = recarr_get(rlp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        if (gnp[0] == ep->gn[0]) {
	            if (strwcmp(ep->gn,gnp,gnl) == 0) break ;
		}
	    }
	} /* end for */

	if (epp != NULL) {
	    *epp = (rs >= 0) ? ep : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (grmems_fetch) */


static int grmems_mkrec(GRMEMS *op,time_t dt,GRMEMS_REC **epp,
		cchar *gnp,int gnl)
{
	int		rs ;
	int		rs1 ;
	int		ri = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems_mkrec: ent gn=%t\n",gnp,gnl) ;
#endif

	*epp = NULL ;
	if ((rs = recarr_count(op->recs)) >= 0) {
	    PQ_ENT	*pep ;
	    const int	n = rs ;

	    if (n >= op->max) {

	        if ((rs = pq_rem(&op->lru,&pep)) >= 0) {
	            GRMEMS_REC	*ep = (GRMEMS_REC *) pep ;

	            if ((rs = grmems_recdel(op,ep)) >= 0) {
	                rs = grmems_recstart(op,dt,ep,gnp,gnl) ;
	                ri = rs ;
	            }

	            rs1 = pq_ins(&op->lru,pep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (removed entry) */

	        if (rs >= 0) *epp = (GRMEMS_REC *) pep ;

	    } else {

	        if ((rs = grmems_newrec(op,dt,epp,gnp,gnl)) >= 0) {
	            ri = rs ;
	            if (*epp != NULL) {
	                pep = (PQ_ENT *) *epp ;
	                rs = pq_ins(&op->lru,pep) ;
	            }
	        } /* end if (new-entry) */

	    } /* end if (at max entries or not) */

	} /* end if (recarr_count) */

#if	CF_DEBUGS
	debugprintf("grmems_mkrec: ret rs=%d ri=%u\n",rs,ri) ;
#endif

	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (grmems_mkrec) */


static int grmems_newrec(GRMEMS *op,time_t dt,GRMEMS_REC **epp,
		cchar *gnp,int gnl)
{
	GRMEMS_REC	*ep ;
	const int	rsize = sizeof(GRMEMS_REC) ;
	int		rs ;

	if (epp == NULL) return SR_NOANODE ;

	if ((rs = uc_malloc(rsize,&ep)) >= 0) {
	    rs = grmems_recstart(op,dt,ep,gnp,gnl) ;
	    if (rs < 0) 
		uc_free(ep) ;
	} /* end if (memory-allocation) */

	*epp = (rs >= 0) ? ep : NULL ;
	return rs ;
}
/* end subroutine (grmems_newrec) */


static int grmems_recstart(GRMEMS *op,time_t dt,GRMEMS_REC *ep,
		cchar *gnp,int gnl)
{
	struct group	gr ;
	const int	grlen = getbufsize(getbufsize_gr) ;
	const int	wc = op->wcount++ ;
	int		rs ;
	int		rs1 ;
	int		ri = 0 ;
	char		*grbuf ;
	char		gn[GROUPNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("grmems_recstart: gn=%t\n",gnp,gnl) ;
#endif

	strdcpy1w(gn,GROUPNAMELEN,gnp,gnl) ;

	if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	    if ((rs = getgr_name(&gr,grbuf,grlen,gn)) >= 0) {
	        vecelem		u ;
	        const gid_t	gid = gr.gr_gid ;
	        const int	esize = sizeof(GRMEMS_USER) ;
	        if ((rs = vecelem_start(&u,esize,10,0)) >= 0) {
	            if ((rs = grmems_recusers(op,dt,&u,gid)) >= 0) {
	                if ((rs = record_start(ep,dt,wc,&u,&gr)) >= 0) {
	                    if ((rs = recarr_add(op->recs,ep)) >= 0) {
	                        ri = rs ;
	                        ep->recidx = rs ;
	                    }
	                    if (rs < 0)
	                        record_finish(ep) ;
	                } /* end if (entry-start) */
	            } /* end if (grmems-recusers) */
	            rs1 = vecelem_finish(&u) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecelem-user) */
	    } /* end if (getgr-name) */
	    uc_free(grbuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("grmems_recstart: rs=%d ri=%u\n",rs,ri) ;
#endif

	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (grmems_recstart) */


static int grmems_recrefresh(GRMEMS *op,time_t dt,GRMEMS_REC *ep)
{
	const int	wc = op->wcount++ ;
	int		rs ;
	int		rs1 ;
	const char	*gnp ;

#if	CF_DEBUGS
	debugprintf("grmems_recrefresh: ent\n") ;
#endif

	if ((rs = record_getgnp(ep,&gnp)) >= 0) {
	    struct group	gr ;
	    const int		grlen = getbufsize(getbufsize_gr) ;
	    char		*grbuf ;

	    if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	        if ((rs = getgr_name(&gr,grbuf,grlen,gnp)) >= 0) {
	            vecelem	u ;
	            const gid_t	gid = gr.gr_gid ;
	            const int	esize = sizeof(GRMEMS_USER) ;
	            if ((rs = vecelem_start(&u,esize,10,0)) >= 0) {
	                if ((rs = grmems_recusers(op,dt,&u,gid)) >= 0) {
	                    rs = record_refresh(ep,dt,wc,&u,&gr) ;
	                } /* end if (grmems-recusers) */
	                rs1 = vecelem_finish(&u) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (vecelem-user) */
	        } /* end if (getgr-name) */
	        uc_free(grbuf) ;
	    } /* end if (memory-allocation) */

	} /* end if (record_getgnp) */

#if	CF_DEBUGS
	debugprintf("grmems_recrefresh: rs=%d wc=%u\n",rs,wc) ;
#endif

	return (rs >= 0) ? wc : rs ;
}
/* end subroutine (grmems_recrefresh) */


/* get all users w/ this specified GID */
static int grmems_recusers(GRMEMS *op,time_t dt,vecelem *ulp,gid_t gid)
{
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems_recusers: ent gid=%d\n",gid) ;
#endif

	if ((rs = grmems_mkug(op,dt)) >= 0) {
	    GRMEMS_USERGID	k, *ugp ;
	    GRMEMS_USERGID	*ugs = (GRMEMS_USERGID *) op->usergids ;
	    const int		esize = sizeof(GRMEMS_USERGID) ;
	    const int		n = op->nusergids ;

	    k.gid = gid ;
	    ugp = (GRMEMS_USERGID *) bsearch(&k,ugs,n,esize,ugcmp) ;
	    if (ugp != NULL) {
		const int	ulen = USERNAMELEN ;
	        while (ugp > ugs) {
	            if (ugp[-1].gid != gid) break ;
	            ugp -= 1 ;
	        }
	        while ((ugp < (ugs+n)) && (ugp->gid == gid)) {
	            GRMEMS_USER		u ;
	            u.up = ugp->un ;
	            u.ul = strnlen(ugp->un,ulen) ;
	            rs = vecelem_add(ulp,&u) ;
	            c += 1 ;
	            ugp += 1 ;
	            if (rs < 0) break ;
	        } /* end while */
	    } /* end if (had some hits) */

	} /* end if (grmems-mkug) */

#if	CF_DEBUGS
	debugprintf("grmems_recusers: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (grmems_recusers) */


static int grmems_mkug(GRMEMS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems_mkug: ent\n") ;
#endif

	if (op->usergids == NULL) {
	    vecelem	ug ;
	    const int	esize = sizeof(GRMEMS_USERGID) ;
	    if ((rs = vecelem_start(&ug,esize,10,0)) >= 0) {
	        if ((rs = grmems_mkugload(op,dt,&ug)) >= 0) {
	            c = rs ;
	            if ((rs = vecelem_sort(&ug,ugcmp)) >= 0) {
	                rs = grmems_mkugstore(op,dt,&ug) ;
		    }
	        } /* end if (grmems-mkugload) */
	        vecelem_finish(&ug) ;
	    } /* end if (vecelem) */
	} /* end if (usergids) */

#if	CF_DEBUGS
	debugprintf("grmems_mkug: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (grmems_mkug) */


static int grmems_mkugload(GRMEMS *op,time_t dt,vecelem *ulp)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems_mkugload: ent\n") ;
#endif

	if ((rs = grmems_pwmapbegin(op,dt)) >= 0) {
	    int		ml = op->fsize ;
	    const char	*mp = (const char *) op->mapdata ;
	    const char	*tp ;
	    op->ti_access = dt ;
	    while ((tp = strnchr(mp,ml,CH_NL)) != NULL) {
	        gid_t	gid ;
	        int	len = (tp-mp) ;
	        int	ul ;
	        if ((rs = pwentparse(mp,len,&gid)) > 0) {
	            GRMEMS_USERGID	ug ;
		    ul = rs ;
#if	CF_DEBUGS
	            debugprintf("grmems_mkugload: un=%t gid=%d\n",
	                mp,ul,gid) ;
#endif
	            if (usergid_load(&ug,mp,ul,gid) > 0) {
	                c += 1 ;
	                rs = vecelem_add(ulp,&ug) ;
	            }
	        } /* end if (pwentparse) */
	        ml -= ((tp+1)-mp) ;
	        mp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */
	    rs1 = grmems_pwmapend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (grmems-pwmap) */

#if	CF_DEBUGS
	{
	    GRMEMS_USERGID	*ugp ;
	    int	i ;
	    for (i = 0 ; vecelem_get(ulp,i,&ugp) >= 0 ; i += 1) {
	        if (ugp != NULL) {
	        debugprintf("grmems_mkugload: check un=%t\n",
	            ugp->un,strnlen(ugp->un,USERNAMELEN)) ;
		}
	    }
	}
	debugprintf("grmems_mkugload: ret rs=%d c=%u\n",rs,c) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (grmems_mkugload) */


static int grmems_mkugstore(GRMEMS *op,time_t dt,vecelem *ulp)
{
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems_mkugstore: ent\n") ;
#endif

	if ((rs = vecelem_count(ulp)) >= 0) {
	    const int	esize = sizeof(GRMEMS_USERGID) ;
	    const int	n = rs ;
	    int		size ;
	    void	*p ;
	    size = ((n+1) * esize) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        GRMEMS_USERGID	*ugs = (GRMEMS_USERGID *) p ;
	        GRMEMS_USERGID	*ugp ;
	        int	i ;
	        for (i = 0 ; vecelem_get(ulp,i,&ugp) >= 0 ; i += 1) {
	            if (ugp != NULL) {
	                ugs[c++] = *ugp ;
		    }
	        } /* end for */
	        ugs[c].un[0] = '\0' ;
	        op->usergids = ugs ;
	        op->nusergids = c ;
	        op->ti_usergids = dt ;
	    } /* end if (memory-allocation) */
	} /* end if (vecelem-count) */

#if	CF_DEBUGS
	{
	    GRMEMS_USERGID	*ugs = (GRMEMS_USERGID *) op->usergids ;
	    GRMEMS_USERGID	*ugp ;
	    if (mems != NULL) {
	        int	i ;
	        for (i = 0 ; ugs[i].un[0] != '\0' ; i += 1) {
	            ugp = (ugs+i) ;
	            debugprintf("grmems_mkugstore: check un=%t gid=%d\n",
	                ugp->un,strnlen(ugp->un,USERNAMELEN),ugp->gid) ;
	        }
	        debugprintf("grmems_mkugstore: check-out i=%u\n",i) ;
	    }
	}
	debugprintf("grmems_mkugstore: ret rs=%d c=%u\n",rs,c) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (grmems_mkugstore) */


static int grmems_recaccess(GRMEMS *op,time_t dt,GRMEMS_REC *ep)
{
	int		rs ;

	if ((rs = grmems_recrear(op,ep)) >= 0) {
	    if ((rs = record_isold(ep,dt,op->ttl)) > 0) {
	        op->s.refreshes += 1 ;
	        rs = grmems_recrefresh(op,dt,ep) ;
	    } else {
	        rs = record_access(ep,dt) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (grmems_recaccess) */


static int grmems_recrear(op,ep)
GRMEMS		*op ;
GRMEMS_REC	*ep ;
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
	                GRMEMS_REC	*ep = (GRMEMS_REC *) pep ;
	                record_finish(ep) ;
	                uc_free(pep) ;
	            }
	        } /* end if (pq_unlink) */
	    }
	} /* end if (pq_gettail) */

	return rs ;
}
/* end subroutine (grmems_recrear) */


static int grmems_recdel(GRMEMS *op,GRMEMS_REC *ep)
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
/* end subroutine (grmems_recdel) */


static int grmems_recfins(GRMEMS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->recs != NULL) {
	    GRMEMS_REC	*ep ;
	    recarr	*rlp = op->recs ;
	    int		i ;
	    for (i = 0 ; recarr_get(rlp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs1 = record_finish(ep) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_free(ep) ;
	            if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	} /* end if */

	return rs ;
}
/* end subroutine (grmems_recfins) */


static int grmems_upstats(GRMEMS *op,int ct,int rs)
{
	int		f_got = (rs > 0) ;
#if	CF_DEBUGS
	debugprintf("grmems_upstats: ct=%u rs=%d\n",ct,rs) ;
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
/* end subroutine (grmems_upstats) */


static int grmems_pwmapbegin(GRMEMS *op,time_t dt)
{
	int		rs = SR_OK ;
	if (op->mapdata == NULL) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    const char		*fn = GRMEMS_SYSPASSWD ;
	    if ((rs = uc_open(fn,of,om)) >= 0) {
	        op->fd = rs ;
	        op->ti_open = dt ;
	        if ((rs = uc_fsize(op->fd)) >= 0) {
	            size_t	ms = MAX(rs,op->pagesize) ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            int		fd = op->fd ;
	            void	*md ;
	            op->fsize = rs ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                const int	madv = MADV_SEQUENTIAL ;
			const caddr_t	ma = md ;
	                if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	                    op->mapdata = md ;
	                    op->mapsize = ms ;
	                } /* end if (advise) */
	                if (rs < 0) {
	                    u_munmap(md,ms) ;
	                }
	            } /* end if (mmap) */
	        } /* end if (file-size) */
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    } /* end if (file-open) */
	} /* end if (need mapping) */
	return rs ;
}
/* end subroutine (grmems_pwmapbegin) */


static int grmems_pwmapend(GRMEMS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->mapdata != NULL) {
	    size_t	ms = op->mapsize ;
	    const void	*md = op->mapdata ;
	    rs1 = u_munmap(md,ms) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->ti_open = 0 ;
	}
	return rs ;
}
/* end subroutine (grmems_pwmapend) */


static int record_start(ep,dt,wc,ulp,grp)
GRMEMS_REC	*ep ;
time_t		dt ;
int		wc ;
vecelem		*ulp ;
struct group	*grp ;
{
	int		rs ;
	int		n = 0 ;

	if (ep == NULL) return SR_FAULT ;
	if (ulp == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("grmems/record_start: gn=%s\n",grp->gr_name) ;
#endif

	memset(ep,0,sizeof(GRMEMS_REC)) ;

	strwcpy(ep->gn,grp->gr_name,GROUPNAMELEN) ;

	rs = record_mems(ep,dt,wc,ulp,grp) ;
	n = rs ;

#if	CF_DEBUGS
	debugprintf("grmems/record_start: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (record_start) */


static int record_finish(ep)
GRMEMS_REC	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->mems != NULL) {
	    rs1 = uc_free(ep->mems) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->mems = NULL ;
	}

	ep->gn[0] = '\0' ;
	return rs ;
}
/* end subroutine (record_finish) */


static int record_refresh(ep,dt,wc,ulp,grp)
GRMEMS_REC	*ep ;
time_t		dt ;
int		wc ;
vecelem		*ulp ;
struct group	*grp ;
{
	int		rs = SR_OK ;
	int		n = 0 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->mems != NULL) {
	    rs = uc_free(ep->mems) ;
	    ep->mems = NULL ;
	    ep->nmems = 0 ;
	}

	if (rs >= 0) {
	    rs = record_mems(ep,dt,wc,ulp,grp) ;
	    n = rs ;
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (record_refresh) */


static int record_mems(ep,dt,wc,ulp,grp)
GRMEMS_REC	*ep ;
time_t		dt ;
int		wc ;
vecelem		*ulp ;
struct group	*grp ;
{
	int		rs ;
	int		n = 0 ;

	if ((rs = record_loadgruns(ep,ulp,grp)) >= 0) {
	    if ((rs = vecelem_count(ulp)) >= 0) {
	        GRMEMS_USER	*up ;
		const int	ulen = USERNAMELEN ;
	        int		size = 0 ;
	        int		masize ;
		int		ul ;
	        int		i ;
	        char		*bp ;
	        n = rs ;
	        masize = ((n+1)*sizeof(const char *)) ;
	        size += masize ;
	        for (i = 0 ; vecelem_get(ulp,i,&up) >= 0 ; i += 1) {
	            if (up != NULL) {
		        ul = up->ul ;
		        if (ul < 0) {
			    ul = strnlen(up->up,ulen) ;
			    up->ul = ul ;
		        }
	                size += (ul+1) ;
		    }
	        } /* end for */
	        if ((rs = uc_malloc(size,&bp)) >= 0) {
	            cchar	**mems = (cchar **) bp ;
	            int		c = 0 ;
	            bp += masize ;
	            for (i = 0 ; vecelem_get(ulp,i,&up) >= 0 ; i += 1) {
	                if (up != NULL) {
	                    mems[c++] = bp ;
	                    bp = (strwcpy(bp,up->up,up->ul) + 1) ;
			}
	            } /* end for */
	            mems[c] = NULL ;
	            ep->ti_create = dt ;
	            ep->ti_access = dt ;
	            ep->wcount = wc ;
	            ep->mems = mems ;
	            ep->nmems = c ;
	        } /* end if (memory-allocation) */
	    } /* end if (vecelem-count) */
	} /* end if (load user members) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (record_mems) */


static int record_loadgruns(GRMEMS_REC *op,vecelem *ulp,struct group *grp)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("grmems/record_loadgruns: gn=%s\n",grp->gr_name) ;
#endif

	if (op == NULL) return SR_FAULT ; /* lint */

	if (grp->gr_mem != NULL) {
	    const char	**mems = (const char **) grp->gr_mem ;
	    int		i ;
	    for (i = 0 ; mems[i] != NULL ; i += 1) {
	        GRMEMS_USER	u ;
	        u.up = mems[i] ;
	        u.ul = -1 ;
	        rs = vecelem_addouruniq(ulp,&u) ;
	        if (rs < INT_MAX) c += 1 ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (had members) */

#if	CF_DEBUGS
	debugprintf("grmems/record_loadgruns: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (record_loadgruns) */


static int record_access(ep,dt)
GRMEMS_REC	*ep ;
time_t		dt ;
{
	int		rs = SR_OK ;
	int		wc ;

	if (ep == NULL) return SR_FAULT ;

	ep->ti_access = dt ;
	wc = ep->wcount ;
	return (rs >= 0) ? wc : rs ;
}
/* end subroutine (record_access) */


static int record_isold(GRMEMS_REC *ep,time_t dt,int ttl)
{
	int		f_old ;

	if (ep == NULL) return SR_FAULT ;

	f_old = ((dt - ep->ti_create) >= ttl) ;
	return f_old ;
}
/* end subroutine (record_isold) */


static int record_getgnp(GRMEMS_REC *ep,const char **rpp)
{
	int		rs = SR_OK ;
	if (ep == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	if (ep->gn[0] == '\0') rs = SR_NOTOPEN ;
	*rpp = (rs >= 0) ? ep->gn : NULL ;
	return rs ;
}
/* end subroutine (record_getgnp) */


static int usergid_load(GRMEMS_USERGID *ugp,cchar *unp,int unl,gid_t gid)
{
	int		ul = strnwcpy(ugp->un,USERNAMELEN,unp,unl) - ugp->un ;
	ugp->gid = gid ;
	return ul ;
}
/* end subroutine (usergid_load) */


/* PASSWD entry parsing */
static int pwentparse(cchar *lbuf,int llen,gid_t *gp)
{
	int		rs = SR_OK ;
	int		fi ;
	int		ll = llen ;
	int		ul = 0 ;
	const char	*lp = lbuf ;
	const char	*tp ;
	for (fi = 0 ; fi < 4 ; fi += 1) {
	    if ((tp = strnchr(lp,ll,':')) != NULL) {
	        switch (fi) {
	        case 0:
	            ul = (tp-lp) ;
	            break ;
	        case 3:
	            {
	                int	v ;
	                if ((rs = cfdeci(lp,(tp-lp),&v)) >= 0) {
	                    *gp = (gid_t) v ;
	                } else if (isNotValid(rs)) {
			    rs = SR_OK ;
	                    ul = 0 ;
			}
	            } /* end block */
	            break ;
	        } /* end switch */
	        ll -= ((tp+1)-lp) ;
	        lp = (tp+1) ;
	    } /* end if (had separator) */
	} /* end for (looping through fields) */
	if (fi < 4) ul = 0 ;

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (pwentparse) */


static int vecelem_addouruniq(vecelem *ulp,GRMEMS_USER *unp)
{
	GRMEMS_USER	*ep ;
	const int	ulen = USERNAMELEN ;
	int		rs ;
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; (rs = vecelem_get(ulp,i,&ep)) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        f = (ep->up[0] == unp->up[0]) ;
	        f = f && (strncmp(ep->up,unp->up,ulen) == 0) ;
	        if (f) break ;
	    }
	} /* end for */
	if (rs == SR_NOTFOUND) {
	    rs = vecelem_add(ulp,unp) ;
	} else {
	    rs = INT_MAX ;
	}
	return rs ;
}
/* end subroutine (vecelem_addouruniq) */


static int ugcmp(const void *v1p,const void *v2p)
{
	GRMEMS_USERGID	*g1p = (GRMEMS_USERGID *) v1p ;
	GRMEMS_USERGID	*g2p = (GRMEMS_USERGID *) v2p ;
	int		rc = (g1p->gid - g2p->gid) ;
	return rc ;
}
/* end subroutine (ugcmp) */


