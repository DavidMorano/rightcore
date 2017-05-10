/* bwrite */

/* routine to write bytes */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Unlike the standard I/O library, all writes of this library
	are atomic in that the entire portion of each write request is
	actually written to the file as a whole.  Each write block is
	either written to the file as a single block or in conjunction
	with previous write requests, but in no way will any single
	write request be broken up and written separately to the file.

	Note that this library can also freely intermix reads and writes
	to a file with the data ending up where it should without getting
	scrambled as in the standard library.

	Both of the above features, as well as some other features unique
	to this library, would normally make this package slower than
	the standard I/O library, but this package is normally faster
	than most versions of the standard package and probably close
	in performance with some of the latest implemtations which use
	some of the buffer management strategies used here.


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */

extern int	ifloor(int,int) ;

extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	bwrite_big(bfile *,const char *,int) ;
static int	bwrite_reg(bfile *,const char *,int) ;

static int	bfile_bufcpy(bfile *,const char *,int) ;


/* local variables */


/* exported subroutines */


int bwrite(fp,ubuf,ulen)
bfile		*fp ;
const void	*ubuf ;
int		ulen ;
{
	int	rs = SR_OK ;

	const char	*ubp = (const char *) ubuf ;


	if (fp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;
	if ((fp->oflags & O_ACCMODE) == O_RDONLY) return SR_RDONLY ;

	if (ulen < 0) ulen = strlen(ubp) ;

#if	CF_DEBUGS
	debugprintf("bwrite: ent ulen=%d\n",ulen) ;
	debugprinthex("bwrite: ",80,ubp,ulen) ;
#endif

	if (! fp->f.nullfile) {
	    int	f_bufnone = (fp->bm == bfile_bmnone) ; /* no buffering */
	        if (f_bufnone || (ulen >= fp->bsize)) {
	            rs = bwrite_big(fp,ubuf,ulen) ;
	        } else {
	            rs = bwrite_reg(fp,ubuf,ulen) ;
	        }
	}

#if	CF_DEBUGS
	debugprintf("bwrite: ret rs=%d ulen=%u\n",rs,ulen) ;
#endif

	return rs ;
}
/* end subroutine (bwrite) */


/* local subroutines */


static int bwrite_big(fp,ubuf,ulen)
bfile		*fp ;
const char	*ubuf ;
int		ulen ;
{
	int	rs ;


	if ((rs = bfile_flush(fp)) >= 0) {
	    int		lenr = ulen ;
	    int		len ;
	    const char	*ubp = (const char *) ubuf ;
	    while ((rs >= 0) && (lenr > 0)) {
	        if ((rs = uc_writen(fp->fd,ubp,lenr)) >= 0) {
	            len = rs ;
	            ubp += len ;
	            lenr -= len ;
	        }
	    } /* end while */
	    if (rs >= 0) fp->foff += ulen ;
	} /* end if (flush) */

	return (rs >= 0) ? ulen : rs ;
}
/* end subroutine (bwrite_big) */


static int bwrite_reg(fp,ubuf,ulen)
bfile		*fp ;
const char	*ubuf ;
int		ulen ;
{
	offset_t	uo = fp->foff ;
	const int	wopts = 0 ;
	int	rs = SR_OK ;
	int	lenr = ulen ;

	const char	*ubp = (const char *) ubuf ;


	while ((rs >= 0) && (lenr > 0)) {
	    rs = bfile_bufwrite(fp,uo,ubp,lenr,wopts) ;
	    ubp += rs ;
	    uo += rs ;
	    lenr -= rs ;
	} /* end while */

	if (rs >= 0) fp->foff = uo ;

	return (rs >= 0) ? ulen : rs ;
}
/* end subroutine (bwrite_reg) */


