/* yags */

/* this  is a YAGS branch predictor */


#define	CF_DEBUGS	0
#define	CF_SAFE		1
#define	CF_VOTEREPLACE	0		/* replace by voting among counters */
#define	CF_COUNTREPLACE	1		/* replace by counting all counters */


/* revision history:

	= 2002-05-01, David A­D­ Morano

	This object module was created for Levo research.  It is a
	value predictor.  This is not coded as hardware.  It is like
	Atom analysis subroutines!


*/


/******************************************************************************

	This object module implements a branch predictor.  This BP
	is a YAGS (see Mudge) type branch predictor.  This BP *may*
	be among the best of the "share" type predictors.


*****************************************************************************/


#define	YAGS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"bpload.h"
#include	"yags.h"



/* local defines */

#define	YAGS_MAGIC	0x29456781
#define	YAGS_DEFCH	4		/* default entries */
#define	YAGS_DEFCA	4		/* default entries */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	GETPRED(c)	(((c) >> 1) & 1)

#ifndef	ENDIAN
#if	defined(SOLARIS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif



/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	flbsi(uint) ;
extern int	uc_malloc() ;


/* forward references */

static uint	satcount(uint,uint,int) ;

static int	calu(struct yags_cache *,uint,uint,uint *) ;
static int	caup(struct yags_cache *,uint,uint,int,int) ;


/* global variables */

struct bpload	yags = {
	"yags",
	sizeof(YAGS),
} ;


/* local variables */







int yags_init(op,chlen,calen)
YAGS		*op ;
int		chlen ;
int		calen ;
{
	int	rs ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(YAGS)) ;


/* the choice PHT */

	if (chlen <= 2)
	    chlen = YAGS_DEFCH ;

	op->chlen = nextpowtwo(chlen) ;

	size = op->chlen * sizeof(struct yags_pht) ;
	rs = uc_malloc(size,&op->choice) ;

	if (rs < 0)
	    goto bad0 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->choice,rs,"yags_init:choice") ;
#endif

	(void) memset(op->choice,0,size) ;

/* the caches */

	if (calen <= 2)
	    calen = YAGS_DEFCA ;

	op->calen = nextpowtwo(calen) ;

	size = op->calen * sizeof(struct yags_cache) ;
	rs = uc_malloc(size,&op->taken) ;

#if	CF_DEBUGS
	debugprintf("yags_init: uc_malloc() rs=%d taken=%p\n",
		rs, op->taken) ;
#endif

	if (rs < 0)
	    goto bad1 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->taken,rs,"yags_init:taken") ;
#endif

	(void) memset(op->taken,0,size) ;

	rs = uc_malloc(size,&op->nottaken) ;

#if	CF_DEBUGS
	debugprintf("yags_init: uc_malloc() rs=%d nottaken=%p\n",
		rs, op->nottaken) ;
#endif

	if (rs < 0)
	    goto bad2 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->nottaken,rs,"yags_init:nottaken") ;
#endif

	(void) memset(op->nottaken,0,size) ;


/* we're out of here */

	op->magic = YAGS_MAGIC ;
	return rs ;

/* we're out of here */
bad2:
	free(op->taken) ;

#ifdef	MALLOCLOG
	malloclog_free(op->taken,"yags_init:taken") ;
#endif

bad1:
	free(op->choice) ;

#ifdef	MALLOCLOG
	malloclog_free(op->choice,"yags_init:choice") ;
#endif

bad0:
	return rs ;
}
/* end subroutine (yags_init) */


/* free up this yags object */
int yags_free(op)
YAGS		*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;

	if (op->choice != NULL) {

	    free(op->choice) ;

#ifdef	MALLOCLOG
	malloclog_free(op->choice,"yags_free:choice") ;
#endif

	}

	if (op->taken != NULL) {

	    free(op->taken) ;

#ifdef	MALLOCLOG
	malloclog_free(op->taken,"yags_free:taken") ;
#endif

	}

	if (op->nottaken != NULL) {

	    free(op->nottaken) ;

#ifdef	MALLOCLOG
	malloclog_free(op->nottaken,"yags_free:nottaken") ;
#endif

	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (yags_free) */


/* lookup an IA */
int yags_lookup(op,ia)
YAGS		*op ;
uint		ia ;
{
	uint	count_ch ;
	uint	count_ca ;

	int	rs ;
	int	chi ;
	int	cai ;
	int	tag ;
	int	f_pred ;
	int	f_chpred ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	chi = (ia >> 2) % op->chlen ;

	cai = ((ia >> 2) ^ (op->bhistory >> 1)) % op->calen ;

	tag = (ia >> 2) & YAGS_TAGMASK ;
	tag <<= 1 ;
	tag |= (op->bhistory & 1) ;

#if	CF_DEBUGS
	debugprintf("yags_lookup: ia=%08x cai=%d tag=%d\n",ia,cai,tag) ;
#endif

	count_ch = op->choice[chi].counter ;

	f_chpred = (count_ch >> 1) & 1 ;
	if (f_chpred) {

/* choice says "taken" so use the "not-taken" cache */

	    rs = calu(op->nottaken,cai,tag,&count_ca) ;

	} else {

/* choice says "not-taken" so use the "taken" cache */

	    rs = calu(op->taken,cai,tag,&count_ca) ;

	} /* end if (taken/not-taken) */

	if (rs >= 0) {

	    op->nottaken[cai].lru = ((rs == 0) ? 1 : 0) ;
	    f_pred = ((count_ca >> 1) & 1) ;

	} else
	    f_pred = f_chpred ;

	return f_pred ;
}
/* end subroutine (yags_lookup) */


/* update on branch resolution */
int yags_update(op,ia,f_outcome)
YAGS		*op ;
uint		ia ;
int		f_outcome ;
{
	uint	count_ch ;
	uint	count_ca ;
	uint	ncount ;

	int	rs ;
	int	chi ;
	int	cai ;
	int	tag ;
	int	f_hit ;
	int	f_pred ;
	int	f_chpred ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	chi = (ia >> 2) % op->chlen ;

	cai = ((ia >> 2) ^ (op->bhistory >> 1)) % op->calen ;

	tag = (ia >> 2) & YAGS_TAGMASK ;
	tag <<= 1 ;
	tag |= (op->bhistory & 1) ;

#if	CF_DEBUGS
	debugprintf("yags_update: ia=%08x cai=%d tag=%d\n",ia,cai,tag) ;
#endif

	count_ch = op->choice[chi].counter ;

	f_chpred = (count_ch >> 1) & 1 ;
	if (f_chpred) {

/* choice says "taken" so use the "not-taken" cache */

	    rs = calu(op->nottaken,cai,tag,&count_ca) ;

	} else {

/* choice says "not-taken" so use the "taken" cache */

	    rs = calu(op->taken,cai,tag,&count_ca) ;

	} /* end if (taken/not-taken) */

	f_hit = FALSE ;
	if (rs >= 0) {

		f_hit = TRUE ;
	    f_pred = ((count_ca >> 1) & 1) ;

	} else
	    f_pred = f_chpred ;

/* update stuff */

/* choice PHT */

	if (! ((! LEQUIV(f_chpred,f_outcome)) && 
	    f_hit && LEQUIV(f_outcome,f_pred))) {

	    ncount = satcount(count_ca,YAGS_COUNTBITS,f_outcome) ;

	    op->choice[chi].counter = ncount ;

	} /* end if (choice PHT conditional update) */

/* the direction caches */

/* update "not-taken" cache */

	if (f_hit && f_chpred) {

	    caup(op->nottaken,cai,tag,f_outcome,0) ;

	} else if (f_chpred && (! f_outcome)) {

	    caup(op->nottaken,cai,tag,f_outcome,0) ;

	}

/* update "taken" cache */

	if (f_hit && (! f_chpred)) {

	    caup(op->taken,cai,tag,f_outcome,1) ;

	} else if ((! f_chpred) && (! f_outcome)) {

	    caup(op->taken,cai,tag,f_outcome,1) ;

	} /* end if */

/* update the branch history register */

	op->bhistory = (op->bhistory << 1) | f_outcome ;

	return 0 ;
}
/* end subroutine (yags_update) */


#ifdef	COMMENT

/* get the entries (serially) */
int yags_get(op,ri,rpp)
YAGS		*op ;
int		ri ;
YAGS_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("yags_get: entered 0\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("yags_get: entered 1\n") ;
#endif

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("yags_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("yags_get: entered ri=%d\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->tablen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->table + ri ;

#if	CF_DEBUGS
	debugprintf("yags_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (yags_get) */

#endif /* COMMENT */

/* zero out the statistics */
int yags_zerostats(op)
YAGS		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;

	(void) memset(&op->s,0,sizeof(YAGS_STATS)) ;

	return rs ;
}
/* end subroutine (yags_zerostats) */


/* get statistics */
int yags_stats(op,rp)
YAGS		*op ;
YAGS_STATS	*rp ;
{
	int	bits_total ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != YAGS_MAGIC)
	    return SR_NOTOPEN ;

/* calculate the bits for this predictor */

	{
		uint	bits_cpht ;
		uint	bits_dpht ;
		uint	bits_caentry ;
		uint	bits_history ;


		bits_history = flbsi(op->calen) ;

		bits_cpht = op->chlen * YAGS_COUNTBITS ;

		bits_caentry = (YAGS_TAGBITS + 1 + YAGS_COUNTBITS) ;
		bits_dpht = 2 * op->calen * bits_caentry ;

		bits_total = bits_cpht + bits_dpht + bits_history ;
	}

/* fill in the extra stuff */

	if (rp != NULL) {

	(void) memcpy(rp,&op->s,sizeof(YAGS_STATS)) ;

	rp->cpht = op->chlen ;
	rp->dpht = op->calen ;
	rp->bits = bits_total ;

	}

	return bits_total ;
}
/* end subroutine (yags_stats) */



/* INTERNAL SUBROUTINES */



uint satcount(v,n,f_up)
uint	v ;
uint	n ;
int	f_up ;
{
	uint	r ;


	if (f_up)
	    r = (v == (n - 1)) ? v : (v + 1) ;

	else 
	    r = (v == 0) ? 0 : (v - 1) ;

	return r ;
}
/* end subroutine (satcount) */


/* cache lookup */
static int calu(cp,ci,tag,rp)
struct yags_cache	*cp ;
uint			ci ;
uint			tag ;
uint			*rp ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("yags/calu: cp=%p ci=%d tag=%d\n",cp,ci,tag) ;
#endif

	rs = -1 ;
	if (cp[ci].tag0 == tag) {

	    rs = 0 ;
	    *rp = cp[ci].counter0 ;

	} else if (cp[ci].tag1 == tag) {

	    rs = 1 ;
	    *rp = cp[ci].counter1 ;

	}

	return rs ;
}
/* end subroutine (calu) */


/* cache update */
static int caup(cp,ci,tag,f_outcome,f_type)
struct yags_cache	*cp ;
uint			ci ;
uint			tag ;
int			f_outcome ;
int			f_type ;
{
	uint	count, ncount ;
	uint	tag0, tag1 ;
	uint	p0, p1 ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("yags/caup: cp=%p ci=%d tag=%d\n",cp,ci,tag) ;
#endif

	rs = -1 ;
	tag0 = cp[ci].tag0 ;
	tag1 = cp[ci].tag1 ;
	if (tag0 == tag) {

	    rs = 0 ;
	    count = cp[ci].counter0 ;

	} else if (tag1 == tag) {

	    rs = 1 ;
	    count = cp[ci].counter1 ;

	}

/* cache update */

	if (rs >= 0) {

/* it was a hit */

	    ncount = satcount(count,YAGS_COUNTBITS,f_outcome) ;

	    if (rs == 0)
	        cp[ci].counter0 = ncount ;

	    else
	        cp[ci].counter1 = ncount ;

	} else {

/* it was a miss */

	    if (f_type) {

	        p0 = GETPRED(cp[ci].counter0) ;
	        p1 = GETPRED(cp[ci].counter1) ;

	        if (! LEQUIV(p0,p1)) {

	            if (! p0) {

	                cp[ci].tag0 = tag ;
	                cp[ci].counter0 = f_outcome ;

	            } else {

	                cp[ci].tag1 = tag ;
	                cp[ci].counter1 = f_outcome ;

	            }

	        } else {

	            if (cp[ci].lru) {

	                cp[ci].tag0 = tag ;
	                cp[ci].counter0 = f_outcome ;

	            } else {

	                cp[ci].tag1 = tag ;
	                cp[ci].counter1 = f_outcome ;

	            }

	        }

	    } else {

	        if (cp[ci].lru) {

	            cp[ci].tag0 = tag ;
	            cp[ci].counter0 = f_outcome ;

	        } else {

	            cp[ci].tag1 = tag ;
	            cp[ci].counter1 = f_outcome ;

	        }

	    }
	}

	return rs ;
}
/* end subroutine (caup) */



