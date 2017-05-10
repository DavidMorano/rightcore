/* vpred */

/* value prediction object */


#define	CF_DEBUGS	0
#define	CF_SAFE		0
#define	CF_VOTEREPLACE	0		/* replace by voting among counters */
#define	CF_COUNTREPLACE	1		/* replace by counting all counters */


/* revision history:

	= 2002-05-01, David A­D­ Morano

	This object module was created for Levo research.
	It is a value predictor.  This is not coded as
	hardware.  It is like Atom analysis subroutines !


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object module implements a value predictor.  It is not
	coded as if it was real hardware (like LevoSim for example).
	It is coded like an analysis subroutine for Atom.


*****************************************************************************/


#define	VPRED_MASTER	0


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
#include	<localmisc.h>

#include	"vpred.h"


/* local defines */

#define	VPRED_MAGIC	0x23456787
#define	VPRED_DEFN	4		/* default entries */
#define	VPRED_EREPLACE	3		/* entry threshold (n-ops) */
#define	VPRED_OREPLACE	2		/* operand threshold (counter) */
#define	VPRED_SREPLACE	1		/* stride threshold (counter) */

#define	MODP2(v,n)	((v) & ((n) - 1))

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

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static uint	satcount(uint,uint,int) ;


/* exported subroutines */


int vpred_init(op,nentry,nops,sbits)
VPRED		*op ;
int		nentry ;
int		nops ;
int		sbits ;
{
	int	rs ;
	int	size ;
	int	n ;

	char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(VPRED)) ;

	if (nentry <= 0)
	    nentry = VPRED_DEFN ;

	op->tablen = nextpowtwo(nentry) ;

	size = op->tablen * sizeof(VPRED_ENT) ;
	rs = uc_malloc(size,&op->table) ;

	if (rs < 0)
	    goto bad0 ;

#ifdef	MALLOCLOG
	malloclog_alloc(op->table,rs,"vpred_init:table") ;
#endif

	(void) memset(op->table,0,sizeof(VPRED)) ;

/* calculate how much to shift the IA (right) */

	n = flbsi(op->tablen) ;

	op->tagshift = 2 + n ;

/* number of operands to predict */

	if (nops <= 0)
		nops = 1 ;

	if (nops > VPRED_NOPS)
		nops = VPRED_NOPS ;

	op->nops = nops ;

/* calculate the stride mask */

	op->stridemask = ((1u << sbits) - 1) ;

/* how many different counts (states) are there ? */

	op->ncount = (1U << VPRED_COUNTBITS) ;

/* load some initial statistic data */

	op->s.tablen = op->tablen ;

/* we're out of here */

	op->magic = VPRED_MAGIC ;

/* we're out of here */
bad1:

bad0:
	return rs ;
}
/* end subroutine (vpred_init) */


/* free up this vpred object */
int vpred_free(op)
VPRED		*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

	if (op->table != NULL) {

	    free(op->table) ;

#ifdef	MALLOCLOG
	malloclog_free(op->table,rs,"vpred_free:table") ;
#endif

	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (vpred_free) */


/* lookup an IA */
int vpred_lookup(op,ia,values,n)
VPRED		*op ;
uint		ia ;
uint		values[] ;
int		n ;
{
	uint	ti ;
	uint	tag ;

	int	rs = SR_NOENT ;
	int	size ;
	int	i, c ;
	int	mops ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

	if (values == NULL)
	    return SR_FAULT ;
#endif /* F_SAFE */

	ti = (ia >> 2) % op->tablen ;
	if (n > VPRED_NOPS)
		n = VPRED_NOPS ;

#if	CF_DEBUGS
	debugprintf("vpred_lookup: ti=%08x\n",ti) ;
#endif

	mops = MIN(n,op->nops) ;

	op->s.in_lu += 1 ;
	op->s.op_lu += mops ;

/* start searching ! */

	tag = ia >> op->tagshift ;
	if (tag != op->table[ti].tag)
	    goto miss ;

/* we got a hit */

	op->s.in_hit += 1 ;
	op->s.op_hit += mops ;
	for (i = 0 ; i < mops ; i += 1) {

	    values[i] = op->table[ti].ops[i].last +
	        op->table[ti].ops[i].stride ;

	} /* end for */

	for ( ; i < n ; i += 1)
		values[i] = 0 ;

	return mops ;

/* we missed (no allocation is done) */
miss:
	size = n * sizeof(uint) ;
	(void) memset(values,0,size) ;
	return SR_NOENT ;
}
/* end subroutine (vpred_lookup) */


/* update an entry */
int vpred_update(op,ia,values,n)
VPRED		*op ;
uint		ia ;
uint		values[] ;
{
	struct vpred_operand	*ops ;

	uint	ti ;
	uint	tag ;

	int	rs = SR_NOENT ;
	int	size ;
	int	i, c ;
	int	mops ;
	int	f_miss = FALSE ;
	int	f_same ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

	if (values == NULL)
	    return SR_FAULT ;
#endif /* F_SAFE */

	ti = (ia >> 2) % op->tablen ;
	ops = op->table[ti].ops ;
	if (n > VPRED_NOPS)
		n = VPRED_NOPS ;

	if (n <= 0)
		return SR_NOENT ;

	mops = MIN(n,op->nops) ;

#if	CF_DEBUGS
	debugprintf("vpred_update: ti=%08x\n",ti) ;
#endif

	op->s.in_up += 1 ;
	op->s.op_up += mops ;

/* do we have a hit ? */

	tag = ia >> op->tagshift ;
	if (tag != op->table[ti].tag) {

	    f_miss = TRUE ;

#if	CF_DEBUGS
	debugprintf("vpred_update: miss ia=%08x\n",ia) ;
#endif

/* miss, should we replace ? */

#if	CF_VOTEREPLACE
	c = 0 ;
	    for (i = 0 ; i < mops ; i += 1) {

	        if (ops[i].counter > VPRED_OREPLACE)
	            c += 1 ;

	    } /* end for */

	    if (c > VPRED_EREPLACE)
	        return SR_NOENT ;
#endif /* F_VOTEREPLACE */

#if	CF_COUNTREPLACE
	c = 0 ;
	for (i = 0 ; i < mops ; i += 1)
		c += ops[i].counter ;

#if	CF_DEBUGS
	debugprintf("vpred_update: replace n=%d mops=%d c=%d\n",n,mops,c) ;
#endif

	if (c > (mops * VPRED_EREPLACE))
		return SR_NOENT ;
#endif /* F_COUNTREPLACE */

/* replace ! */

#if	CF_DEBUGS
	debugprintf("vpred_update: replacing\n") ;
#endif

		op->table[ti].replaces += 1 ;

	    size = op->nops * sizeof(struct vpred_operand) ;
	    (void) memset(ops,0,size) ;

/* load our tag into the entry */

		op->table[ti].tag = tag ;

	op->s.in_replace += 1 ;
	op->s.op_replace += mops ;

	} /* end if (miss specific processing) */

/* update this entry (either new or old) */
update:
	if (! f_miss) {

		op->table[ti].hits += 1 ;
		op->s.in_update += 1 ;
		op->s.op_update += mops ;

	}

	for (i = 0 ; i < mops ; i += 1) {

	    if (f_miss || (ops[i].counter <= VPRED_SREPLACE))
	        ops[i].stride = (values[i] - ops[i].last) & op->stridemask ;

	    f_same = (ops[i].last == values[i]) ;
	    ops[i].last = values[i] ;

	    c = ops[i].counter ;
	    ops[i].counter = satcount(c,op->ncount,f_same) ;

	} /* end for */

	for ( ; i < op->nops ; i += 1) {

	    ops[i].last = 0 ;
	    ops[i].stride = 0 ;
	    ops[i].counter = 0 ;

	} /* end for */

	return n ;
}
/* end subroutine (vpred_update) */


/* get the entries (serially) */
int vpred_get(op,ri,rpp)
VPRED		*op ;
int		ri ;
VPRED_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("vpred_get: entered 0\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vpred_get: entered 1\n") ;
#endif

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vpred_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vpred_get: entered ri=%d\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->tablen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->table + ri ;

#if	CF_DEBUGS
	debugprintf("vpred_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (vpred_get) */


/* zero out the statistics */
int vpred_zerostats(op)
VPRED		*op ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

	(void) memset(&op->s,0,sizeof(VPRED_STATS)) ;

	return rs ;
}
/* end subroutine (vpred_zerostats) */


/* get information about all static branches */
int vpred_stats(op,rp)
VPRED		*op ;
VPRED_STATS	*rp ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != VPRED_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

	(void) memcpy(rp,&op->s,sizeof(VPRED_STATS)) ;

	return rs ;
}
/* end subroutine (vpred_stats) */



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



