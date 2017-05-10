/* eveight */

/* this is a EVEIGHT branch predictor */


#define	CF_DEBUGS	0
#define	CF_SAFE		1
#define	CF_VOTEREPLACE	0		/* replace by voting among counters */
#define	CF_COUNTREPLACE	1		/* replace by counting all counters */


/* revision history:

	= 02/05/01, David A­D­ Morano

	This object module was created for Levo research.
	It is a value predictor.  This is not coded as
	hardware.  It is like Atom analysis subroutines !


*/



/******************************************************************************

	This object module implements a branch predictor.  This BP is a
	Tournament type branch predictor (see McFarling and then Alpha
	21264).


*****************************************************************************/


#define	EVEIGHT_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"bpload.h"
#include	"eveight.h"



/* local defines */

#define	EVEIGHT_MAGIC		0x45678429
#define	EVEIGHT_DEFLEN		(64 * 1024)
#define	EVEIGHT_STATES		4
#define	EVEIGHT_DEFGLEN		4		/* default entries */
#define	EVEIGHT_DEFLPLEN	4		/* default entries */
#define	EVEIGHT_DEFLBLEN	4		/* default entries */
#define	EVEIGHT_LPHSTATES	8		/* LPHT states */
#define	EVEIGHT_GPHSTATES	4		/* GPHT states */
#define	EVEIGHT_GCHSTATES	4		/* CPHT states */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	GETPRED(c)	(((c) >> 1) & 1)
#define	GETPRED2(c)	(((c) >> 1) & 1)
#define	GETPRED3(c)	(((c) >> 2) & 1)

#define	BIT(w,n)	(((w) >> (n)) & 1)

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
static uint	fi_bim(uint,uint) ;
static uint	fi_g0(uint,uint) ;
static uint	fi_g1(uint,uint) ;
static uint	fi_meta(uint,uint) ;


/* global variables */

struct bpload	eveight = {
	"eveight",
	sizeof(EVEIGHT),
} ;


/* local variables */







int eveight_init(op,p1,p2,p3,p4)
EVEIGHT	*op ;
int	p1, p2, p3, p4 ;
{
	int	rs ;
	int	size ;
	int	max ;


	if (op == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(EVEIGHT)) ;


	max = -1 ;
	max = MAX(p1,p2) ;

	if (p3 > max)
	    max = p3 ;

	if (p4 > max)
	    max = p4 ;

	if (max < 0)
	    max = EVEIGHT_DEFLEN ;

	op->tlen = nextpowtwo(max) ;

	op->tmask = op->tlen - 1 ;

#if	CF_DEBUGS
	debugprintf("eveight_init: tlen=%d tmask=%08x\n",
		op->tlen,op->tmask) ;
#endif

/* allocate the space */

	size = op->tlen * sizeof(struct eveight_banks) ;
	rs = uc_malloc(size,&op->table) ;

	if (rs < 0)
	    goto bad0 ;

	(void) memset(op->table,0,size) ;


/* we're out of here */

	op->magic = EVEIGHT_MAGIC ;
	return rs ;

/* bad things come here */
bad0:
	return rs ;
}
/* end subroutine (eveight_init) */


/* free up this eveight object */
int eveight_free(op)
EVEIGHT	*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;


	if (op->table != NULL)
	    free(op->table) ;


	op->magic = 0 ;
	return rs ;
}
/* end subroutine (eveight_free) */


/* lookup an IA */
int eveight_lookup(op,ia)
EVEIGHT	*op ;
uint	ia ;
{
	uint	a ;

	int	rs ;
	int	ibim, ig0, ig1, imeta ;
	int	f_meta ;
	int	f_bim, f_g0, f_g1 ;
	int	f_pred ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	a = ia >> 2 ;

	imeta = fi_meta(op->bhistory,a) & op->tmask ;

	ibim = fi_bim(op->bhistory,a) & op->tmask ;

#if	CF_DEBUGS
	debugprintf("eveight_lookup: ibim=%d ig0=%d ig1=%d imeta=%d\n",
	    ibim,ig0,ig1,imeta) ;
#endif

	f_meta = GETPRED(op->table[imeta].meta) ;

	f_bim = GETPRED(op->table[ibim].bim) ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	if (f_meta) {

	    f_pred = f_bim ;

	} else {

	    int	c ;


	    ig0 = fi_g0(op->bhistory,a) & op->tmask ;

	    ig1 = fi_g1(op->bhistory,a) & op->tmask ;

	    f_g0 = GETPRED(op->table[ig0].g0) ;

	    f_g1 = GETPRED(op->table[ig1].g1) ;

	    c = f_bim + f_g0 + f_g1 ;

	    f_pred = (c >= 2) ;

	}

	return f_pred ;
}
/* end subroutine (eveight_lookup) */


#ifdef	COMMENT

/* get confidence */
int eveight_confidence(op,ia)
EVEIGHT	*op ;
uint	ia ;
{
	int	rs ;
	int	lbi, lpi ;
	int	gi ;
	int	pred ;
	int	f_select ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	a = ia >> 2 ;

	gi = op->bhistory & op->historymask ;

#if	CF_DEBUGS
	debugprintf("eveight_confidence: gi=%d\n",gi) ;
#endif

	f_select = GETPRED(op->cpht[gi]) ;

	if (f_select) {

	    pred = op->gpht[gi] * 2 ;

	} else {

	    lbi = a % op->lhlen ;
	    lpi = op->lbht[lbi] % op->lplen ;

#if	CF_DEBUGS
	    debugprintf("eveight_confidence: lbi=%d lpi=%d\n",lbi,lpi) ;
#endif

	    pred = op->lpht[lpi] ;

	}

	return pred ;
}
/* end subroutine (eveight_confidence) */

#endif /* COMMENT */


/* update on branch resolution */
int eveight_update(op,ia,f_outcome)
EVEIGHT	*op ;
uint	ia ;
int	f_outcome ;
{
	uint	a ;
	uint	nc ;

	int	rs ;
	int	ibim, ig0, ig1, imeta ;
	int	vote ;
	int	f_meta ;
	int	f_bim, f_eskew, f_g0, f_g1 ;
	int	f_agree ;
	int	f_pred ;
	int	f_bimagree ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	a = ia >> 2 ;

	imeta = fi_meta(op->bhistory,a) & op->tmask ;

	ibim = fi_bim(op->bhistory,a) & op->tmask ;

	ig0 = fi_g0(op->bhistory,a) & op->tmask ;

	ig1 = fi_g1(op->bhistory,a) & op->tmask ;

#if	CF_DEBUGS
	debugprintf("eveight_update: ibim=%d ig0=%d ig1=%d imeta=%d\n",
	    ibim,ig0,ig1,imeta) ;
#endif

	{

	    f_meta = GETPRED(op->table[imeta].meta) ;

	    f_bim = GETPRED(op->table[ibim].bim) ;

	    f_g0 = GETPRED(op->table[ig0].g0) ;

	    f_g1 = GETPRED(op->table[ig1].g1) ;

	    vote = f_bim + f_g0 + f_g1 ;
	    f_eskew = (vote >= 2) ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	    f_pred = (f_meta) ? f_bim : f_eskew ;

	}

/* do the updating */

	f_agree = LEQUIV(f_bim,f_g0) && LEQUIV(f_bim,f_g1) ;
	if (LEQUIV(f_outcome,f_pred) && (! f_agree)) {

	    if (! LEQUIV(f_bim,f_eskew)) {

/* strengthen META */

	        nc = 
	            satcount(op->table[imeta].meta,EVEIGHT_STATES,f_meta) ;

	        op->table[imeta].meta = nc ;

	    }

/* strengthen others */

	    if (f_meta) {

/* strengthen BIM */

	        nc = 
	            satcount(op->table[ibim].bim,EVEIGHT_STATES,f_outcome) ;

	        op->table[ibim].bim = nc ;


	    } else {

/* strengthen correct tables */

	        if (LEQUIV(f_bim,f_outcome)) {

	            nc = 
	                satcount(op->table[ibim].bim,
			EVEIGHT_STATES,f_outcome) ;

	            op->table[ibim].bim = nc ;

	        }

	        if (LEQUIV(f_g0,f_outcome)) {

	            nc = 
	                satcount(op->table[ig0].g0,EVEIGHT_STATES,f_outcome) ;

	            op->table[ig0].g0 = nc ;

	        }

	        if (LEQUIV(f_g1,f_outcome)) {

	            nc = 
	                satcount(op->table[ig1].g1,EVEIGHT_STATES,f_outcome) ;

	            op->table[ig1].g1 = nc ;

	        }

	    }

	} else {

	    if (! LEQUIV(f_bim,f_eskew)) {

/* strengthen META */

		f_bimagree = LEQUIV(f_bim,f_outcome) ;
	        nc = satcount(op->table[imeta].meta,
			EVEIGHT_STATES,f_bimagree) ;

	        op->table[imeta].meta = nc ;

/* re-compute */

	        {

	            f_meta = GETPRED(op->table[imeta].meta) ;

	            f_bim = GETPRED(op->table[ibim].bim) ;

	            f_g0 = GETPRED(op->table[ig0].g0) ;

	            f_g1 = GETPRED(op->table[ig1].g1) ;

	            vote = f_bim + f_g0 + f_g1 ;
	            f_eskew = (vote >= 2) ;

/* BIM will be "UP", ESKEW will be "DOWN" */

	            f_pred = (f_meta) ? f_bim : f_eskew ;

	        }

	        if (LEQUIV(f_pred,f_outcome)) {

/* "strengthen participating tables" */

	            nc = satcount(op->table[ibim].bim,
			EVEIGHT_STATES,f_outcome) ;

	            op->table[ibim].bim = nc ;

	            if (! f_meta) {

	                nc = satcount(op->table[ig0].g0,
				EVEIGHT_STATES,f_outcome) ;

	                op->table[ig0].g0 = nc ;

	                nc = satcount(op->table[ig1].g1,
				EVEIGHT_STATES,f_outcome) ;

	                op->table[ig1].g1 = nc ;

	            }

	        } else {

/* update all "banks" */

	            nc = satcount(op->table[ibim].bim,
				EVEIGHT_STATES,f_outcome) ;

	            op->table[ibim].bim = nc ;

	            nc = 
	                satcount(op->table[ig0].g0,EVEIGHT_STATES,f_outcome) ;

	            op->table[ig0].g0 = nc ;

	            nc = 
	                satcount(op->table[ig1].g1,EVEIGHT_STATES,f_outcome) ;

	            op->table[ig1].g1 = nc ;

	        }

	    }

	} /* end if (prediction/misprediction) */

/* update global branch history register */

	op->bhistory = (op->bhistory << 1) | f_outcome ;


	return 0 ;
}
/* end subroutine (eveight_update) */


/* zero out the statistics */
int eveight_zerostats(op)
EVEIGHT		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;

	(void) memset(&op->s,0,sizeof(EVEIGHT_STATS)) ;

	return rs ;
}
/* end subroutine (eveight_zerostats) */


/* get the statistics about this particular predictor */
int eveight_stats(op,rp)
EVEIGHT		*op ;
EVEIGHT_STATS	*rp ;
{
	int	bits_total ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != EVEIGHT_MAGIC)
	    return SR_NOTOPEN ;

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

	    bits_history = 30 ;

	    bits_total = 
	        bits_bim + bits_g0 + bits_g1 + bits_meta + bits_history ;

	}

/* fill in the extra stuff */

	if (rp != NULL) {

	(void) memcpy(rp,&op->s,sizeof(EVEIGHT_STATS)) ;

	rp->tlen = op->tlen ;
	rp->bits = bits_total ;

	}

	return bits_total ;
}
/* end subroutine (eveight_stats) */



/* INTERNAL SUBROUTINES */



static uint satcount(v,n,f_up)
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


static uint fi_bim(h,a)
uint	h, a ;
{
	int	ibank, iline, iextra_lo, iextra_hi ;
	int	imore ;
	int	index ;


	ibank = (BIT(a,6) << 1) | BIT(a,5) ;

	iline = ((h & 0x0f) << 2) | ((a >> 7) & 0x03) ;

	iextra_hi = (BIT(a,11) << 2) |
	    ((BIT(a,9) ^ BIT(a,5)) << 1) |
	    (BIT(a,10) ^ BIT(a,6)) ;

	iextra_lo = (BIT(a,4)  << 2) |
	    ((BIT(a,3) ^ BIT(a,6)) << 1) |
	    (BIT(a,2) ^ BIT(a,5)) ;

	imore = (BIT(a,15) << 1) | BIT(a,14) ;

	index = (imore << 14) | (iextra_hi << 11) | (iline << 5) | 
	    (iextra_lo << 2) | ibank ;

	return index ;
}
/* end subroutine (fi_bim) */


static uint fi_g0(h,a)
uint	h, a ;
{
	int	ibank, iline, iextra_lo, iextra_hi ;
	int	imore ;
	int	index ;


	ibank = (BIT(a,6) << 1) | BIT(a,5) ;

	iline = ((h & 0x0f) << 2) | ((a >> 7) & 0x03) ;

	iextra_hi = ((BIT(h,7) ^ BIT(h,11)) << 4) |
	    ((BIT(h,8) ^ BIT(h,12)) << 3) |
	    ((BIT(h,4) ^ BIT(h,5)) << 2) |
	    ((BIT(a,9) ^ BIT(h,9)) << 1) |
	    ((BIT(h,10) ^ BIT(h,6)) << 0) ;

	{
	    uint	i4, i3, i2 ;


	    i4 = BIT(a,4) ^ BIT(a,9) ^ BIT(a,13) ^
	        BIT(a,12) ^ BIT(h,5) ^ 
	        BIT(h,5) ^ BIT(h,11) ^ BIT(h,8) ^
	        BIT(a,5) ;

	    i3 = BIT(a,3) ^ BIT(a,11) ^
	        BIT(h,9) ^ BIT(h,10) ^ BIT(h,12) ^
	        BIT(a,6) ^ BIT(a,5) ;

	    i2 = BIT(a,2) ^ BIT(a,14) ^ BIT(a,10) ^
	        BIT(h,6) ^ BIT(h,4) ^ BIT(h,7) ^
	        BIT(a,6) ;

	    iextra_lo = (i4 << 2) | (i3 << 1) | i2 ;

	}

	index = (iextra_hi << 11) | (iline << 5) | 
	    (iextra_lo << 2) | ibank ;

	return index ;
}
/* end subroutine (fi_g0) */


static uint fi_g1(h,a)
uint	h, a ;
{
	int	ibank, iline, iextra_lo, iextra_hi ;
	int	imore ;
	int	index ;


	ibank = (BIT(a,6) << 1) | BIT(a,5) ;

	iline = ((h & 0x0f) << 2) | ((a >> 7) & 0x03) ;

	iextra_hi = ((BIT(h,19) ^ BIT(h,12)) << 4) |
	    ((BIT(h,18) ^ BIT(h,11)) << 3) |
	    ((BIT(h,17) ^ BIT(h,10)) << 2) |
	    ((BIT(a,16) ^ BIT(h,4)) << 1) |
	    ((BIT(h,15) ^ BIT(h,20)) << 0) ;

	{
	    uint	i4, i3, i2 ;


	    i4 = BIT(a,4) ^ BIT(a,11) ^ BIT(a,14) ^ BIT(a,6) ^
	        BIT(h,4) ^ BIT(h,6) ^ BIT(h,9) ^
	        BIT(h,14) ^ BIT(h,15) ^ BIT(h,16) ^
	        BIT(a,6) ;

	    i3 = BIT(a,3) ^ BIT(a,10) ^ BIT(a,13) ^
	        BIT(h,5) ^ BIT(h,11) ^ BIT(h,13) ^ 
	        BIT(h,18) ^ BIT(h,19) ^ BIT(h,20) ^
	        BIT(a,5) ;

	    i2 = BIT(a,2) ^ BIT(a,5) ^ BIT(a,9) ^
	        BIT(h,4) ^ BIT(h,8) ^ BIT(h,7) ^ BIT(a,10) ^
	        BIT(h,12) ^ BIT(h,13) ^ BIT(h,14) ^ BIT(h,17) ;

	    iextra_lo = (i4 << 2) | (i3 << 1) | i2 ;

	}

	index = (iextra_hi << 11) | (iline << 5) | 
	    (iextra_lo << 2) | ibank ;

	return index ;
}
/* end subroutine (fi_g1) */


static uint fi_meta(h,a)
uint	h, a ;
{
	int	ibank, iline, iextra_lo, iextra_hi ;
	int	imore ;
	int	index ;


	ibank = (BIT(a,6) << 1) | BIT(a,5) ;

	iline = ((h & 0x0f) << 2) | ((a >> 7) & 0x03) ;

	iextra_hi = ((BIT(h,7) ^ BIT(h,11)) << 4) |
	    ((BIT(h,8) ^ BIT(h,12)) << 3) |
	    ((BIT(h,5) ^ BIT(h,13)) << 2) |
	    ((BIT(a,4) ^ BIT(h,9)) << 1) |
	    ((BIT(h,9) ^ BIT(h,6)) << 0) ;

	{
	    uint	i4, i3, i2 ;


	    i4 = BIT(a,4) ^ BIT(a,10) ^ BIT(a,5) ^
	        BIT(h,7) ^ BIT(h,10) ^ BIT(h,14) ^ BIT(h,13) ^
	        BIT(a,5) ;

	    i3 = BIT(a,3) ^ BIT(a,12) ^ BIT(a,14) ^ BIT(a,6) ^
	        BIT(h,4) ^ BIT(h,6) ^ BIT(h,8) ^ BIT(h,14) ;

	    i2 = BIT(a,2) ^ BIT(a,9) ^ BIT(a,11) ^ BIT(a,13) ^
	        BIT(h,5) ^ BIT(h,9) ^ BIT(h,11) ^ BIT(a,12) ^
	        BIT(a,6) ;

	    iextra_lo = (i4 << 2) | (i3 << 1) | i2 ;

	}

	index = (iextra_hi << 11) | (iline << 5) | 
	    (iextra_lo << 2) | ibank ;

	return index ;
}
/* end subroutine (fi_meta) */



