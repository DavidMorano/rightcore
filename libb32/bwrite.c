/* bwrite */

/* routine to write bytes */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_CHUNKCPY	0		/* try chunk copy */
#define	CF_FLUSHPART	1		/* do partial flushes */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Unlike the standard I/O library, all writes of this library are atomic
        in that the entire portion of each write request is actually written to
        the file as a whole. Each write block is either written to the file as a
        single block or in conjunction with previous write requests, but in no
        way will any single write request be broken up and written separately to
        the file.

        Note that this library can also freely intermix reads and writes to a
        file with the data ending up where it should without getting scrambled
        as in the standard library.

        Both of the above features, as well as some other features unique to
        this library, would normally make this package slower than the standard
        I/O library, but this package is normally faster than most versions of
        the standard package and probably close in performance with some of the
        latest implemtations which use some of the buffer management strategies
        used here.


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

#define	MEMCPYLEN	100		/* minimum length for 'memcpy(3c)' */


/* external subroutines */

extern int	ifloor(int,int) ;

extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	bwrite_big(bfile *,const void *,int) ;
static int	bwrite_reg(bfile *,const void *,int) ;

static int	bfile_bufcpy(bfile *,const char *,int) ;


/* local variables */


/* exported subroutines */


int bwrite(fp,abuf,alen)
bfile		*fp ;
const void	*abuf ;
int		alen ;
{
	int		rs = SR_OK ;
	int		f_bufnone ;
	const char	*abp = (const char *) abuf ;

	if (fp == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if ((fp->oflags & O_ACCMODE) == 0) return SR_RDONLY ;

	if (alen < 0) alen = strlen(abp) ;

#if	CF_DEBUGS
	debugprintf("bwrite: ent len=%d\n",alen) ;
	debugprinthex("bwrite: ",80,abp,alen) ;
#endif

	if (fp->f.nullfile) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("bwrite: bufmode=%u\n",fp->bm) ;
#endif

#if	CF_DEBUGS && 0
	debugprintf("bwrite: off=%llu alen=%u\n",
	    fp->offset,alen) ;
#endif

	f_bufnone = (fp->bm == bfile_bmnone) ;

/* if we were not previously writing, seek to the proper place */

	if (! fp->f.write) {

	    if ((! fp->f.notseek) &&
	        (! (fp->oflags & O_APPEND))) {

	        rs = u_seek(fp->fd,fp->offset,SEEK_SET) ;

	    }

	    fp->len = 0 ;
	    fp->bp = fp->bdata ;
	    fp->f.write = TRUE ;

	} /* end if (previously reading) */

/* check for a large 'write' */

	if (rs >= 0) {
	    if (f_bufnone) {
	        rs = bwrite_big(fp,abuf,alen) ;
	    } else {
	        rs = bwrite_reg(fp,abuf,alen) ;
	    }
	}

ret0:

#if	CF_DEBUGS
	debugprintf("bwrite: ret rs=%d alen=%u\n",rs,alen) ;
#endif

	return rs ;
}
/* end subroutine (bwrite) */


/* local subroutines */


static int bwrite_big(fp,abuf,alen)
bfile		*fp ;
const void	*abuf ;
int		alen ;
{
	int		rs = SR_OK ;
	int		alenr = alen ;
	int		len ;
	const char	*abp = (const char *) abuf ;

	if (fp->len > 0)
	    rs = bfile_flush(fp) ;

	while ((rs >= 0) && (alenr > 0)) {

	    rs = uc_writen(fp->fd,abp,alenr) ;
	    len = rs ;

	    if (rs >= 0) {
	        fp->offset += len ;
	        abp += len ;
	        alenr -= len ;
	    }

	} /* end while */

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (bwrite_big) */


static int bwrite_reg(fp,abuf,alen)
bfile		*fp ;
const void	*abuf ;
int		alen ;
{
	int		rs = SR_OK ;
	int		alenr = alen ;
	int		mlen ;
	int		len ;
	int		f_bufline = (fp->bm == bfile_bmline) ;
	const char	*abp = (const char *) abuf ;

	while ((rs >= 0) && (alenr > 0)) {

#if	CF_CHUNKCPY
	    if ((rs >= 0) && (fp->len == 0) && (alenr >= fp->bsize)) {

		while ((rs >= 0) && (alenr >= fp->bsize)) {
	            mlen = fp->bsize ;
		    rs = bwrite_big(fp,abuf,mlen)
		    alenr -= mlen ;
		}

	    } /* end if */
#endif /* CF_CHUNKCPY */

	    if ((rs >= 0) && (alenr > 0)) {
	        int	n = 0 ;
	        int	blenr = (fp->bdata + fp->bsize - fp->bp) ;

	        mlen = MIN(alenr,blenr) ;
	        if (f_bufline && (mlen > 0)) {
	            const char	*tp ;
#if	CF_DEBUGS
	            debugprintf("bwrite: lb mlen=%u\n",mlen) ;
#endif
	            if ((tp = strnrchr(abp,mlen,'\n')) != NULL) {
	                n = (fp->len + ((tp+1) - abp)) ;
	            }
	        }

	        len = bfile_bufcpy(fp,abp,mlen) ;
	        abp += len ;
	        alenr -= len ;

#if	CF_FLUSHPART
	        if (fp->bp == (fp->bdata + fp->bsize)) {
	            rs = bfile_flush(fp) ;
	        } else if (f_bufline && (n > 0)) {
#if	CF_DEBUGS
	            debugprintf("bwrite: bfile_flushn() n=%u\n",n) ;
#endif
	            rs = bfile_flushn(fp,n) ;
	        }
#else /* CF_FLUSHPART */
	        if (fp->bp == (fp->bdata + fp->bsize))
	            rs = bfile_flush(fp) ;
#endif /* CF_FLUSHPART */

	    } /* end if */

	} /* end while */

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (bwrite_reg) */


static int bfile_bufcpy(fp,abp,mlen)
bfile		*fp ;
const char	*abp ;
int		mlen ;
{

	if (mlen > MEMCPYLEN) {
	    memcpy(fp->bp,abp,mlen) ;
	} else {
	    register int	i ;
	    register char	*bp = fp->bp ;
	    for (i = 0 ; i < mlen ; i += 1)
	        *bp++ = *abp++ ;
	}

	fp->bp += mlen ;
	fp->len += mlen ;
	fp->offset += mlen ;
	return mlen ;
}
/* end subroutine (bfile_bufcpy) */


