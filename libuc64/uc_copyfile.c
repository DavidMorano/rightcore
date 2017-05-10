/* uc_copyfile */

/* copy from one file descriptor to another */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_MAPPABLE	0		/* use file mapping ? (DO NOT USE) */


/* revision history:

	= 88/01/10, David A­D­ Morano

	The subroutine was originally written.


*/


/*****************************************************************************

	This subroutine copies the remaining contents (up until
	the EOF) from a source file descriptor to the destination
	file descriptor.

	The idea of having a separate subroutine for this sort of
	(usually) trivial function, is that we can perform optimizations
	that a typical user would find tiresome for such a "simple"
	function.

	Synopsis:

	int uc_copyfile(sfd,dfd)
	int	sfd, dfd ;

	Arguments:

	sfd		source file descriptor
	dfd		destination file descriptor

	Returns:

	<0		error
	>=0		length copied


******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#undef	LOCAL_DARWIN
#define	LOCAL_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<unistd.h>

#if	LOCAL_SOLARIS
#include	<malloc.h>
#elif	LOCAL_DARWIN
#include	<stdlib.h>
#endif

#include	<vsystem.h>

#include	"localmisc.h"



/* local defines */

#define	NPAGES		8



/* external subroutines */


/* exported subroutines */


int uc_copyfile(sfd,dfd)
int	sfd, dfd ;
{

#if	CF_MAPPABLE
	struct ustat	sb ;

	offset_t	offset, offset2 ;
#endif

	int	pagesize ;
	int	len, mlen, wlen ;
	int	tlen = 0 ;
	int	pageoff ;
	int	rs = SR_OK ;

	char	*buf ;


	pagesize = getpagesize() ;

#if	CF_MAPPABLE

#if	CF_DEBUGS
	debugprintf("uc_copyfile: entered, pagesize=%u sfd=%d\n",
	    pagesize,sfd) ;
#endif

	if ((rs = u_fstat(sfd,&sb)) < 0) {

#if	CF_DEBUGS
	    debugprintf("uc_copyfile: stat of source, rs=%d\n",rs) ;
#endif

	    return rs ;
	}

/* is the source file mappable or not ? */

	if (S_ISREG(sb.st_mode) && 
	    ((offset = lseek(sfd,0L,SEEK_CUR)) >= 0L)) {

#if	CF_DEBUGS
	    debugprintf("uc_copyfile: map copy\n") ;
#endif

/* copy the fragmented page before a page boundary */

	    pageoff = offset & (pagesize - 1) ;
	    len = (pagesize - pageoff) ;
	    if (len > 0) {

#if	CF_DEBUGS
	        debugprintf("uc_copyfile: initial page fragment\n") ;
#endif

	        offset2 = offset & (~ (pagesize - 1)) ;
	        rs = u_mmap(NULL,(size_t) pagesize,
	            PROT_READ,MAP_SHARED,sfd,offset2,&buf) ;

	        if (rs < 0)
	            return rs ;

	        wlen = MIN(len,(sb.st_size - offset)) ;
	        rs = uc_writen(dfd,buf,wlen) ;

	        u_munmap(buf,(size_t) pagesize) ;

	        if (rs < wlen) {

	            if (rs < 0)
	                return rs ;

	            u_seek(sfd,wlen,SEEK_CUR) ;

	            return wlen ;
	        }

	        offset += rs ;
	        tlen += rs ;

	    } /* end if */

/* copy all other pages */

#if	CF_DEBUGS
	    debugprintf("uc_copyfile: all other pages \n") ;
#endif

	    while (offset < sb.st_size) {

#if	CF_DEBUGS
	        debugprintf("uc_copyfile: top loop\n") ;
#endif

	        mlen = pagesize * NPAGES ;
	        rs = u_mmap(NULL,(size_t) mlen,
	            PROT_READ,MAP_SHARED,sfd,offset,&buf) ;

	        if (rs < 0)
	            return rs ;

	        wlen = MIN(mlen,(sb.st_size - offset)) ;
	        rs = uc_writen(dfd,buf,wlen) ;

	        u_munmap(buf,(size_t) pagesize) ;

	        if (rs < wlen) {

	            if (rs < 0)
	                return rs ;

	            tlen += rs ;
	            u_seek(sfd,tlen,SEEK_CUR) ;

	            return tlen ;
	        }

	        offset += rs ;
	        tlen += rs ;

	    } /* end while */

	    u_seek(sfd,tlen,SEEK_CUR) ;

	} else {

	    if ((rs = uc_valloc(pagesize,&buf)) < 0)
	        return rs ;

	    while ((len = u_read(sfd,buf,pagesize)) > 0) {

	        if ((rs = uc_writen(dfd,buf,len)) < len)
	            break ;

	        tlen += len ;

	    } /* end while */

	    if (rs < 0)
	        tlen = rs ;

	    if (len < 0)
	        tlen = len ;

	    free(buf) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_copyfile: exit tlen=%d\n",tlen) ;
#endif

#else /* CF_MAPPABLE */

#if	CF_DEBUGS
	debugprintf("uc_copyfile: entered, sfd=%d\n",sfd) ;
#endif

	rs = uc_valloc(pagesize,&buf) ;

	if (rs >= 0) {

/* CONSTCOND */

	while (TRUE) {

	    rs = u_read(sfd,buf,pagesize) ;

#if	CF_DEBUGS
	    debugprintf("uc_copyfile: inside loop len=%d\n",len) ;
#endif

	    if (rs <= 0)
		break ;

		len = rs ;
	    rs = uc_writen(dfd,buf,len) ;

	    if (rs < len)
	        break ;

	    tlen += len ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("uc_copyfile: after loop rs=%d\n",rs) ;
#endif

		free(buf) ;

	} /* end if */

#endif /* CF_MAPPABLE */

#if	CF_DEBUGS
	debugprintf("uc_copyfile: ret rs=%d tlen=%d\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uc_copyfile) */



