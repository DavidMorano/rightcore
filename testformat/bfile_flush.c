/* bfile_flush */

/* "Basic I/O" package (BFILE) */
/* last modifed %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Internal (BFILE) flush subroutine.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#undef	COMMENT


/* external subroutines */


/* external variables */


/* exported subroutines */


int bfile_flushn(bfile *fp,int n)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (n != 0) {
	    int	f_sa = (! fp->f.notseek) && (fp->oflags & O_APPEND) ;

#if	CF_DEBUGS
	debugprintf("bfile_flush: f_sa=%u\n",f_sa) ;
#endif

	if (f_sa) {
	    offset_t	o ;
	    rs = u_seeko(fp->fd,0LL,SEEK_END,&o) ;
	    fp->offset = o ;

#if	CF_DEBUGS
	    debugprintf("bfile_flush: u_seeko() rs=%d\n",rs) ;
	    debugprintf("bfile_flush: file offset=%ld\n",fp->offset) ;
#endif

	} /* end if (sa) */

	if ((rs >= 0) && (fp->len > 0)) {
	    int	mlen = (fp->bp - fp->bbp) ;
	    if ((n > 0) && (n < mlen)) mlen = n ;
	    rs = uc_writen(fp->fd,fp->bbp,mlen) ;
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("bfile_flush: uc_writen() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        fp->bbp += len ;
	        fp->len -= len ;
	        if (fp->len == 0) {
		    fp->bbp = fp->bdata ;
		    fp->bp = fp->bdata ;
	        }
	        if (f_sa)
	            fp->offset += len ;
	    }

	} /* end if (flush needed) */

	} /* end if (not-zero) */

#if	CF_DEBUGS
	debugprintf("bfile_flush: ret rs=%d len=%u\n",
	    rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bfile_flushn) */


int bfile_flush(bfile *fp)
{

	return bfile_flushn(fp,-1) ;
}
/* end subroutine (bfile_flush) */


