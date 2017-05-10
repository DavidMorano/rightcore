/* uc_copyfile */

/* copy from one file descriptor to another */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */
#define	F_MAPPABLE	0		/* use file mapping ? */


/* revision history :

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


	Synopsis :

	int uc_copyfile(sfd,dfd)
	int	sfd, dfd ;


	Arguments :

	sfd		source file descriptor
	dfd		destination file descriptor


	Returns :

	<0		error
	>=0		length copied



******************************************************************************/




#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<malloc.h>

#include	<vsystem.h>

#include	"misc.h"



/* local defines */

#define	NPAGES		8



/* external subroutines */

extern int	u_fstat(), u_seek(), u_mmap(), u_munmap() ;
extern int	u_read(), u_writen() ;

extern int	uc_valloc(int,void *) ;






int uc_copyfile(sfd,dfd)
int	sfd, dfd ;
{

#if	F_MAPPABLE
	struct ustat	sb ;

	offset_t	offset, offset2 ;
#endif

	ulong	pagesize ;

	int	len, len2, mlen, wlen ;
	int	tlen = 0 ;
	int	pageoff ;
	int	rs = SR_OK ;

	char	*buf ;


	pagesize = sysconf(_SC_PAGESIZE) ;

#if	F_MAPPABLE

#if	F_DEBUGS
	eprintf("uc_copyfile: entered, pagesize=%u sfd=%d\n",
	    pagesize,sfd) ;
#endif

	if ((rs = u_fstat(sfd,&sb)) < 0) {

#if	F_DEBUGS
	    eprintf("uc_copyfile: stat of source, rs=%d\n",rs) ;
#endif

	    return rs ;
	}

/* is the source file mappable or not ? */

	if (S_ISREG(sb.st_mode) && 
	    ((offset = lseek(sfd,0L,SEEK_CUR)) >= 0L)) {

#if	F_DEBUGS
	    eprintf("uc_copyfile: map copy\n") ;
#endif

/* copy the fragmented page before a page boundary */

	    pageoff = offset & (pagesize - 1) ;
	    if (len2 = (pagesize - pageoff)) {

#if	F_DEBUGS
	        eprintf("uc_copyfile: initial page fragment\n") ;
#endif

	        offset2 = offset & (~ (pagesize - 1)) ;
	        rs = u_mmap(NULL,pagesize,
	            PROT_READ,MAP_SHARED,sfd,offset2,&buf) ;

	        if (rs < 0)
	            return rs ;

	        wlen = MIN(len2,(sb.st_size - offset)) ;
	        rs = u_writen(dfd,buf,wlen) ;

	        u_munmap(buf,pagesize) ;

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

#if	F_DEBUGS
	    eprintf("uc_copyfile: all other pages \n") ;
#endif

	    while (offset < sb.st_size) {

#if	F_DEBUGS
	        eprintf("uc_copyfile: top loop\n") ;
#endif

	        mlen = pagesize * NPAGES ;
	        rs = u_mmap(NULL,mlen,
	            PROT_READ,MAP_SHARED,sfd,offset,&buf) ;

	        if (rs < 0)
	            return rs ;

	        wlen = MIN(mlen,(sb.st_size - offset)) ;
	        rs = u_writen(dfd,buf,wlen) ;

	        (void) u_munmap(buf,pagesize) ;

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

	        if ((rs = u_writen(dfd,buf,len)) < len)
	            break ;

	        tlen += len ;

	    } /* end while */

	    if (rs < 0)
	        tlen = rs ;

	    if (len < 0)
	        tlen = len ;

	    free(buf) ;

	} /* end if */

#if	F_DEBUGS
	eprintf("uc_copyfile: exit tlen=%d\n",tlen) ;
#endif

#else /* F_MAPPABLE */

#if	F_DEBUGS
	eprintf("uc_copyfile: entered, sfd=%d\n",sfd) ;
#endif

	if ((rs = uc_valloc(pagesize,&buf)) < 0)
	    return rs ;

/* CONSTCOND */

	while (TRUE) {

	    len = u_read(sfd,buf,pagesize) ;

#if	F_DEBUGS
	    eprintf("uc_copyfile: inside loop len=%d\n",len) ;
#endif

	    if (len <= 0)
		break ;

	    if ((rs = u_writen(dfd,buf,len)) < len)
	        break ;

	    tlen += len ;

	} /* end while */

#if	F_DEBUGS
	eprintf("uc_copyfile: after loop len=%d\n",len) ;
#endif

	if (len < 0) {

	    tlen = len ;

	} else if (rs < len) {

	    if (rs >= 0)
	        tlen += rs ;

	    else
	        tlen = rs ;

	}

	free(buf) ;

#if	F_DEBUGS
	eprintf("uc_copyfile: exit tlen=%d\n",tlen) ;
#endif

#endif /* F_MAPPABLE */

	return tlen ;
}
/* end subroutine (uc_copyfile) */



