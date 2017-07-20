/* uc_writedesc */

/* interface component for UNIX® library-3c */
/* copy from one file descriptor to another */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_VALLOC	0		/* only use 'valloc(3c)' */


/* revision history:

	= 1998-01-10, David A­D­ Morano
	The subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine copies data from one file descriptor to another (source
        to destination) for the length specified as an argument.

        The idea of having a separate subroutine for this sort of (usually)
        trivial function, is that we can perform optimizations that a typical
        user would find tiresome for such a "simple" function.

	Synopsis:

	int uc_copy(sfd,dfd,len)
	int	sfd ;
	int	dfd ;
	int	len ;

	Arguments:

	sfd		source file descriptor
	dfd		destination file descriptor
	len		length to copy

	Returns:

	<0		error
	>=0		length copied


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	PIPEBUFLEN	2048

#if	CF_VALLOC
#define	MAXBUFLEN	(16*1024*1024)
#else
#define	MAXBUFLEN	(2*1024*1024)
#endif

#ifndef	BCEIL
#define	BCEIL(v,m)	(((v) + ((m) - 1)) & (~ ((m) - 1)))
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
static int	deubgprintstat(cchar *,int) ;
#endif


/* exported subroutines */


int uc_copy(int sfd,int dfd,int ulen)
{
	int		rs = SR_OK ;
	int		tlen = 0 ;

	if (sfd == dfd) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_copy: ent sfd=%u dfd=%u ulen=%d\n",sfd,dfd,ulen) ;
#endif

	if (ulen != 0) {
	    size_t	fsize = SIZE_MAX ;
	    const int	ps = getpagesize() ;
	    int		readlen ;

/* calculate the size of a buffer to allocate */

	    if ((ulen < 0) || (ulen > ps)) {
	        if (ulen >= 0) {
	            readlen = BCEIL(ulen,ps) ;
	        } else {
	            USTAT	sb ;
	            int		bsize ;
	            if ((rs = u_fstat(sfd,&sb)) >= 0) {
	                const mode_t	m = sb.st_mode ;
	                if (S_ISREG(m) || S_ISBLK(m)) {
	                    fsize = (size_t) (sb.st_size & SIZE_MAX) ;
	                    bsize = (MAX(sb.st_size,1) & INT_MAX) ;
	                    readlen = BCEIL(bsize,ps) ;
	                } else if (S_ISFIFO(m)) {
	                    readlen = PIPEBUFLEN ;
	                } else {
	                    readlen = ps ;
	                }
	            } /* end if (u_fstat) */
	        } /* end if */
	        if (readlen > MAXBUFLEN) readlen = MAXBUFLEN ;
	    } else {
	        readlen = ps ;
	    }

#if	CF_DEBUGS
	    debugprintf("uc_copy: mid1 rs=%d fsize=%lu readlen=%u\n",rs,
	        fsize,readlen) ;
#endif

	    if ((rs >= 0) && (fsize > 0)) {
	        char		*mdata = NULL ;

#if	CF_VALLOC
#else
	        size_t		msize = 0 ;
#endif /* CF_VALLOC */

#if	CF_DEBUGS
	        debugprintf("uc_copy: readlen=%u\n",readlen) ;
#endif

/* allocate the buffer */

#if	CF_VALLOC
	        rs = uc_libvalloc(readlen,&mdata) ;
#else
	        if (readlen <= ps) {
	            rs = uc_libvalloc(readlen,&mdata) ;
	        } else {
	            const int	mprot = (PROT_READ|PROT_WRITE) ;
	            const int	mflag = (MAP_PRIVATE|MAP_ANON) ;
	            msize = readlen ;
	            rs = u_mmap(NULL,msize,mprot,mflag,-1,0L,&mdata) ;
	        }
#endif /* CF_VALLOC */

#if	CF_DEBUGS
	        debugprintf("uc_copy: before rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            int	wlen ;

/* perform the copy (using the allocated buffer) */

	            while ((ulen < 0) || (tlen < ulen)) {
	                int	rlen = readlen ;

	                if (ulen >= 0) rlen = MIN((ulen - tlen),readlen) ;

	                rs = u_read(sfd,mdata,rlen) ;
	                wlen = rs ;
#if	CF_DEBUGS
	                debugprintf("uc_copy: u_read() rs=%d\n",rs) ;
#endif
	                if (rs <= 0) break ;

	                rs = uc_writen(dfd,mdata,wlen) ;
	                tlen += rs ;

	                if (rs < 0) break ;
	            } /* end while */

#if	CF_DEBUGS
	            debugprintf("uc_copy: while-end rs=%d tlen=%u\n",rs,tlen) ;
#endif

#if	CF_VALLOC
	            uc_libfree(mdata) ;
#else
	            if (msize == 0) {
	                uc_libfree(mdata) ;
	            } else {
	                u_munmap(mdata,msize) ;
	            }
#endif /* CF_VALLOC */
	        } /* end if (memory allocation) */

	        tlen &= INT_MAX ;

	    } /* end if (positive) */

	} /* end if (non-zero) */

#if	CF_DEBUGS
	debugprintf("uc_copy: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uc_copy) */


int uc_writedesc(int ofd,int ifd,int wlen)
{
#if	CF_DEBUGS
	debugprintf("uc_writedesc: ent ulen=%d\n",wlen) ;
	deubgprintstat("ifd",ifd) ;
	deubgprintstat("ofd",ofd) ;
#endif
	return uc_copy(ifd,ofd,wlen) ;
}
/* end subroutine (uc_writedesc) */


#if	CF_DEBUGS
static int deubgprintstat(cchar *id,int fd)
{
	USTAT		sb ;
	int		rs ;
	if ((rs = u_fstat(fd,&sb)) >= +0) {
	    ulong	fs = sb.st_size ;
	    debugprintf("uc_writedesc: %s size=%lu\n",id,fs) ;
	}
	return rs ;
}
/* end subroutine (debugprintstat) */
#endif /* CF_DEBUGS */


