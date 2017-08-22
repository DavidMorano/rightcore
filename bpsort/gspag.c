/* gspag */

/* this is a gshare-PAg branch predictor */


#define	CF_DEBUGS	0
#define	CF_SAFE		1
#define	CF_VOTEREPLACE	0		/* replace by voting among counters */
#define	CF_COUNTREPLACE	1		/* replace by counting all counters */


/* revision history:

	= 2002-05-01, David A­D­ Morano
        This object module was created for Levo research. It is a value
        predictor. This is not coded as hardware. It is like Atom analysis
        subroutines!

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This object module implements a branch predictor. This BP is a GSPAG
        (see Patt and then McFarling) type branch predictor.


*****************************************************************************/


#define	GSPAG_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bpload.h"
#include	"gspag.h"


/* local defines */

#define	GSPAG_DEFBHLEN	4		/* default entries */
#define	GSPAG_DEFPHLEN	4		/* default entries */
#define	GSPAG_GPHSTATES	4		/* GPHT states */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	GETPRED(c)	(((c) >> 1) & 1)
#define	GETPRED2(c)	(((c) >> 1) & 1)
#define	GETPRED3(c)	(((c) >> 2) & 1)


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	flbsi(uint) ;
extern int	uc_malloc() ;


/* forward references */

static uint	satcount(uint,uint,int) ;


/* global variables */

struct bpload	gspag = {
	"gspag",
	sizeof(GSPAG),
} ;


/* local variables */


/* exported subroutines */


int gspag_init(op,bhlen,phlen)
GSPAG	*op ;
int	bhlen ;
int	phlen ;
{
	int		rs ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(GSPAG)) ;

/* BHT */

	if (bhlen < 2)
	    bhlen = GSPAG_DEFBHLEN ;

	op->bhlen = nextpowtwo(bhlen) ;

	size = op->bhlen * sizeof(uint) ;
	rs = uc_malloc(size,&op->lbht) ;

	if (rs < 0)
	    goto bad0 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->lbht,rs,"gspag_init:lbht") ;
#endif

	(void) memset(op->lbht,0,size) ;

/* global PHT */

	if (phlen < 2)
	    phlen = GSPAG_DEFPHLEN ;

	op->phlen = nextpowtwo(phlen) ;

	size = op->phlen * sizeof(uchar) ;
	rs = uc_malloc(size,&op->gpht) ;

	if (rs < 0)
	    goto bad1 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->gpht,rs,"gspag_init:gpht") ;
#endif

	(void) memset(op->gpht,0,size) ;


/* we're out of here */

	op->magic = GSPAG_MAGIC ;
	return rs ;

/* we're out of here */
bad1:
	free(op->lbht) ;

#ifdef	MALLOCLOG
	malloclog_free(op->lbht,"gspag_init:lbht") ;
#endif

bad0:
	return rs ;
}
/* end subroutine (gspag_init) */


/* free up this gspag object */
int gspag_free(op)
GSPAG	*op ;
{
	int		rs = SR_BADFMT ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;

	if (op->gpht != NULL) {
	    free(op->gpht) ;
	}

	if (op->lbht != NULL) {
	    free(op->lbht) ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (gspag_free) */


/* lookup an IA */
int gspag_lookup(op,ia)
GSPAG	*op ;
uint	ia ;
{
	int		lbi, gpi ;
	int		f_pred ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	lbi = (ia >> 2) % op->bhlen ;
	gpi = (op->lbht[lbi] ^ (ia >> 2)) % op->phlen ;

#if	CF_DEBUGS
	debugprintf("gspag_lookup: lbi=%d gpi=%d\n",lbi,gpi) ;
#endif

	f_pred = GETPRED(op->gpht[gpi]) ;

	return f_pred ;
}
/* end subroutine (gspag_lookup) */


/* lookup an IA to get its confidence prediction */
int gspag_confidence(op,ia)
GSPAG	*op ;
uint	ia ;
{
	int		rs ;
	int		lbi, gpi ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	lbi = (ia >> 2) % op->bhlen ;
	gpi = (op->lbht[lbi] ^ (ia >> 2)) % op->phlen ;

#if	CF_DEBUGS
	debugprintf("gspag_lookup: lbi=%d gpi=%d\n",lbi,gpi) ;
#endif

	rs = op->gpht[gpi] ;

	return (rs << 1) ;
}
/* end subroutine (gspag_confidence) */


/* update on branch resolution */
int gspag_update(op,ia,f_outcome)
GSPAG	*op ;
uint	ia ;
int	f_outcome ;
{
	uint		ncount ;
	int		lbi, gpi ;
	int		f_pred ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	lbi = (ia >> 2) % op->bhlen ;
	gpi = (op->lbht[lbi] ^ (ia >> 2)) % op->phlen ;
	f_pred = GETPRED(op->gpht[gpi]) ;

/* update GPHT */

	ncount = satcount(op->gpht[gpi],GSPAG_GPHSTATES,f_outcome) ;

	op->gpht[gpi] = ncount ;

/* update local BHT */

	op->lbht[lbi] = (op->lbht[lbi] << 1) | f_outcome ;

	return f_pred ;
}
/* end subroutine (gspag_update) */


#ifdef	COMMENT

/* get the entries (serially) */
int gspag_get(op,ri,rpp)
GSPAG		*op ;
int		ri ;
GSPAG_ENT	**rpp ;
{

#if	CF_DEBUGS
	debugprintf("gspag_get: ent 0\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;

	if ((ri < 0) || (ri >= op->tablen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->table + ri ;

#if	CF_DEBUGS
	debugprintf("gspag_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (gspag_get) */

#endif /* COMMENT */

/* zero out the statistics */
int gspag_zerostats(op)
GSPAG		*op ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;

	memset(&op->s,0,sizeof(GSPAG_STATS)) ;

	return rs ;
}
/* end subroutine (gspag_zerostats) */


/* get the statistics about this particular predictor */
int gspag_stats(op,rp)
GSPAG		*op ;
GSPAG_STATS	*rp ;
{
	int		bits_total ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != GSPAG_MAGIC) return SR_NOTOPEN ;

/* calculate the bits */

	{
		uint	bits_lbht ;
		uint	bits_gpht ;
		uint	bits_history ;
		int	n ;

		n = flbsi(op->phlen) ;

		bits_lbht = op->bhlen * n ;

		bits_history = flbsi(op->phlen) ;

		bits_gpht = op->phlen * GSPAG_COUNTBITS ;

		bits_total = bits_lbht + bits_gpht + bits_history ;

	} /* end block */

/* fill in the extra stuff */

	if (rp != NULL) {

	memcpy(rp,&op->s,sizeof(GSPAG_STATS)) ;
	rp->lbht = op->bhlen ;
	rp->gpht = op->phlen ;
	rp->bits = bits_total ;

	}

	return bits_total ;
}
/* end subroutine (gspag_stats) */


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


