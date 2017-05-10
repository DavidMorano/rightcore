/* bgetline */

/* "Basic I/O" package similiar to "stdio" */
/* last modifed %G% version %I% */


#define	F_DEBUGS	0
#define	F_MEMCCPY	0		/* we are faster than 'memccpy()' ! */
#define	F_LARGEFILE	1


/* revision history :

	= 86/07/01, David A­D­ Morano

	This subroutine was originally written.


	= 99/01/10, David A­D­ Morano

	I added the little extra code to allow for memory mapped I/O.
	It is all a waste because it is way slower than without it !
	This should teach me to leave old programs alone !!!


*/


/******************************************************************************

	This subroutine reads a single line from the buffer (or where
	ever) and returns that line to the caller.  Any remaining data
	is left for subsequent reads (of any kind).



******************************************************************************/


#define	BIO_MASTER	1


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>

#include	"misc.h"
#include	"bio.h"



/* local defines */

#define	F_BIOLARGEFILE	\
		(_LFS64_LARGEFILE && F_LARGEFILE && defined(O_LARGEFILE))



/* external subroutines */

extern int	bio_pagein() ;


/* external variables */





int bgetline(fp,ubuf,ulen)
bfile	*fp ;
char	ubuf[] ;
int	ulen ;
{
	int	rs, i, mlen, tlen ;
	int	f_statted, f_already ;

	register char	*dbp  = ubuf ;


#if	F_DEBUGS
	eprintf("bgetline: entered\n") ;
#endif

	if (fp == NULL) 
		return SR_FAULT ;

#if	F_DEBUGS
	eprintf("bgetline: entered 2\n") ;
#endif

	if (fp->magic != BIO_MAGIC) 
		return SR_NOTOPEN ;

#if	F_DEBUGS
	eprintf("bgetline: entered 3\n") ;
#endif

	if (fp->oflag & O_WRONLY)
		return SR_WRONLY ;

#if	F_DEBUGS
	eprintf("bgetline: continuing\n") ;
#endif

	if (fp->stat & BIOSM_WRITE) {

#if	F_DEBUGS
	    eprintf("bgetline: previously writing\n") ;
#endif

	    if ((fp->len > 0) && ((rs = bio_flush(fp)) < 0)) {

#if	F_DEBUGS
	    eprintf("bgetline: bio_flush() rs=%d\n",rs) ;
#endif

	        return rs ;
		}

	    fp->stat &= (~ BIOSM_WRITE) ;

	} /* end if (switching from writing to reading) */

/* decide how we want to transfer the data */

	tlen = 0 ;
	if (fp->stat & BIOSM_MAPINIT) {

#if	F_BIOLARGEFILE
	    struct stat64	sb ;
#else
	    struct ustat		sb ;
#endif

	    int	pagemask = fp->pagesize - 1 ;


#if	F_DEBUGS
	    eprintf("bgetline: mapped\n") ;
#endif

	    f_already = FALSE ;
	    while (tlen < ulen) {

	        offset_t	baseoff ;


#if	F_DEBUGS
	        eprintf("bgetline: ulen=%d tlen=%d\n",ulen,tlen) ;
#endif

/* is there more data in the file and are we at a map page boundary ? */

	        mlen = fp->size - fp->offset ;

#if	F_DEBUGS
	        eprintf("bgetline: more mlen=%d\n",mlen) ;
#endif

	        if ((mlen > 0) &&
	            ((fp->bp == NULL) || (fp->len == fp->pagesize))) {

#if	F_DEBUGS
	            eprintf("bgetline: need to check on page mapping\n") ;
#endif

	            i = (fp->offset / fp->pagesize) & (BIO_NMAPS - 1) ;
	            baseoff = fp->offset & (~ pagemask) ;
	            if ((! fp->maps[i].f.valid) || (fp->maps[i].buf == NULL)
	                || (fp->maps[i].offset != baseoff)) {

#if	F_DEBUGS
	                eprintf("bgetline: mapping a page, offset=%ld\n",
	                    fp->offset) ;
#endif

	                bio_pagein(fp,fp->offset,i) ;

	            }

	            fp->len = fp->offset & pagemask ;
	            fp->bp = fp->maps[i].buf + fp->len ;

	        } /* end if (initializing memory mapping) */

/* prepare to move data */

#if	F_DEBUGS
	        eprintf("bgetline: preparing to move data\n") ;
#endif

	        if ((fp->pagesize - fp->len) < mlen)
	            mlen = (fp->pagesize - fp->len) ;

	        if ((ulen - tlen) < mlen)
	            mlen = (ulen - tlen) ;

	        if (mlen > 0) {

	            char	*bp ;
	            char	*lastp ;


#if	F_DEBUGS
	            eprintf("bgetline: moving data\n") ;
#endif

#if	F_MEMCCPY
	            if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL)
	                lastp = dbp + mlen ;

	            i = lastp - dbp ;
	            dbp += i ;
	            fp->bp += i ;
#else
	            bp = fp->bp ;
	            lastp = fp->bp + mlen ;
	            while (bp < lastp) {

	                if ((*dbp++ = *bp++) == '\n') 
				break ;

			}

	            i = bp - fp->bp ;
	            fp->bp += i ;
#endif /* F_MEMCCPY */

	            fp->len += i ;
	            fp->offset += i ;
	            tlen += i ;
	            if ((i > 0) && (dbp[-1] == '\n'))
	                return tlen ;

	        } /* end if (move it) */

/* if we were file size limited */

	        if (fp->offset >= fp->size) {

#if	F_DEBUGS
	            eprintf("bgetline: offset at end\n") ;
#endif

	            if (f_already) 
			break ;

#if	F_DEBUGS
	            eprintf("bgetline: re-statting\n") ;
#endif

#if	F_BIOLARGEFILE
	            rs = u_fstat64(fp->fd,&sb) ;
#else
	            rs = u_fstat(fp->fd,&sb) ;
#endif

#if	F_DEBUGS
		if (rs < 0)
	        eprintf("bgetline: u_fstat() rs=%d\n",rs) ;
#endif

			if (rs < 0)
	                return tlen ;

	            fp->size = sb.st_size ;
	            f_already = TRUE ;

	        } /* end if (file size limited) */

#if	F_DEBUGS
	        eprintf("bgetline: bottom loop\n") ;
#endif

	    } /* end while (reading) */

	} else {

#if	F_DEBUGS
	    eprintf("bgetline: normal unmapped\n") ;
#endif

	    f_already = FALSE ;
	    tlen = 0 ;
	    dbp = ubuf ;
	    while (ulen > 0) {

	        if (fp->len <= 0) {

	            if (f_already && (! (fp->stat & BIOSM_LINEIN)))
	                break ;

	            fp->len = u_read(fp->fd,fp->buf,fp->bufsize) ;

#if	F_DEBUGS
		if (fp->len < 0)
	    eprintf("bgetline: rs=%d\n",fp->len) ;
#endif

			if (fp->len < 0)
	                return fp->len ;

	            if (fp->len < fp->bufsize)
	                f_already = TRUE ;

	            if (fp->len == 0) 
			break ;

	            fp->bp = fp->buf ;

	        } /* end if (refilling up buffer) */

	        mlen = (fp->len < ulen) ? fp->len : ulen ;

#if	F_DEBUGS
	    eprintf("bgetline: mlen=%d\n",mlen) ;
#endif

	        if (mlen > 0) {

	            register char	*bp ;
	            register char	*lastp ;


#if	F_MEMCCPY
	            if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL)
	                lastp = dbp + mlen ;

	            i = lastp - dbp ;
	            dbp += i ;
	            fp->bp += i ;
#else
	            bp = fp->bp ;
	            lastp = fp->bp + mlen ;
	            while (bp < lastp) {

	                if ((*dbp++ = *bp++) == '\n') 
				break ;

			}

	            i = bp - fp->bp ;
	            fp->bp += i ;
#endif /* F_MEMCCPY */

#if	F_DEBUGS
	    eprintf("bgetline: i=%d\n",i) ;
#endif

	            fp->len -= i ;
	            fp->offset += i ;
	            tlen += i ;
	            if ((i > 0) && (dbp[-1] == '\n'))
	                return tlen ;

	            ulen -= mlen ;

	        } /* end if (move it) */

#if	F_DEBUGS
	    eprintf("bgetline: bottom while\n") ;
#endif

	    } /* end while (trying to satisfy request) */

	    fp->offset += tlen ;

	} /* end if (read map or not) */

#if	F_DEBUGS
	    eprintf("bgetline: bottom exit rs=%d\n",tlen) ;
#endif

	return tlen ;
}
/* end subroutine (bgetline) */



