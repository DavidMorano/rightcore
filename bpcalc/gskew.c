/* gskew */

/* this is a GSKEW branch predictor */


#define	CF_DEBUGS	0
#define	CF_DEBUGS2	0
#define	CF_SAFE		1
#define	CF_ALLONES	0		/* initialize META to ones */
#define	CF_ALLMIDDLE	1
#define	CF_MUSTAGREE	0		/* predictor must agree w/ outcome */


/* revision history:

	= 2002-05-01, David A­D­ Morano
        This object module was created for Levo research. It is a value
        predictor. This is not coded as hardware. It is like Atom analysis
        subroutines!

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module implements the GSKEW (2Bc-gskew) branch predictor.


	Synopsis:

	int gskew_init(op,p1,p2,p3,p4)
	GSKEW	*op ;
	int	p1, p2, p3, p4 ;

	Arguments:

	p1	table length
	p2	number of history bits

	Returns:


*******************************************************************************/


#define	GSKEW_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bpload.h"
#include	"gskew.h"


/* local defines */

#define	GSKEW_DEFLEN		(64 * 1024)
#define	GSKEW_STATES		4
#define	GSKEW_DEFGLEN		4		/* default entries */
#define	GSKEW_DEFHIST		15		/* default history bits */
#ifdef	COMMENT
#define	GSKEW_DEFHIST		9		/* default history bits */
#define	GSKEW_DEFHIST		11		/* default history bits */
#endif /* COMMENT */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	GETPRED(c)	(((c) >> 1) & 1)
#define	GETPRED2(c)	(((c) >> 1) & 1)
#define	GETPRED3(c)	(((c) >> 2) & 1)

#define	BIT(w,n)	(((w) >> (n)) & 1)


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	flbsi(uint) ;


/* forward references */

static uint	satcount(uint,uint,int) ;
static uint	h(int,uint), hinv(int,uint) ;
static uint	fi_bim(int,uint,uint) ;
static uint	fi_g0(int,uint,uint) ;
static uint	fi_g1(int,uint,uint) ;
static uint	fi_meta(int,uint,uint) ;


/* global variables */

struct bpload	gskew = {
	"gskew",
	sizeof(GSKEW),
} ;


/* local variables */


/* exported subroutines */


int gskew_init(op,p1,p2,p3,p4)
GSKEW	*op ;
int	p1, p2, p3, p4 ;
{
	int		rs ;
	int		i ;
	int		size ;
	int		max ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(GSKEW)) ;

	max = p1 ;
	if (max < 0)
	    max = GSKEW_DEFLEN ;

	op->tlen = nextpowtwo(max) ;

	op->tmask = op->tlen - 1 ;

	op->n = flbsi(op->tlen) ;

	if (p4 < 0)
	    p4 = GSKEW_DEFHIST ;

	op->nhist = p4 ;
	op->hmask = (1 << op->nhist) - 1 ;

#if	CF_DEBUGS2
	debugprintf("gskew_init: nhist=%d\n",op->nhist) ;
#endif

/* allocate the space */

	size = op->tlen * sizeof(struct gskew_banks) ;
	rs = uc_malloc(size,&op->table) ;

	if (rs < 0)
	    goto bad0 ;

	(void) memset(op->table,0,size) ;

#if	CF_ALLMIDDLE

	for (i = 0 ; i < op->tlen ; i += 1) {

	    op->table[i].bim = 1 ;
	    op->table[i].g0 = 1 ;
	    op->table[i].g1 = 1 ;
	    op->table[i].meta = 1 ;

	}

#else /* CF_ALLMIDDLE */

#if	CF_ALLONES
	for (i = 0 ; i < op->tlen ; i += 1)
	    op->table[i].meta = 3 ;
#endif

#endif /* CF_ALLMIDDLE */

/* we're out of here */

	op->magic = GSKEW_MAGIC ;
	return rs ;

/* bad things come here */
bad0:
	return rs ;
}
/* end subroutine (gskew_init) */


/* free up this gskew object */
int gskew_free(op)
GSKEW	*op ;
{
	int		rs = SR_BADFMT ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS2
	debugprintf("gskew_free: lu=%u bim=%u eskew=%u\n",
	    op->s.lu,op->s.use_bim,op->s.use_eskew) ;
#endif

	if (op->table != NULL) {
	    free(op->table) ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (gskew_free) */


/* lookup an IA */
int gskew_lookup(op,ia)
GSKEW	*op ;
uint	ia ;
{
	ULONG		v ;
	uint		a ;
	uint		v1, v2 ;
	int		ibim, ig0, ig1, imeta ;
	int		f_meta ;
	int		f_bim, f_g0, f_g1 ;
	int		f_pred ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("gskew_lookup: ia=%08x bhist=%08x\n",ia,op->bhistory) ;
	debugprintf("gskew_lookup: nhist=%d hmask=%08x tmask=%08x\n",
	    op->nhist,op->hmask,op->tmask) ;
#endif

	op->s.lu += 1 ;

	a = ia >> 2 ;

	v = a ;
	v = v << op->nhist ;
	v = v | (op->bhistory & op->hmask) ;

#if	CF_DEBUGS
	debugprintf("gskew_lookup: v=%016llx\n",v) ;
#endif

	v1 = v & op->tmask ;
	v2 = (v >> op->n) & op->tmask ;

#if	CF_DEBUGS
	debugprintf("gskew_lookup: v1=%08x v2=%08x\n",v1,v2) ;
#endif

	imeta = fi_meta(op->n,v1,v2) ;

	ibim = fi_bim(op->n,op->bhistory,a) & op->tmask ;

#if	CF_DEBUGS
	debugprintf("gskew_lookup: imeta=%d ibim=%d\n",
	    imeta,ibim) ;
#endif

	f_meta = GETPRED(op->table[imeta].meta) ;

	f_bim = GETPRED(op->table[ibim].bim) ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	if (f_meta) {

	    op->s.use_bim += 1 ;

	    f_pred = f_bim ;

	} else {
	    int	vote ;

	    op->s.use_eskew += 1 ;

	    ig0 = fi_g0(op->n,v1,v2) ;

	    ig1 = fi_g1(op->n,v1,v2) ;

#if	CF_DEBUGS
	    debugprintf("gskew_lookup: ig0=%d ig1=%d\n",
	        ig0,ig1) ;
#endif

	    f_g0 = GETPRED(op->table[ig0].g0) ;

	    f_g1 = GETPRED(op->table[ig1].g1) ;

	    vote = f_bim + f_g0 + f_g1 ;

	    f_pred = (vote >= 2) ;

	}

	return f_pred ;
}
/* end subroutine (gskew_lookup) */


/* get confidence */
int gskew_confidence(op,ia)
GSKEW	*op ;
uint	ia ;
{
	ULONG		v ;
	uint		a ;
	uint		v1, v2 ;
	uint		c ;
	int		ibim, ig0, ig1, imeta ;
	int		f_meta ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	a = ia >> 2 ;

	v = a ;
	v = v << op->nhist ;
	v = v | (op->bhistory & op->hmask) ;

	v1 = v & op->tmask ;
	v2 = (v >> op->n) & op->tmask ;

	imeta = fi_meta(op->n,v1,v2) ;

	ibim = fi_bim(op->n,op->bhistory,a) & op->tmask ;

	f_meta = GETPRED(op->table[imeta].meta) ;

	c = op->table[ibim].bim ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	if (f_meta) {

	    op->s.use_bim += 1 ;

	} else {
	    uint	sum ;
	    uint	vote ;

	    op->s.use_eskew += 1 ;

	    ig0 = fi_g0(op->n,v1,v2) ;

	    ig1 = fi_g1(op->n,v1,v2) ;

#if	CF_DEBUGS
	    debugprintf("gskew_lookup: ig0=%d ig1=%d\n",
	        ig0,ig1) ;
#endif

	    vote = GETPRED(c) ;
	    vote = vote | (GETPRED(op->table[ig0].g0) << 1) ;
	    vote = vote | (GETPRED(op->table[ig1].g1) << 2) ;
	    switch (vote) {

	    case 0:
	        c = 0 ;
	        break ;

	    case 1:
	        sum = op->table[ig1].g1 + op->table[ig0].g0 ;
	        c = sum / 2 ;
	        break ;

	    case 2:
	        sum = op->table[ig1].g1 + c ;
	        c = sum / 2 ;
	        break ;

	    case 3:
	        sum = op->table[ig0].g0 + c ;
	        c = sum / 2 ;
	        break ;

	    case 4:
	        sum = op->table[ig0].g0 + c ;
	        c = sum / 2 ;
	        break ;

	    case 5:
	        sum = op->table[ig1].g1 + c ;
	        c = sum / 2 ;
	        break ;

	    case 6:
	        sum = op->table[ig1].g1 + op->table[ig0].g0 ;
	        c = sum / 2 ;
	        break ;

	    case 7:
	        sum = op->table[ig1].g1 + op->table[ig0].g0 + c ;
	        c = sum / 3 ;
	        break ;

	    } /* end switch */

	} /* end if (which predictor) */

	return (c << 1) ;
}
/* end subroutine (gskew_confidence) */


/* update on branch resolution */
int gskew_update(op,ia,f_outcome)
GSKEW	*op ;
uint	ia ;
int	f_outcome ;
{
	ULONG		v ;
	uint		nc ;
	uint		a ;
	uint		v1, v2 ;
	int		ibim, ig0, ig1, imeta ;
	int		vote ;
	int		f_meta ;
	int		f_bim, f_eskew, f_g0, f_g1 ;
	int		f_pred ;
	int		f_bimagree ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("gskew_update: ia=%08x f_outcome=%d\n",ia,f_outcome) ;
#endif

	a = ia >> 2 ;

	v = a ;
	v = v << op->nhist ;
	v = v | (op->bhistory & op->hmask) ;

	v1 = v & op->tmask ;
	v2 = (v >> op->n) & op->tmask ;

	imeta = fi_meta(op->n,v1,v2) ;

	ibim = fi_bim(op->n,op->bhistory,a) & op->tmask ;

	ig0 = fi_g0(op->n,v1,v2) ;

	ig1 = fi_g1(op->n,v1,v2) ;

#if	CF_DEBUGS
	debugprintf("gskew_update: imeta=%d ibim=%d ig0=%d ig1=%d\n",
	    imeta,ibim,ig0,ig1) ;
#endif

	f_meta = GETPRED(op->table[imeta].meta) ;

	f_bim = GETPRED(op->table[ibim].bim) ;

	f_g0 = GETPRED(op->table[ig0].g0) ;

	f_g1 = GETPRED(op->table[ig1].g1) ;

	vote = f_bim + f_g0 + f_g1 ;
	f_eskew = (vote >= 2) ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	f_pred = (f_meta) ? f_bim : f_eskew ;

/* do the updating */

	if (! LEQUIV(f_outcome,f_pred)) {

/* incorrect prediction */

	    op->s.update_all += 1 ;

	    nc = 
	        satcount(op->table[ibim].bim,
	        GSKEW_STATES,f_outcome) ;

	    op->table[ibim].bim = nc ;

	    nc = 
	        satcount(op->table[ig0].g0,GSKEW_STATES,f_outcome) ;

	    op->table[ig0].g0 = nc ;

	    nc = 
	        satcount(op->table[ig1].g1,GSKEW_STATES,f_outcome) ;

	    op->table[ig1].g1 = nc ;

	} else {

/* correct prediction */

	    if (f_meta) {
	        int	f ;

	        op->s.update_bim += 1 ;

#if	CF_MUSTAGREE
	        f = LEQUIV(f_bim,f_outcome) ;
#else
	        f = TRUE ;
#endif

	        if (f) {

	            nc = satcount(op->table[ibim].bim,
	                GSKEW_STATES,f_outcome) ;

	            op->table[ibim].bim = nc ;

	        }

	    } else {

/* strengthen correct tables */

	        op->s.update_eskew += 1 ;

	        if (LEQUIV(f_bim,f_outcome)) {

	            nc = 
	                satcount(op->table[ibim].bim,
	                GSKEW_STATES,f_outcome) ;

	            op->table[ibim].bim = nc ;

	        }

	        if (LEQUIV(f_g0,f_outcome)) {

	            nc = 
	                satcount(op->table[ig0].g0,GSKEW_STATES,f_outcome) ;

	            op->table[ig0].g0 = nc ;

	        }

	        if (LEQUIV(f_g1,f_outcome)) {

	            nc = 
	                satcount(op->table[ig1].g1,GSKEW_STATES,f_outcome) ;

	            op->table[ig1].g1 = nc ;

	        }

	    }

	} /* end if (prediction/misprediction) */

/* update the META predictor */

	if (! LEQUIV(f_bim,f_eskew)) {

#if	CF_DEBUGS
	    debugprintf("gskew_update: updating META f_up=%d\n",
	        f_bimagree) ;
#endif

	    op->s.update_meta += 1 ;

	    f_bimagree = LEQUIV(f_bim,f_outcome) ;

	    if (f_bimagree)
	        op->s.updateup_meta += 1 ;

	    nc = 
	        satcount(op->table[imeta].meta,GSKEW_STATES,f_bimagree) ;

	    op->table[imeta].meta = nc ;

	} /* end if (updated META) */

/* update global branch history register */

	op->bhistory = (op->bhistory << 1) | f_outcome ;

	return f_pred ;
}
/* end subroutine (gskew_update) */


/* zero out the statistics */
int gskew_zerostats(op)
GSKEW		*op ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;

	memset(&op->s,0,sizeof(GSKEW_STATS)) ;

	return rs ;
}
/* end subroutine (gskew_zerostats) */


/* get the statistics about this particular predictor */
int gskew_stats(op,rp)
GSKEW		*op ;
GSKEW_STATS	*rp ;
{
	int		bits_total ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSKEW_MAGIC) return SR_NOTOPEN ;

/* calculate the bits */

	{
	    uint	bits_bim ;
	    uint	bits_g0 ;
	    uint	bits_g1 ;
	    uint	bits_meta ;
	    uint	bits_history ;

	    bits_bim = op->tlen * 2 ;

	    bits_g0 = op->tlen * 2 ;

	    bits_g1 = op->tlen * 2 ;

	    bits_meta = op->tlen * 2 ;

	    bits_history = op->nhist ;

	    bits_total = 
	        bits_bim + bits_g0 + bits_g1 + bits_meta + bits_history ;

	} /* end block */

/* fill in the extra stuff */

	if (rp != NULL) {

	    memcpy(rp,&op->s,sizeof(GSKEW_STATS)) ;

	    rp->tlen = op->tlen ;
	    rp->bits = bits_total ;

	}

	return bits_total ;
}
/* end subroutine (gskew_stats) */


/* private subroutines */


static uint satcount(v,n,f_up)
uint	v ;
uint	n ;
int	f_up ;
{
	uint		r ;

	if (f_up) {
	    r = (v == (n - 1)) ? v : (v + 1) ;
	} else {
	    r = (v == 0) ? 0 : (v - 1) ;
	}

	return r ;
}
/* end subroutine (satcount) */


/* index function for the BIM */
static uint fi_bim(n,h,a)
int	n ;
uint	h, a ;
{

	return a ;
}
/* end subroutine (fi_bim) */


static uint fi_g0(n,v1,v2)
int	n ;
uint	v1, v2 ;
{

	return h(n,v1) ^ hinv(n,v2) ^ v1 ;
}
/* end subroutine (fi_g0) */


static uint fi_g1(n,v1,v2)
int	n ;
uint	v1, v2 ;
{

	return hinv(n,v1) ^ h(n,v2) ^ v2 ;
}
/* end subroutine (fi_g1) */


static uint fi_meta(n,v1,v2)
int	n ;
uint	v1, v2 ;
{

	return h(n,v1) ^ hinv(n,v2) ^ v2 ;
}
/* end subroutine (fi_meta) */


/* forward H function */
static uint h(n,v)
int	n ;
uint	v ;
{

	return (v >> 1) | ((BIT(v,(n - 1)) ^ (v & 1)) << (n - 1)) ;
}
/* end subroutine (h) */


/* inverse H function */
static uint hinv(n,v)
int	n ;
uint	v ;
{

	return (v << 1) | (BIT(v,(n - 1)) ^ BIT(v,(n - 2))) ;
}
/* end subroutine (hinv) */


