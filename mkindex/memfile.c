/* memfile */

/* support low-overhead file bufferring requirements */


#define	CF_DEBUGS	0		/* debug print-outs */
#define	CF_SAFE		1		/* safe mode */
#define	CF_ACTUAL	1		/* actually write the file */


/* revision history:

	= 1998-4-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I-O requirements.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"memfile.h"


/* local defines */

#define	ZEROBUFLEN	1024


/* external subroutines */

extern uint	uceil(uint,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	memfile_extend(MEMFILE *) ;
static int	memfile_mapextend(MEMFILE *,uint) ;
static int	memfile_ismemfree(MEMFILE *,caddr_t,uint) ;


/* local variables */


/* exported subroutines */


int memfile_open(MEMFILE *fbp,cchar *fname,int oflags,mode_t om)
{
	int		rs ;

	if (fbp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	oflags &= (~ (O_RDONLY | O_WRONLY)) ;
	oflags |= O_RDWR ;

	memset(fbp,0,sizeof(MEMFILE)) ;

	if ((rs = uc_open(fname,oflags,om)) >= 0) {
	    USTAT	sb ;
	    const int	fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            size_t	ms, e ;
	            int		mf, mp ;
	            void	*md ;

	    	    fbp->pagesize = getpagesize() ;
	            uc_closeonexec(fd,TRUE) ;

	            fbp->off = 0 ;
	            fbp->fd = fd ;

	            e = (sb.st_size + (fbp->pagesize - 1)) ;
	            ms = uceil(e,fbp->pagesize) ;

	            mp = (PROT_READ | PROT_WRITE) ;
	            mf = MAP_SHARED ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                fbp->fsize = sb.st_size ;
	                fbp->dbuf = md ;
	                fbp->dlen = ms ;
	                if ((rs = memfile_extend(fbp)) >= 0) {
	                    fbp->bp = fbp->dbuf ;
	                    fbp->magic = MEMFILE_MAGIC ;
	                }
	                if (rs < 0) {
	                    u_munmap(fbp->dbuf,fbp->dlen) ;
	                    fbp->dbuf = NULL ;
	                    fbp->dlen = 0 ;
	                }
	            } /* end if (map) */
	        } else {
	            rs = SR_PROTO ;
		}
	    } /* end if (stat) */
	    if (rs < 0) {
	        u_close(fd) ;
		fbp->fd = -1 ;
	    }
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("memfile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (memfile_open) */


int memfile_close(MEMFILE *fbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

	if (fbp->fd >= 0) {
	    rs1 = u_close(fbp->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    fbp->fd = -1 ;
	}

	if (fbp->dbuf != NULL) {
	    rs1 = u_munmap(fbp->dbuf,fbp->dlen) ;
	    if (rs >= 0) rs = rs1 ;
	    fbp->dbuf = NULL ;
	    fbp->dlen = 0 ;
	}

	fbp->magic = 0 ;
	return rs ;
}
/* end subroutine (memfile_close) */


int memfile_write(MEMFILE *fbp,const void *buf,int buflen)
{
	offset_t	off ;
	uint		pmo ;
	int		rs = SR_OK ;
	int		allocation, p, extra, extension ;
	char		zerobuf[2] ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("memfile_write: buflen=%d\n",buflen) ;
#endif

	pmo = fbp->pagesize - 1 ;
	if ((fbp->off + buflen) > fbp->fsize) {

#if	CF_DEBUGS
	    debugprintf("memfile_write: writing beyond file end\n") ;
#endif

	    allocation = uceil(fbp->fsize,fbp->pagesize) ;

	    if ((fbp->off + buflen) > allocation) {

#if	CF_DEBUGS
	        debugprintf("memfile_write: writing beyond file allocation\n") ;
#endif

	        p = 4 * fbp->pagesize ;
	        extension = MAX(((fbp->off + buflen) - allocation),p) ;

/* extend the map */

	        if ((rs = memfile_mapextend(fbp,extension)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("memfile_write: extending file\n") ;
#endif

	            zerobuf[0] = '\0' ;
	            for (off = allocation ; off < (allocation + extension) ; 
	                off += fbp->pagesize) {

#if	CF_DEBUGS
	                debugprintf("memfile_write: off=%llu\n",off) ;
#endif

#if	F_ZEROFILL
	                {
	                    char	*zp ;
	                    uc_malloc(fbp->pagesize,&zp) ;
	                    memset(zp,0,fbp->pagesize) ;
	                    rs = u_pwrite(fbp->fd,zp,fbp->pagesize,off) ;
	                    uc_free(zp) ;
	                }
#else
	                rs = u_pwrite(fbp->fd,zerobuf,1,(off + pmo)) ;
#endif

	                if (rs < 0) break ;
	            } /* end for */

#if	CF_DEBUGS
	            debugprintf("memfile_write: u_pwrite() collective rs=%d\n",
	                rs) ;
#endif

	        } /* end if (extending file) */

	    } /* end if (extending map and file) */

	    if (rs >= 0) {
	        extra = (fbp->off + buflen) - fbp->fsize ;
	        fbp->fsize += extra ;
	    }

	} /* end if (writing beyond file end) */

/* finally do the write! */

#if	CF_DEBUGS
	debugprintf("memfile_write: rs=%d writing at off=%llu\n",
	    rs,fbp->off) ;
#endif

#if	CF_ACTUAL
	if (rs >= 0)
	    rs = u_pwrite(fbp->fd, buf,buflen,fbp->off) ;
#else

#if	CF_DEBUGS
	debugprintf("memfile_write: memcpy() to(%p) \n",
	    (fbp->dbuf + fbp->off)) ;
#endif

	memcpy((fbp->dbuf + fbp->off),buf,buflen) ;

#endif /* CF_ACTUAL */

	if (rs >= 0) {
	    if ((fbp->off + buflen) > fbp->fsize) {
	        fbp->fsize = (fbp->off + buflen) ;
	    }
	    fbp->off += buflen ;
	}

#if	CF_DEBUGS
	debugprintf("memfile_write: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? buflen : rs ;
}
/* end subroutine (memfile_write) */


int memfile_len(MEMFILE *fbp)
{
	int		rs = SR_OK ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

	return fbp->fsize ;
}
/* end subroutine (memfile_len) */


int memfile_allocation(MEMFILE *fbp)
{
	int		rs = SR_OK ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

	return fbp->dlen ;
}
/* end subroutine (memfile_allocation) */


int memfile_tell(MEMFILE *fbp,offset_t *offp)
{
	int		rs = SR_OK ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

	if (offp != NULL)
	    *offp = fbp->off ;

	return fbp->off ;
}
/* end subroutine (memfile_tell) */


int memfile_buf(MEMFILE *fbp,void *vp)
{
	caddr_t		*rpp = (caddr_t *) vp ;

	if (fbp == NULL) return SR_FAULT ;

	if (fbp->magic != MEMFILE_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL) *rpp = (caddr_t) fbp->dbuf ;

	return SR_OK ;
}
/* end subroutine (memfile_buf) */


/* private subroutines */


/* extend the file out to the allocation (for starters) */
static int memfile_extend(MEMFILE *fbp)
{
	offset_t	off = fbp->fsize ;
	int		rs = SR_OK ;
	char		zerobuf[ZEROBUFLEN + 1] ;

	memset(zerobuf,0,ZEROBUFLEN) ;

	while (off < fbp->dlen) {
	    int	clen, dlen ;

	    if ((off % ZEROBUFLEN) == 0) {
	        clen = ZEROBUFLEN ;
	    } else {
	        clen = uceil((uint) off,ZEROBUFLEN) ;
	    }

	    dlen = MIN(ZEROBUFLEN,(clen - fbp->fsize)) ;

#if	CF_DEBUGS
	    debugprintf("memfile_open: clearing off=%llu dlen=%u\n",
	        off,dlen) ;
#endif

	    rs = u_pwrite(fbp->fd,zerobuf,dlen,off) ;
	    off += dlen ;

	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (memfile_extend) */


static int memfile_mapextend(MEMFILE *fbp,uint extension)
{
	offset_t	moff ;
	caddr_t		addr ;
	size_t		msize ;
	int		rs = SR_INVALID ;
	int		mprot ;
	int		mflags ;

	mprot = PROT_READ | PROT_WRITE ;
	mflags = MAP_SHARED ;

/* first we try to extend our existing map */

	addr = (fbp->dbuf + fbp->dlen) ;
	if (memfile_ismemfree(fbp,addr,extension) > 0) {
	    int		fd = fbp->fd ;
	    void	*p ;

	    moff = fbp->dlen ;
	    msize = extension ;
	    if ((rs = u_mmap(addr,msize,mprot,mflags,fd,moff,&p)) >= 0) {
	        fbp->dbuf = p ;
	        fbp->dlen += extension ;
	    }

	} /* end if */

/* do we need to remap entirely? */

	if (rs < 0) {

	    if ((rs = uc_fdatasync(fbp->fd)) >= 0) {
	        msize = fbp->dlen ;
	        rs = u_munmap(fbp->dbuf,msize) ;
	        fbp->dbuf = NULL ;
	    }

	    if (rs >= 0) {
	        const int	fd = fbp->fd ;
	        void		*p ;

	        msize = (fbp->dlen + extension) ;
	        fbp->dlen = 0 ;
	        if ((rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&p)) >= 0) {
	            fbp->dbuf = p ;
	            fbp->dlen = msize ;
	        }

	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (memfile_mapextend) */


static int memfile_ismemfree(MEMFILE *fbp,caddr_t addr,uint len)
{
	size_t		tlen ;
	caddr_t		ta = addr ;
	int		rs = SR_NOMEM ;
	char		vec[10] ;

	tlen = fbp->pagesize ;
	for (ta = addr ; ta < (addr + len) ; ta += fbp->pagesize) {
	    rs = u_mincore(ta,tlen,vec) ;
	    if (rs != SR_NOMEM) break ;
	} /* end for */

	return (rs == SR_NOMEM) ? TRUE : SR_EXIST ;
}
/* end subroutine (memfile_ismemfree) */


