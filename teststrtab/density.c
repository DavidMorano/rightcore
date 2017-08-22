/* density */

/* this is a density gathering object */


/* revision history:

	= 2002-02-16, David A­D­ Morano
        This was written for some statistics gathering for some software
        evaluation.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object facilitates maintaining the probability density for some
        random variable.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"density.h"


/* local defines */


/* external subroutines */

extern int	densitystatll(ULONG *,int,double *,double *) ;


/* exported subroutines */


int density_start(DENSITY *op,int len)
{
	int		rs ;
	int		size ;
	void		*p ;

	if (len < 1)
	    return SR_INVALID ;

	memset(op,0,sizeof(DENSITY)) ;

	size = (len + 1) * sizeof(ULONG) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    op->a = p ;
	    memset(op->a,0,size) ;
	    op->len = len ;
	    op->magic = DENSITY_MAGIC ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (density_start) */


int density_finish(DENSITY *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DENSITY_MAGIC)
	    return SR_NOTOPEN ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (density_finish) */


int density_update(DENSITY *op,int ai)
{
	int		rs = SR_OK ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DENSITY_MAGIC)
	    return SR_NOTOPEN ;

	if (ai > op->max)
	    op->max = ai ;

	if (ai > (op->len - 1)) {
	    rs = SR_RANGE ;
	    ai = (op->len - 1) ;
	    op->ovf += 1 ;
	} /* end if */

	op->c += 1 ;
	op->a[ai] += 1 ;

	return rs ;
}
/* end subroutine (density_update) */


int density_slot(DENSITY *op,int ai,ULONG *rp)
{

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != DENSITY_MAGIC) return SR_NOTOPEN ;

	if (ai > (op->len - 1)) return SR_INVALID ;

	*rp = op->a[ai] ;
	return SR_OK ;
}
/* end subroutine (density_slot) */


int density_stats(DENSITY *op,DENSITY_STATS *sp)
{
	int		rs ;
	int		len ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != DENSITY_MAGIC) return SR_NOTOPEN ;

	sp->mean = 0.0 ;
	sp->var = 0.0 ;
	sp->count = op->c ;
	sp->max = op->max ;
	sp->ovf = op->ovf ;
	sp->len = op->len ;
	len = (int) op->len ;
	if ((rs = densitystatll(op->a,len,&sp->mean,&sp->var)) >= 0) {
	    ULONG	sum = 0 ;
	    int		i ;
	    for (i = 0 ; i < op->len ; i += 1) {
	        sum += op->a[i] ;
	    }
	    if (sum != op->c) rs = SR_BADFMT ;
	} /* end if */

	return rs ;
}
/* end subroutine (density_stats) */


