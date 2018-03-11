/* bits */

/* perform some bit-array type operations */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		1		/* safe mode? */


/* revistion history:

	= 1998-05-27, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module does some bit-array type [of] stuff.


*******************************************************************************/


#define	BITS_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<findbit.h>
#include	<localmisc.h>

#include	"bits.h"


/* local defines */

#ifndef	NBBY
#ifdef	CHAR_BIT	/* from limits */
#define	NBBY		CHAR_BIT
#else /* CHAR_BIT */
#define	NBBY		8
#endif /* CHAR_BIT */
#endif

#define	BITS_BPW	(NBBY * sizeof(ULONG))
#define	BITS_NMINWORDS	1


/* external subroutines */

extern int	iceil(int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	bits_alloc(BITS *,int) ;


/* local variables */


/* exported subroutines */


int bits_start(BITS *op,int n)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (n < 0) return SR_INVALID ;

	op->a = NULL ;
	op->nwords = 0 ;
	op->nbits = 0 ;
	op->n = 0 ;
	if (n > 0) {
	    rs = bits_alloc(op,n) ;
	}

	return rs ;
}
/* end subroutine (bits_start) */


int bits_finish(BITS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_OK ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	op->nwords = 0 ;
	op->nbits = 0 ;
	return rs ;
}
/* end subroutine (bits_finish) */


int bits_set(BITS *op,int i)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (i < 0) return SR_INVALID ;

	if (i >= op->nbits) {
	    rs = bits_alloc(op,i+1) ;
	}

	if (rs >= 0) {
	    if ((i+1) > op->n) op->n = (i+1) ;
	    rs = BATSTL(op->a,i) ;
	    BASETL(op->a,i) ;
	}

	return rs ;
}
/* end subroutine (bits_set) */


int bits_clear(BITS *op,int i)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (i >= op->nbits) {
	    rs = bits_alloc(op,i+1) ;
	}

	if (rs >= 0) {
	    if (i >= 0) {
	        if ((i+1) > op->n) op->n = (i+1) ;
	        rs = BATSTL(op->a,i) ;
	        BACLRL(op->a,i) ;
	    } else if (op->a != NULL) {
		memset(op->a,0,op->nwords) ;
	    }
	}

	return rs ;
}
/* end subroutine (bits_clear) */


int bits_test(BITS *op,int i)
{
	int		rs = SR_OK ; /* default return is "FALSE" */

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	if (i < 0) return SR_INVALID ;
	if (i < op->nbits) {
	    rs = BATSTL(op->a,i) ;
	}

	return rs ;
}
/* end subroutine (bits_test) */


int bits_anyset(BITS *op)
{
	int		rs = SR_OK ; /* default return is "FALSE" */
	int		w ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	for (w = 0 ; w < op->nwords ; w += 1) {
	   f = (op->a[w] != 0) ;
	   if (f) break ;
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bits_anyset) */


int bits_ffbs(BITS *op)
{
	int		rs = SR_OK ; /* default return is "FALSE" */
	int		w ;
	int		i = -1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	for (w = 0 ; w < op->nwords ; w += 1) {
	   if ((i = ffbsll(op->a[w])) >= 0) break ;
	} /* end for */

	if (i >= 0) {
	    i += (w*BITS_BPW) ;
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (bits_ffbs) */


int bits_extent(BITS *op)
{
#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif
	return op->n ;
}
/* end subroutine (bits_extent) */


int bits_count(BITS *op)
{
	int		i ;
	int		sum = 0 ;
#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif
	for (i = 0 ; i < op->n ; i += 1) {
	    if (BATSTL(op->a,i)) sum += 1 ;
	}
	return sum ;
}
/* end subroutine (bits_count) */


/* private subroutines */


static int bits_alloc(BITS *op,int nn)
{
	int		rs = SR_OK ;
	int		nnwords = ((nn +(BITS_BPW-1))/BITS_BPW) ;

#if	CF_DEBUGS
	debugprintf("bits_alloc: nw=%d nnw=%d\n",op->nwords,nnwords) ;
#endif

	if (nnwords > op->nwords) {
	    const int	nminwords = (op->nwords+BITS_NMINWORDS) ;
	    int		size ;
	    char	*na ;

	    if (nnwords < nminwords) nnwords = nminwords ;

#if	CF_DEBUGS
	debugprintf("bits_alloc: nnw=%d\n",nnwords) ;
#endif

	    size = nnwords * sizeof(ULONG) ;

#if	CF_DEBUGS
	debugprintf("bits_alloc: a=%p size=%d\n",op->a,size) ;
#endif

	    if (op->a == NULL) {
	        rs = uc_malloc(size,&na) ;
	        if (rs >= 0) memset(na,0,size) ;
	    } else {
		const int	i = (op->nwords*sizeof(ULONG)) ;
		int	newsize ;
		newsize = (size-i) ;
	        rs = uc_realloc(op->a,size,&na) ;
	        if (rs >= 0) memset((na+i),0,newsize) ;
	    }

	    if (rs >= 0) {
	        op->a = (ULONG *) na ;
	        op->nwords = nnwords ;
	        op->nbits = (nnwords * BITS_BPW) ;
	    }

	} /* end if (allocation or reallocation needed) */

#if	CF_DEBUGS
	debugprintf("bits_alloc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bits_alloc) */


