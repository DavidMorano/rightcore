/* bdup */

/* "Basic I/O" package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-01-10, David A­D­ Morano
        I added the little extra code to allow for memory mapped I/O. It is all
        a waste because it is way slower than without it! This should teach me
        to leave old programs alone!!!

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Duplicate a BFILE stream.


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bdup(bfile *fp,bfile *fnewp)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("bdup: ent\n") ;
#endif

	if (fp == NULL) return SR_FAULT ;
	if (fnewp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	memcpy(fnewp,fp,sizeof(bfile)) ;

	if ((rs = bfile_flush(fp)) >= 0) {
	    if ((rs = u_dup(fp->fd)) >= 0) {
	        fnewp->fd = rs ;
	        if (fp->bsize > 0) {
		    char	*p ;
	            if ((rs = uc_malloc(fp->bsize,&p)) >= 0) {
	                fnewp->bdata = p ;
	                fnewp->bbp = p ;
	                fnewp->bp = p ;
	            }
	        }
		if (rs < 0) {
		    u_close(fnewp->fd) ;
		    fnewp->fd = -1 ;
		}
	    } /* end if (u_dup) */
	    if (rs < 0) {
		fnewp->magic = 0 ;
	    }
	} /* end if (bfile_flush) */

#if	CF_DEBUGS
	debugprintf("bdup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdup) */


