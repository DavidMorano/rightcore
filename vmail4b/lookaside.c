/* lookaside */

/* vector list operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSGET	1		/* debug 'lookaside_get()' */
#define	CF_DEBUGSCHUNK	1
#define	CF_DEBUGSFREE	1
#define	CF_DEBUGSAUDIT	1		/* debugging w/ audits */
#define	CF_SAFE		0		/* safe mode */
#define	CF_PARSEL	1		/* parsel out pieces (faster?) */

#ifndef	LOOKASIDE_LIBMEMALLOC
#define	LOOKASIDE_LIBMEMALLOC	0	/* default is |uc_memalloc(3uc)| */
#endif


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides a fix-sized pool of memory blocks for fast
        allocation and deallocation. However, memory is never released back to
        'malloc(3c)' (the origin from which all memory comes) once it is
        allocated. Freed blocks are, however, available for new allocation
        requests.


*******************************************************************************/


#define	LOOKASIDE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<pq.h>
#include	<localmisc.h>

#include	"lookaside.h"


/* local defines */

#define	QALIGN			(2 * sizeof(void *))

#if	LOOKASIDE_LIBMEMALLOC
#define	OURMALLOC(size,pointer)		uc_libmalloc((size),(pointer))
#define	OURREALLOC(p1,size,p2)		uc_librealloc((p1),(size),(p2))
#define	OURFREE(pointer)		uc_libfree((pointer))
#else /* LOOKASIDE_LIBMEMALLOC */
#define	OURMALLOC(size,pointer)		uc_malloc((size),(pointer))
#define	OURREALLOC(p1,size,p2)		uc_realloc((p1),(size),(p2))
#define	OURFREE(pointer)		uc_free((pointer))
#endif /* LOOKASIDE_LIBMEMALLOC */

#define	LOOKASIDE_CHUNK		struct lookaside_chunk


/* external subroutines */

extern uint	uceil(uint,int) ;


/* external variables */


/* local structures */

struct lookaside_chunk {
	PQ_ENT		dummy ;
} ;


/* forward references */

static int	lookaside_newchunk(LOOKASIDE *) ;


/* local variables */


/* exported subroutines */


int lookaside_start(LOOKASIDE *op,int esize,int n)
{
	int		rs ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("lookaside_start: ent esize=%d n=%d\n",esize,n) ;
#endif

	if (esize <= 0) return SR_INVALID ;

	if (esize < QALIGN)
	    esize = QALIGN ;

	if (n < 0)
	    n = LOOKASIDE_MINENTRIES ;

#if	CF_DEBUGS
	debugprintf("lookaside_start: adjusted n=%d\n",n) ;
#endif

	memset(op,0,sizeof(LOOKASIDE)) ;
	op->esize = esize ;
	op->n = n ;
	op->i = -1 ;

/* calculate the entry-array offset */

	size = sizeof(struct lookaside_chunk) ;
	op->eaoff = uceil(size,QALIGN) ;

#if	CF_DEBUGS
	debugprintf("lookaside_start: eaoff=%u\n",op->eaoff) ;
#endif

	if ((rs = pq_start(&op->cq)) >= 0) {
	    rs = pq_start(&op->estack) ;
	    if (rs < 0)
		pq_finish(&op->cq) ;
	} /* end if (pq_start) */

	return rs ;
}
/* end subroutine (lookaside_start) */


int lookaside_finish(LOOKASIDE *op)
{
	LOOKASIDE_CHUNK	*cp ;
	PQ		*cqp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	op->eap = NULL ;

/* for entries, we only need to free up chunks, they were all in chunks! */

	cqp = &op->cq ; /* loop invariant */
	while (pq_rem(cqp,(PQ_ENT **) &cp) >= 0) {
	    c += 1 ;
	    op->nchunks -= 1 ;
	    rs1 = OURFREE(cp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end while */

/* free up any other structures */

	rs1 = pq_finish(&op->estack) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pq_finish(&op->cq) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS && CF_DEBUGSFREE
	debugprintf("lookaside_finish: ret rs=%d c=%u nchunks=%u\n",
	    rs,c,op->nchunks) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (lookaside_finish) */


/* get a new entry */
#if	CF_PARSEL
int lookaside_get(LOOKASIDE *op,void *p)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (p == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (op->nfree > 0) {
	    PQ_ENT	**epp = (PQ_ENT **) p ;
	    if ((rs = pq_remtail(&op->estack,epp)) >= 0) {
	        op->nfree -= 1 ;
	    }
	} else {
	    caddr_t	*pp = (caddr_t *) p ;
	    if ((op->i < 0) || (op->i >= op->n)) {
	        rs = lookaside_newchunk(op) ;
	    }
	    if (rs >= 0) {
	        *pp = (caddr_t) (((long) op->eap) + (op->i * op->esize)) ;
	        op->i += 1 ;
	    }
	} /* end if */

	op->nused += 1 ;
	return rs ;
}
/* end subroutine (lookaside_get) */
#else /* CF_PARSEL */
/* get a new entry */
int lookaside_get(LOOKASIDE *op,void *p)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (p == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	{
	    PQ_ENT	**epp = (PQ_ENT **) p ;
	    if (op->nfree == 0) {
	        rs = lookaside_newchunk(op) ;
	    } /* end if (getting more) */
	    if (rs >= 0) {
	        if ((rs = pq_remtail(&op->estack,epp)) >= 0) {
	            op->nfree -= 1 ;
	        }
	    }
	}

	op->nused += 1 ;
	return rs ;
}
/* end subroutine (lookaside_get) */
#endif /* CF_PARSEL */


/* release (return) an entry to pool */
int lookaside_release(LOOKASIDE *op,void *p)
{
	PQ_ENT		*ep = (PQ_ENT *) p ;
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if ((rs = pq_ins(&op->estack,ep)) >= 0) {
	    op->nfree += 1 ;
	    op->nused -= 1 ;
	}

	return rs ;
}
/* end subroutine (lookaside_release) */


/* return the count of the number of items currently in use */
int lookaside_count(LOOKASIDE *op)
{

	if (op == NULL) return SR_FAULT ;

	return op->nused ;
}
/* end subroutine (lookaside_count) */


int lookaside_audit(LOOKASIDE *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = pq_audit(&op->cq) ;

	if (rs >= 0)
	    rs = pq_audit(&op->estack) ;

	return rs ;
}
/* end subroutine (lookaside_audit) */


/* private subroutines */


#if	CF_PARSEL
static int lookaside_newchunk(LOOKASIDE *op)
{
	int		rs ;
	int		size ;
	caddr_t		a ;

	size = op->eaoff + (op->n * op->esize) ;
	if ((rs = OURMALLOC(size,&a)) >= 0) {
	    if ((rs = pq_ins(&op->cq,(PQ_ENT *) a)) >= 0) {
	        op->eap = (caddr_t) (a + op->eaoff) ;
	        op->i = 0 ;
	        op->nchunks += 1 ;
	    } /* end if (pq_ins) */
	    if (rs < 0)
		OURFREE(a) ;
	} /* end if (m-a) */

#if	CF_DEBUGS && CF_DEBUGSCHUNK
	debugprintf("lookaside_newchunk: ret rs=%d added nchunks=%u\n",
	    rs,op->nchunks) ;
#endif

	return rs ;
}
/* end subroutine (lookaside_newchunk) */
#else /* CF_PARSEL */
static int lookaside_newchunk(LOOKASIDE *op)
{
	int		rs ;
	int		size ;
	caddr_t		a ;

	size = op->eaoff + (op->n * op->esize) ;
	if ((rs = OURMALLOC(size,&a)) >= 0) {
	    if ((rs = pq_ins(&op->cq,(PQ_ENT *) a)) >= 0) {
	        PQ_ENT	*eap ;
	        op->eap = (caddr_t) (a + op->eaoff) ;
	        eap = (PW_ENT *) op->eap,
	        if ((rs = pq_insgroup(&op->estack,eap,op->esize,op->n)) >= 0) {
	            op->nfree += op->n ;
	            op->nchunks += 1 ;
	        }
	        if (rs < 0) {
	    	    PQ_ENT	*dummy ;
	    	    pq_remtail(&op->cq,&dummy) ;
	        }
	    } /* end if (pq_ins) */
	    if (rs < 0)
		OURFREE(a) ;
	} /* end if (m-a) */

#if	CF_DEBUGS && CF_DEBUGSCHUNK
	debugprintf("lookaside_newchunk: ret rs=%d added nchunks=%u\n",
	    rs,op->nchunks) ;
#endif

	return rs ;
}
/* end subroutine (lookaside_newchunk) */
#endif /* CF_PARSEL */


