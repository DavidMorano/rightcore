/* bfile */

/* utility subroutines for BFILE */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These are some general utility subroutines for BFILE, mainly to
        maintenance portability consideration.


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


/* external subroutines */


/* external variables */


/* local structures */


/* local variables */


/* exported variables */

const char	*bfile_fnames[] = {
	"*STDIN*",
	"*STDOUT*",
	"*STDERR*",
	"*STDNULL*",
	NULL
} ;


/* exported subroutines */


int bfile_flushn(fp,n)
bfile		*fp ;
int		n ;
{
	int		rs = SR_OK ;
	int		len = 0 ;
	int		f_sa ;

	if (n == 0) goto ret0 ;

	f_sa = (! fp->f.notseek) && (fp->oflags & O_APPEND) ;

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

ret0:

#if	CF_DEBUGS
	debugprintf("bfile_flush: ret rs=%d len=%u\n",
	    rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bfile_flushn) */


int bfile_flush(fp)
bfile		*fp ;
{

	return bfile_flushn(fp,-1) ;
}
/* end subroutine (bfile_flush) */


int bfilestat(fname,type,sbp)
const char	fname[] ;
int		type ;
BFILE_STAT	*sbp ;
{
	int		rs ;

	if (fname == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (type > 0) {
	    rs = uc_lstat(fname,sbp) ;
	} else
	    rs = uc_stat(fname,sbp) ;

	return rs ;
}
/* end subroutine (bfilestat) */


int bfilefstat(fd,sbp)
int		fd ;
BFILE_STAT	*sbp ;
{
	int		rs ;

	if (sbp == NULL) return SR_FAULT ;

	if (fd < 0) return SR_BADF ;

	rs = u_fstat(fd,sbp) ;

	return rs ;
}
/* end subroutine (bfilefstat) */


