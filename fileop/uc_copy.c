/* uc_copy */

/* interface component for UNIX® library-3c */
/* copy from one file descriptor to another */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_MALLOC	0		/* only use 'vmalloc(3c)' */


/* revision history:

	= 1998-01-10, David A­D­ Morano

	The subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies data from one file descriptor to another
	(source to destination) for the length specified as an argument.

	The idea of having a separate subroutine for this sort of
	(usually) trivial function, is that we can perform optimizations
	that a typical user would find tiresome for such a "simple"
	function.

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


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#undef	LOCAL_DARWIN
#define	LOCAL_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#if	CF_MALLOC
#define	MAXBUFLEN	(16*1024*1024)
#else
#define	MAXBUFLEN	(2*1024*1024)
#endif

#ifndef	BCEIL
#define	BCEIL(v,m)	(((v) + ((m) - 1)) & (~ ((m) - 1)))
#endif


/* external subroutines */


/* exported subroutines */


int uc_copy(int sfd,int dfd,int ulen)
{
	const int	mprot = (PROT_READ|PROT_WRITE) ;
	const int	mflag = (MAP_PRIVATE|MAP_ANON) ;
	size_t		fsize = -1 ;
	size_t		msize = 0 ;
	size_t		tlen = 0 ;
	const int	ps = getpagesize() ;
	int		rs = SR_OK ;
	int		readlen ;
	int		wlen ;
	char		*mdata = NULL ;

	if (sfd == dfd) return SR_INVALID ;

	if (ulen == 0)
	    goto ret0 ;

/* calculate the size of a buffer to allocate */

	if ((ulen < 0) || (ulen > ps)) {

	    if (ulen >= 0) {
	        readlen = BCEIL(ulen,ps) ;
	    } else {
	        struct ustat	sb ;
		int		bsize ;
		rs = u_fstat(sfd,&sb) ;
		if (rs >= 0) {
		    const mode_t	m = sb.st_mode ;
		    if (S_ISREG(m) || S_ISBLK(m)) {
			fsize = sb.st_size ;
			bsize = MAX(sb.st_size,1) & INT_MAX ;
	        	readlen = BCEIL(bsize,ps) ;
		    } else if (S_ISFIFO(m)) {
			readlen = 2048 ;
		    } else
	    	        readlen = ps ;
		}
	    } /* end if */
	    if (readlen > MAXBUFLEN) readlen = MAXBUFLEN ;

	} else
	    readlen = ps ;

	if (rs < 0) goto ret0 ;
	if (fsize == 0) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("uc_copy: readlen=%u\n",readlen) ;
#endif

/* allocate the buffer */

#if	CF_MALLOC
	rs = uc_valloc(readlen,&mdata) ;
#else
	if (readlen <= ps) {
	    rs = uc_valloc(readlen,&mdata) ;
	} else {
	    msize = readlen ;
	    rs = u_mmap(NULL,msize,mprot,mflag,-1,0L,&mdata) ;
	}
#endif /* CF_MALLOC */

#if	CF_DEBUGS
	debugprintf("uc_copy: before rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

/* perform the copy (using the allocated buffer) */

	    while ((ulen < 0) || (tlen < ulen)) {
	        int rlen = (ulen >= 0) ? MIN((ulen - tlen),readlen) : readlen ;

	        rs = u_read(sfd,mdata,rlen) ;
	        wlen = rs ;

#if	CF_DEBUGS
	debugprintf("uc_copy: u_read() rs=%d\n",rs) ;
#endif

	        if (rs <= 0) break ;

	        rs = uc_writen(dfd,mdata,wlen) ;

#if	CF_DEBUGS
	debugprintf("uc_copy: uc_writen() rs=%d\n",rs) ;
#endif

	        if (rs < 0) break ;
	        tlen += rs ;

	    } /* end while */

#if	CF_DEBUGS
	debugprintf("uc_copy: while-end rs=%d tlen=%u\n",rs,tlen) ;
#endif

#if	CF_MALLOC
	    uc_free(mdata) ;
#else
	    if (msize <= 0) {
	        uc_free(mdata) ;
	    } else {
	        u_munmap(mdata,msize) ;
	    }
#endif /* CF_MALLOC */
	} /* end if (memory allocation) */

	tlen &= INT_MAX ;

ret0:

#if	CF_DEBUGS
	debugprintf("uc_copy: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uc_copy) */


/* local subroutines */


#ifdef	COMMENT

static int copymapped(sfd,dfd,ulen)
int	sfd, dfd ;
int	ulen ;
{
	TYPESTAT	sb ;
	TYPEOFFSET	offset, offset2 ;
	const int	ps = getpagesize() ;
	int		rs = SR_OK ;
	int		pageoff ;
	int		len, mlen, rlen, wlen ;
	int		tlen = 0 ;
	int		f ;
	char	*buf = NULL ;

	rs = FUNCFSTAT(sfd,&sb) ;
	if (rs < 0)
	    goto ret0 ;

	if (ulen < 0)
	    ulen = (sb.st_size & INT_MAX) ;

/* is the source file mappable or not? */

	if (f = S_ISREG(sb.st_mode)) {
	    rs = FUNCSEEKOFF(sfd,0L,SEEK_CUR,&offset) ;
	    f = (rs >= 0) ;
	}

	if (f) {

#if	CF_DEBUGS
	    debugprintf("uc_copy: map copy\n") ;
#endif

/* copy the fragmented page before a page boundary */

	    pageoff = offset & (ps - 1) ;
	    len = (ps - pageoff) & (ps - 1) ;
	    if (len > 0) {

#if	CF_DEBUGS
	        debugprintf("uc_copy: initial page fragment\n") ;
#endif

	        offset2 = offset & (~ (ps - 1)) ;
	        rs = u_mmap(NULL,(size_t) ps,
	            PROT_READ,MAP_SHARED,sfd,offset2,&buf) ;

#if	CF_DEBUGS
	        debugprintf("uc_copy: u_mmap() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            return rs ;

	        wlen = MIN(len,ulen) ;
	        rs = uc_writen(dfd,(buf + pageoff),wlen) ;

#if	CF_DEBUGS
	        debugprintf("uc_copy: uc_writen() rs=%d\n",rs) ;
#endif

	        u_munmap(buf,(size_t) ps) ;

	        if (rs < wlen) {

	            if (rs < 0)
	                return rs ;

	            FUNCSEEK(sfd,(TYPEOFFSET) wlen,SEEK_CUR) ;

	            return wlen ;
	        }

	        offset += rs ;
	        tlen += rs ;

	    } /* end if */

/* copy all other pages */

#if	CF_DEBUGS
	    debugprintf("uc_copy: all other pages \n") ;
#endif

	    while (tlen < ulen) {

#if	CF_DEBUGS
	        debugprintf("uc_copy: top loop\n") ;
#endif

	        mlen = MIN((ps * NPAGES),ulen) ;
	        mlen = (mlen + (ps - 1)) & (~ (ps - 1)) ;
	        rs = u_mmap(NULL,(size_t) mlen,
	            PROT_READ,MAP_SHARED,sfd,offset,&buf) ;

	        if (rs < 0)
	            break ;

	        wlen = MIN(mlen,ulen) ;
	        rs = uc_writen(dfd,buf,wlen) ;

	        u_munmap(buf,(size_t) mlen) ;

	        if (rs < 0)
	            break ;

	        offset += rs ;
	        tlen += rs ;
	        if (rs < wlen)
	            break ;

	    } /* end while */

	    FUNCSEEK(sfd,(TYPEOFFSET) tlen,SEEK_CUR) ;

	} /* end if (mappable) */

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (copymapped) */

#endif /* COMMENT */


