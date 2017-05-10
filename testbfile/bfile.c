/* bfile */

/* utility subroutines for BFILE */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FLUSHN	0		/* compile |bflushn(3b)| */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are some general utility subroutines for BFILE, mainly
	to maintenance portability consideration.


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

#define	BDESC		BFILE_BD


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

static int bfile_findread(bfile *,offset_t,char *,int,int,int) ;

static int bfile_findunused(bfile *) ;
static int bfile_findmin(bfile *) ;

static int bfilebd_check(BDESC *,bfile *,offset_t,int) ;
static int bfilebd_overlap(BFILE_BD *,offset_t,int) ;
static int bfilebd_invalidate(BFILE_BD *,bfile *,offset_t,int) ;


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


#if	CF_FLUSHN
int bfile_flushn(fp,n)
bfile		*fp ;
int		n ;
{
	int	rs = SR_OK ;
	int	len = 0 ;
	int	f_sa ;


	if (n == 0) goto ret0 ;

	f_sa = (! fp->f.notseek) && (fp->oflags & O_APPEND) ;

#if	CF_DEBUGS
	debugprintf("bfile_flush: f_sa=%u\n",f_sa) ;
#endif

	if (f_sa) {
	    offset_t	o ;
	    rs = u_seeko(fp->fd,0LL,SEEK_END,&o) ;
	    fp->foff = o ;

#if	CF_DEBUGS
	    debugprintf("bfile_flush: u_seeko() rs=%d\n",rs) ;
	    debugprintf("bfile_flush: file offset=%lld\n",fp->foff) ;
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
	            fp->foff += len ;
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
#endif /* CF_FLUSHN */


int bfile_flush(fp)
bfile		*fp ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;

	if (fp->bds != NULL) {
	    BFILE_BD	*bdp = fp->bds ;
	    const int	nbds = fp->nbds ;
	    int		bi = fp->bdi ;
	    int		i ;
	    for (i = 0 ; i < nbds ; i += 1) {
	        bdp = (fp->bds+bi) ;
		rs = bfilebd_flush(bdp,fp) ;
		wlen += rs ;
		bi = ((bi+1)%nbds) ;
		if (rs < 0) break ;
	    } /* end for */
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bfile_flush) */


int bfile_seeko(fp,o,w,fop)
bfile		*fp ;
offset_t	o ;
int		w ;
offset_t	*fop ;
{
	int	rs = SR_OK ;
	switch (w) {
	case SEEK_SET:
	    fp->foff = o ;
	    break ;
	case SEEK_CUR:
	    fp->foff += o ;
	    break ;
	case SEEK_END:
	    {
		offset_t	eo ;
	        rs = u_seeko(fp->fd,0L,w,&eo) ;
	        fp->foff = (eo + o) ;
	    }
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */
	if (fop != NULL) {
	    *fop = (rs>=0) ? fp->foff : 0L ;
	}
	return rs ;
}
/* end subroutine (bfile_seeko) */


int bfile_invalidate(fp,uoff,ulen)
bfile		*fp ;
offset_t	uoff ;
int		ulen ;
{
	int	rs = SR_OK ;
	int	f = FALSE ;

	if (ulen > 0) {
	    BFILE_BD	*bdp ;
	    const int	nbds = fp->nbds ;
	    int		i ;
	    int		bi = fp->bdi ;
	    for (i = 0 ; i < nbds ; i += 1) {
	        bdp = (fp->bds+bi) ;
	        if ((bdp->bdata != NULL) && (bdp->blen > 0)) {
		    if (bfilebd_overlap(bdp,uoff,ulen) > 0) {
		        f = TRUE ;
		        rs = bfilebd_invalidate(bdp,fp,uoff,ulen) ;
		    }
	        }
	        bi = ((bi+1)%nbds) ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (non-zero ulen) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bfile_invalidate) */


int bfile_bufwrite(fp,uoff,ubuf,ulen,wopts)
bfile		*fp ;
offset_t	uoff ;
const char	ubuf[] ;
int		ulen ;
int		wopts ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;



	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bfile_bufwrite) */


int bfile_bufread(fp,uo,ubuf,ulen,to,opts)
bfile		*fp ;
offset_t	uo ;
char		*ubuf ;
int		ulen ;
int		to ;
int		opts ;
{
	BFILE_BD	*bdp ;
	const int	nbds = fp->nbds ;
	const int	blen = ulen ;
	int	rs = SR_OK ;
	int	bl = 0 ;
	int	i ;
	int	bi = fp->bdi ;
	int	f_eof = FALSE ;
	char	*bp = (char *) ubuf ;

	for (i = 0 ; (bl < blen) && (i < nbds) ; i += 1) {
	    bdp = (fp->bds+bi) ;
	    f_eof = FALSE ;
	    if (bdp->bdata != NULL) {
	        rs = bfilebd_read(bdp,fp,uo,(bp+bl),(blen-bl),to,opts) ;
	        bl += rs ;
	        uo += rs ;
		f_eof = (rs == 0) ;
	    }
	    bi = ((bi+1)%nbds) ;
	    if (f_eof) break ;
	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (bl < blen)) {
	    rs = bfile_findread(fp,uo,(bp+bl),(blen-bl),to,opts) ;
	}

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bfile_bufread) */


static int bfile_findread(fp,fo,bbuf,blen,to,opts)
bfile		*fp ;
offset_t	fo ;
char		bbuf[] ;
int		blen ;
int		to ;
int		opts ;
{
	BFILE_BD	*bdp ;
	int	rs = SR_OK ;
	int	bl = 0 ;
	int	bi ;

	if ((bi = bfile_findunused(fp)) < 0) {
	    bi = bfile_findmin(fp) ;
	}
	bdp = (fp->bds+bi) ;
	if ((rs = bfilebd_check(bdp,fp,fo,blen)) >= 0) {
	    rs = bfilebd_read(bdp,fp,fo,bbuf,blen,to,opts) ;
	    bl = rs ;
	} /* end if */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bfile_findread) */


static int bfile_findunused(bfile *fp)
{
	BFILE_BD	*bdp ;
	const int	nbds = fp->nbds ;
	int	i ;
	for (i = 0 ; i < nbds ; i += 1) {
	    bdp = (fp->bds+i) ;
	    if ((bdp->bdata == NULL) || (bdp->blen == 0)) break ;
	} /* end for */
	return (i<nbds) ? i : -1 ;
}
/* end subroutine (bfile_findunused) */


static int bfile_findmin(bfile *fp)
{
	BFILE_BD	*bdp ;
	offset_t	min = LLONG_MAX ;
	const int	nbds = fp->nbds ;
	int		mi = -1 ;
	int	i ;
	for (i = 0 ; i < nbds ; i += 1) {
	    bdp = (fp->bds+i) ;
	    if ((bdp->bdata != NULL) && (bdp->blen > 0)) {
		if (bdp->boff < min) {
		    min = bdp->boff ;
		    mi = i ;
		}
	    }
	} /* end for */
	return mi ;
}
/* end subroutine (bfile_findmin) */


int bfilebd_start(BDESC *bdp,int bsize,offset_t boff)
{
	int	rs ;
	char	*bp ;

	memset(bdp,0,sizeof(BDESC)) ;

	if ((rs = uc_malloc(bsize,&bp)) >= 0) {
	    bdp->bdata = bp ;
	    bdp->bsize = bsize ;
	    bdp->boff = boff ;
	    bdp->blen = 0 ;
	}

	return rs ;
}
/* end subroutine (bfilebd_start) */


int bfilebd_finish(BDESC *bdp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bdp->bdata != NULL) {
	    rs1 = uc_free(bdp->bdata) ;
	    if (rs >= 0) rs = rs1 ;
	    bdp->bdata = NULL ;
	    bdp->bsize = 0 ;
	    bdp->boff = 0 ;
	    bdp->blen = 0 ;
	    bdp->di = 0 ;
	    bdp->dlen = 0 ;
	    bdp->f.dirty = FALSE ;
	}

	return rs ;
}
/* end subroutine (bfilebd_finish) */


static int bfilebd_check(BDESC *bdp,bfile *fp,offset_t fo,int blen)
{
	int		rs ;
	if (bdp->bdata == NULL) {
	    rs = bfilebd_start(bdp,fp->bsize,fo) ;
	} else {
	    rs = bfilebd_invalidate(bdp,fp,fo,blen) ;
	}
	return rs ;
}
/* end subroutine (bfilebd_check) */


int bfilebd_flush(BDESC *bdp,bfile *fp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if ((bdp->bdata != NULL) && (bdp->dlen > 0)) {
	    offset_t	doff = (bdp->boff + bdp->di) ;
	    const int	dlen = bdp->dlen ;
	    const char	*ddata = (bdp->bdata + bdp->di) ;
	    if (fp->f.notseek) {
		rs = u_write(fp->fd,ddata,dlen) ;
	        wlen += rs ;
	    } else {
		rs = u_pwrite(fp->fd,ddata,dlen,doff) ;
	        wlen += rs ;
	    }
	    bdp->dlen = 0 ;
	    bdp->f.dirty = FALSE ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bfilebd_flush) */


int bfilebd_read(bdp,fp,uoff,ubuf,ulen,to,opts)
BFILE_BD	*bdp ;
bfile		*fp ;
offset_t	uoff ;
void		*ubuf ;
int		ulen ;
int		to ;
int		opts ;
{
	int		rs = SR_OK ;
	int		bl = 0 ;

	if ((bdp->bdata != NULL) && (bdp->blen > 0)) {
	    offset_t	boff = bdp->boff ;
	    int		bsize = bdp->bsize ;
	    int		blen = bdp->blen ;
	    if ((uoff >= boff) && (uoff < (boff+bsize))) {
		const int	fd = fp->fd ;
		int	bi = (uoff-boff) ;
		int	alen ;
		char	*bp ;
		if (blen > bi) {
		    bp = (bdp->bdata+bi) ;
		    alen = (bdp->blen - bi) ;
		    bl = MIN(alen,ulen) ;
		} else {
		    int		rlen = (bdp->bsize - bdp->blen) ;
		    int		len ;
		    bp = (bdp->bdata+blen) ;
		    if (fp->f.slow) {
	     		rs = uc_reade(fd,bp,rlen,to,opts) ;
	                len = rs ;
		    } else {
			if (fp->f.notseek) {
	                    rs = u_read(fd,bp,rlen) ;
	                    len = rs ;
			} else {
	                    rs = u_pread(fd,bp,rlen,(boff+blen)) ;
	                    len = rs ;
			}
		    }
		    if (rs >= 0) {
		        bp = (bdp->bdata+bi) ;
		        bdp->blen += len ;
		        alen = (bdp->blen - bi) ;
		        bl = MIN(alen,ulen) ;
		    }
		}
		if ((rs >= 0) && (bl > 0))
		    memcpy(ubuf,bp,bl) ;
	    }
	}

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bfilebd_read) */


static int bfilebd_overlap(BFILE_BD *bdp,offset_t uoff,int ulen)
{
	offset_t	boff = bdp->boff ;
	size_t		bsize = bdp->bsize ;
	int		rs = SR_OK ;
	int		bl = 0 ;

	if (uoff < boff) {
	    if ((uoff+ulen) > boff) {
		offset_t	o = MIN((uoff+ulen),(boff+bsize)) ;
		bl = (o-boff) ;
	    }
	} else if (uoff < (boff+bsize)) {
	    offset_t	o = MIN((uoff+ulen),(boff+bsize)) ;
	    bl = (o-boff) ;
	}

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bfilebd_overlap) */


static int bfilebd_invalidate(BFILE_BD *bdp,bfile *fp,offset_t uoff,int ulen)
{
	int		rs = SR_OK ;
	if (ulen != 0) {
	    if ((rs = bfilebd_flush(bdp,fp)) >= 0) {
	        offset_t	boff = bdp->boff ;
	        if (uoff < boff) {
	            bdp->blen = 0 ;
	        } else {
	            bdp->blen = (uoff-boff) ;
	        }
	    } /* end if */
	} /* end if */
	return rs ;
}
/* end subroutine (bfilebd_invalidate) */


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

	if (fd < 0)
	   return SR_BADF ;

	if (sbp == NULL)
	   return SR_FAULT ;

	rs = u_fstat(fd,sbp) ;

	return rs ;
}
/* end subroutine (bfilefstat) */


