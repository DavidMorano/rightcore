/* bpalpha */

/* this is a BPALPHA branch predictor */


#define	CF_DEBUGS	0
#define	CF_SAFE		1
#define	CF_VOTEREPLACE	0		/* replace by voting among counters */
#define	CF_COUNTREPLACE	1		/* replace by counting all counters */


/* revision history:

	= 02/05/01, David A­D­ Morano

	This object module was created for Levo research.


*/


/******************************************************************************

	This object module implements a branch predictor.  This BP is a
	Tournament type branch predictor (see McFarling and then Alpha
	21264).


*****************************************************************************/


#define	BPALPHA_MASTER	0


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
#include	"bpalpha.h"



/* local defines */

#define	BPALPHA_MAGIC		0x29456781
#define	BPALPHA_DEFGLEN		4		/* default entries */
#define	BPALPHA_DEFLPLEN	4		/* default entries */
#define	BPALPHA_DEFLBLEN	4		/* default entries */
#define	BPALPHA_LPHSTATES	8		/* LPHT states */
#define	BPALPHA_GPHSTATES	4		/* GPHT states */
#define	BPALPHA_GCHSTATES	4		/* CPHT states */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	GETPRED(c)	(((c) >> 1) & 1)
#define	GETPRED2(c)	(((c) >> 1) & 1)
#define	GETPRED3(c)	(((c) >> 2) & 1)

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


/* forward references */

static uint	satcount(uint,uint,int) ;


/* global variables */

struct bpload	bpalpha = {
	"bpalpha",
	sizeof(BPALPHA),
} ;


/* local variables */







int bpalpha_init(op,lhlen,lplen,glen)
BPALPHA	*op ;
int	lhlen ;
int	lplen ;
int	glen ;
{
	int	rs ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(BPALPHA)) ;


/* the choice PHT */

	if (glen <= 2)
	    glen = BPALPHA_DEFGLEN ;

	op->glen = nextpowtwo(glen) ;

	size = op->glen * sizeof(uchar) ;

/* choice PHT */

	rs = uc_malloc(size,&op->cpht) ;

	if (rs < 0)
	    goto bad0 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->cpht,rs,"bpalpha_init:cpht") ;
#endif

	(void) memset(op->cpht,0,size) ;

/* global PHT */

	rs = uc_malloc(size,&op->gpht) ;

	if (rs < 0)
	    goto bad1 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->gpht,rs,"bpalpha_init:gpht") ;
#endif

	(void) memset(op->gpht,0,size) ;

/* local BHT */

	if (lhlen <= 2)
	    lhlen = BPALPHA_DEFLBLEN ;

	op->lhlen = nextpowtwo(lhlen) ;

	size = op->lhlen * sizeof(uint) ;
	rs = uc_malloc(size,&op->lbht) ;

#if	CF_DEBUGS
	debugprintf("bpalpha_init: uc_malloc() rs=%d lbht=%p\n",
	    rs, op->lbht) ;
#endif

	if (rs < 0)
	    goto bad2 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->lbht,rs,"bpalpha_init:lbht") ;
#endif

	(void) memset(op->lbht,0,size) ;

/* local PHT */

	if (lplen <= 2)
	    lplen = BPALPHA_DEFLPLEN ;

	op->lplen = nextpowtwo(lplen) ;

	size = op->lplen * sizeof(uchar) ;
	rs = uc_malloc(size,&op->lpht) ;

#if	CF_DEBUGS
	debugprintf("bpalpha_init: uc_malloc() rs=%d lpht=%p\n",
	    rs, op->lpht) ;
#endif

	if (rs < 0)
	    goto bad3 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->lpht,rs,"bpalpha_init:lpht") ;
#endif

	(void) memset(op->lpht,0,size) ;

/* global branch history register */

	op->historymask = (op->glen - 1) ;


/* we're out of here */

	op->magic = BPALPHA_MAGIC ;
	return rs ;

/* we're out of here */
bad3:
	free(op->lbht) ;

#ifdef	MALLOCLOG
	malloclog_free(op->lbht,"bpalpha_init:lbht") ;
#endif

bad2:
	free(op->gpht) ;

#ifdef	MALLOCLOG
	malloclog_free(op->gpht,"bpalpha_init:gpht") ;
#endif

bad1:
	free(op->cpht) ;

#ifdef	MALLOCLOG
	malloclog_free(op->cpht,"bpalpha_init:cpht") ;
#endif

bad0:
	return rs ;
}
/* end subroutine (bpalpha_init) */


/* free up this bpalpha object */
int bpalpha_free(op)
BPALPHA	*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;

	if (op->lpht != NULL) {

	    free(op->lpht) ;

#ifdef	MALLOCLOG
	    malloclog_free(op->lpht,"bpalpha_free:lpht") ;
#endif

	}

	if (op->lbht != NULL) {

	    free(op->lbht) ;

#ifdef	MALLOCLOG
	    malloclog_free(op->lbht,"bpalpha_free:lbht") ;
#endif

	}

	if (op->gpht != NULL) {

	    free(op->gpht) ;

#ifdef	MALLOCLOG
	    malloclog_free(op->gpht,"bpalpha_free:gpht") ;
#endif

	}

	if (op->cpht != NULL) {

	    free(op->cpht) ;

#ifdef	MALLOCLOG
	    malloclog_free(op->cpht,"bpalpha_free:cpht") ;
#endif

	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bpalpha_free) */


/* lookup an IA */
int bpalpha_lookup(op,ia)
BPALPHA	*op ;
uint	ia ;
{
	int	lbi, lpi ;
	int	gi ;
	int	f_pred ;
	int	f_select ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	gi = op->bhistory & op->historymask ;

#if	CF_DEBUGS
	debugprintf("bpalpha_lookup: gi=%d\n",gi) ;
#endif

	f_select = GETPRED(op->cpht[gi]) ;

	if (f_select) {

	    f_pred = GETPRED(op->gpht[gi]) ;

	} else {

	    lbi = (ia >> 2) % op->lhlen ;
	    lpi = op->lbht[lbi] % op->lplen ;

#if	CF_DEBUGS
	    debugprintf("bpalpha_lookup: lbi=%d lpi=%d\n",lbi,lpi) ;
#endif

	    f_pred = GETPRED3(op->lpht[lpi]) ;

	}

	return f_pred ;
}
/* end subroutine (bpalpha_lookup) */


/* get confidence */
int bpalpha_confidence(op,ia)
BPALPHA	*op ;
uint	ia ;
{
	int	lbi, lpi ;
	int	gi ;
	int	pred ;
	int	f_select ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	gi = op->bhistory & op->historymask ;

#if	CF_DEBUGS
	debugprintf("bpalpha_lookup: gi=%d\n",gi) ;
#endif

	f_select = GETPRED(op->cpht[gi]) ;

	if (f_select) {

	    pred = op->gpht[gi] * 2 ;

	} else {

	    lbi = (ia >> 2) % op->lhlen ;
	    lpi = op->lbht[lbi] % op->lplen ;

#if	CF_DEBUGS
	    debugprintf("bpalpha_lookup: lbi=%d lpi=%d\n",lbi,lpi) ;
#endif

	    pred = op->lpht[lpi] ;

	}

	return pred ;
}
/* end subroutine (bpalpha_confidence) */


/* update on branch resolution */
int bpalpha_update(op,ia,f_outcome)
BPALPHA	*op ;
uint	ia ;
int	f_outcome ;
{
	uint	ncount ;

	int	rs ;
	int	lbi, lpi ;
	int	gi ;
	int	f_lpred, f_gpred ;
	int	f_lagree, f_gagree ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	lbi = (ia >> 2) % op->lhlen ;

/* update local PHT */

	lpi = op->lbht[lbi] % op->lplen ;
	f_lpred = GETPRED3(op->lpht[lpi]) ;
	ncount = satcount(op->lpht[lpi],BPALPHA_LPHSTATES,f_outcome) ;

	op->lpht[lpi] = ncount ;

/* update local BHT */

	op->lbht[lbi] = (op->lbht[lbi] << 1) | f_outcome ;

/* update GPHT */

	gi = op->bhistory & op->historymask ;
	f_gpred = GETPRED(op->gpht[gi]) ;
	ncount = satcount(op->gpht[gi],BPALPHA_GPHSTATES,f_outcome) ;

	op->gpht[gi] = ncount ;

/* update CPHT (global is UP, and local is DOWN) */

	f_lagree = LEQUIV(f_lpred,f_outcome) ;
	f_gagree = LEQUIV(f_gpred,f_outcome) ;
	if (! LEQUIV(f_lagree,f_gagree)) {

	    ncount = satcount(op->cpht[gi],BPALPHA_GCHSTATES,f_gagree) ;

	    op->cpht[gi] = ncount ;

	} /* end if (conditional update) */

/* update global branch history register */

	op->bhistory = (op->bhistory << 1) | f_outcome ;

	return 0 ;
}
/* end subroutine (bpalpha_update) */


#ifdef	COMMENT

/* get the entries (serially) */
int bpalpha_get(op,ri,rpp)
BPALPHA		*op ;
int		ri ;
BPALPHA_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("bpalpha_get: entered 0\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("bpalpha_get: entered 1\n") ;
#endif

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bpalpha_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("bpalpha_get: entered ri=%d\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->tablen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->table + ri ;

#if	CF_DEBUGS
	debugprintf("bpalpha_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (bpalpha_get) */

#endif /* COMMENT */


/* zero out the statistics */
int bpalpha_zerostats(op)
BPALPHA		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;

	(void) memset(&op->s,0,sizeof(BPALPHA_STATS)) ;

	return rs ;
}
/* end subroutine (bpalpha_zerostats) */


/* get the statistics about this particular predictor */
int bpalpha_stats(op,rp)
BPALPHA		*op ;
BPALPHA_STATS	*rp ;
{
	int	bits_total ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BPALPHA_MAGIC)
	    return SR_NOTOPEN ;

/* calculate the bits */

	{
	    uint	bits_lbht ;
	    uint	bits_lpht ;
	    uint	bits_lhistory ;
	    uint	bits_gpht ;
	    uint	bits_ghistory ;


	    bits_ghistory = flbsi(op->lplen) ;

	    bits_lbht = op->lhlen * bits_ghistory ;
	    bits_lpht = op->lplen * 3 ;

	    bits_ghistory = flbsi(op->glen) ;

/* there are two of these tables (the selector and the real GPHT) */

	    bits_gpht = 2 * op->glen * 2 ;

	    bits_total = bits_lbht + bits_lpht + bits_lhistory +
		bits_gpht + bits_ghistory ;

	}

/* fill in the extra stuff */

	if (rp != NULL) {

	(void) memcpy(rp,&op->s,sizeof(BPALPHA_STATS)) ;

	rp->lbht = op->lhlen ;
	rp->lpht = op->lplen ;
	rp->gpht = op->glen ;
	rp->bits = bits_total ;

	}

	return bits_total ;
}
/* end subroutine (bpalpha_stats) */



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



