/* filemap */

/* support low-overhead file bufferring requirements */


#define	CF_DEBUGS	0		/* debug print-outs */
#define	CF_SAFE		0		/* safe mode */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I/O requirements.


*******************************************************************************/


#include	<envstandards.h>

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

#include	"filemap.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#define	BUFLEN		(2 * MAXPATHLEN)


/* external subroutines */

#if	CF_DEBUGS
extern int	isprintlatin(int) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int filemap_open(FILEMAP *fmp,cchar *fname,int oflags,size_t maxsize)
{
	int		rs ;

	if (fmp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("filemap_open: fn=%s\n",fname) ;
#endif

	if (maxsize == 0) maxsize = INT_MAX ;

	memset(fmp,0,sizeof(FILEMAP)) ;
	fmp->maxsize = maxsize ;

	if ((rs = uc_open(fname,oflags,0666)) >= 0) {
	    struct ustat	*sbp = &fmp->sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,sbp)) >= 0) {
		if (S_ISREG(sbp->st_mode)) {
		    const size_t	ps = getpagesize() ;
		    if ((maxsize > 0) && (sbp->st_size <= maxsize)) {
	    	        size_t	ms = MAX(ps,sbp->st_size) ;
	    	        int	mp = PROT_READ ;
	    	        int	mf = MAP_SHARED ;
	    	        void	*md ;
		        if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
			    const int		madv = MADV_SEQUENTIAL ;
			    const caddr_t	ma = md ;
	    		    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
			         fmp->mapdata = md ;
			         fmp->mapsize = ms ;
			         fmp->bp = md ;
			    }
			    if (rs < 0)
				u_munmap(md,ms) ;
	    	        } /* end if (mmap) */
		    } else
	    	        rs = SR_TOOBIG ;
	        } else
	            rs = SR_PROTO ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("filemap_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filemap_open) */


int filemap_close(FILEMAP *fmp)
{
	int		rs = SR_OK ;

	if (fmp == NULL) return SR_FAULT ;

	if (fmp->mapdata != NULL) {
	    size_t	ms = fmp->mapsize ;
	    const void	*md = fmp->mapdata ;
	    rs = u_munmap(md,ms) ;
	    fmp->mapdata = NULL ;
	    fmp->mapsize = 0 ;
	}

	fmp->bp = NULL ;
	return rs ;
}
/* end subroutine (filemap_close) */


int filemap_stat(FILEMAP *fmp,struct ustat *sbp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	*sbp = fmp->sb ;

	return rs ;
}
/* end subroutine (filemap_stat) */


int filemap_read(FILEMAP *fmp,int rlen,void *vp)
{
	size_t		fsize ;
	int		rs ;
	const char	*bdata ;
	const char	*sbp, *ebp ;
	const void	**bpp = (const void **) vp ;

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (bpp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("filemap_read: ent rlen=%u\n",rlen) ;
#endif

	bdata = (const char *) fmp->mapdata ;
	fsize = MIN((fmp->sb.st_size & SIZE_MAX),fmp->maxsize) ;
	sbp = fmp->bp ;
	ebp = (bdata + fsize) ;

	fmp->bp = (const char *) MIN(ebp,(sbp+rlen)) ;

	*bpp = (void *) sbp ;
	rs = (fmp->bp - sbp) ;

#if	CF_DEBUGS
	debugprintf("filemap_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filemap_read) */


int filemap_getline(FILEMAP *fmp,cchar **bpp)
{
	size_t		fsize ;
	int		rs = SR_OK ;
	const char	*bdata ;
	const char	*sbp, *ebp ;

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (bpp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	bdata = (const char *) fmp->mapdata ;
	fsize = MIN((fmp->sb.st_size & SIZE_MAX),fmp->maxsize) ;
	sbp = fmp->bp ;
	ebp = (bdata + fsize) ;

#if	CF_DEBUGS
	{
	    int	ch = MKCHAR(*sbp) ;
	    debugprintf("filemap_getline: first ch=%02x (%c)\n",
		ch,(isprintlatin(ch) ? ch : '¯')) ;
	}
	debugprintf("filemap_getline: remaining len=%d\n",(ebp-fmp->bp)) ;
#endif

	while (fmp->bp < ebp) {
	    if (*fmp->bp++ == '\n') break ;
	} /* end while */

	*bpp = sbp ;
	rs = (fmp->bp - sbp) ;

#if	CF_DEBUGS
	debugprintf("filemap_getline: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (filemap_getline) */


int filemap_seek(FILEMAP *fmp,offset_t off,int w)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */


	switch (w) {
	case SEEK_SET:
	    break ;
	case SEEK_END:
	    off = (fmp->sb.st_size + off) ;
	    break ;
	case SEEK_CUR:
	    {
		const char	*bdata = (const char *) fmp->mapdata ;
	        off = ((fmp->bp - bdata) + off) ;
	    }
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if (rs >= 0) {
	    const char	*bdata = (const char *) fmp->mapdata ;
	    if (off < 0) {
	        rs = SR_INVALID ;
	    } else if (off > fmp->mapsize)
	        off = fmp->mapsize ;
	    fmp->bp = (bdata + off) ;
	} /* end if */

	return rs ;
}
/* end subroutine (filemap_seek) */


int filemap_tell(FILEMAP *fmp,offset_t *offp)
{
	int		rs ;

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	{
	    const char	*bdata = (const char *) fmp->mapdata ;
	    if (offp != NULL) *offp = (fmp->bp - bdata) ;
	    rs = ((fmp->bp - bdata) & INT_MAX) ;
	}
	return rs ;
}
/* end subroutine (filemap_tell) */


int filemap_rewind(FILEMAP *fmp)
{

#if	CF_SAFE
	if (fmp == NULL) return SR_FAULT ;
	if (fmp->mapdata == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	fmp->bp = fmp->mapdata ;
	return SR_OK ;
}
/* end subroutine (filemap_rewind) */


