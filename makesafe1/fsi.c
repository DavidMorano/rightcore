/* fsi */

/* FIFO-String-Interlocked management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages interlocked FIFO-string operations.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<fifostr.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"fsi.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int fsi_start(FSI *op)
{
	int		rs ;

	if ((rs = ptm_create(&op->m,NULL)) >= 0) {
	    rs = fifostr_start(&op->q) ;
	    if (rs < 0)
	        ptm_destroy(&op->m) ;
	}

	return rs ;
}
/* end subroutine (fsi_start) */


int fsi_finish(FSI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = fifostr_finish(&op->q) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (fsi_finish) */


int fsi_add(FSI *op,const char *sbuf,int slen)
{
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = fifostr_add(&op->q,sbuf,slen) ;
		rv = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (fsi_add) */


int fsi_remove(FSI *op,char *sbuf,int slen)
{
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = fifostr_remove(&op->q,sbuf,slen) ;
	        rl = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (fsi_remove) */


int fsi_rem(FSI *op,char *sbuf,int slen)
{
	return fsi_remove(op,sbuf,slen) ;
}
/* end subroutine (fsi_rem) */


/* mutex locking is not really necessary here, but we do it anyway! */
int fsi_count(FSI *op)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = ptm_lock(&op->m)) >= 0) {
	    {
	        rs = fifostr_count(&op->q) ;
	        c = rs ;
	    }
	    rs1 = ptm_unlock(&op->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (fsi_count) */


