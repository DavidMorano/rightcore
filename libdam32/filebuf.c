/* filebuf */

/* support low-overhead file bufferring requirements */


#define	CF_DEBUGS	0		/* debug print-outs */
#define	CF_NULTERM	0		/* NUL terminate read-string */


/* revision history:

	= 2002-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I/O requirements.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<format.h>
#include	<localmisc.h>

#include	"filebuf.h"


/* local defines */

#define	FILEBUF_RCNET	3		/* read-count for network */

#define	MEMCPYLEN	100

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */

extern LONG	llceil(LONG,int) ;

extern int	snopenflags(char *,int,int) ;
extern int	ifloor(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	filebuf_bufcpy(FILEBUF *,const char *,int) ;


/* local variables */


/* exported subroutines */


int filebuf_start(FILEBUF *op,int fd,offset_t coff,int bufsize,int of)
{
	int		rs = SR_OK ;
	char		*p ;

	if (op == NULL) return SR_FAULT ;

	if (fd < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("filebuf_start: fd=%u\n",fd) ;
	debugprintf("filebuf_start: coff=%llu\n",coff) ;
	debugprintf("filebuf_start: bufsize=%u\n",bufsize) ;
	{
	    char	ofbuf[MAXNAMELEN+1] ;
	    snopenflags(ofbuf,MAXNAMELEN,of) ;
	    debugprintf("filebuf_start: of={%s}\n",ofbuf) ;
	}
#endif

	memset(op,0,sizeof(FILEBUF)) ;

	if (bufsize <= 0) {
	    struct ustat	sb ;
	    rs = u_fstat(fd,&sb) ;
	    if (S_ISFIFO(sb.st_mode)) {
	        bufsize = 1024 ;
	    } else {
		LONG	ps = getpagesize() ;
		if ((of & O_ACCMODE) == O_RDONLY) {
		    LONG	fs = ((sb.st_size == 0) ? 1 : sb.st_size) ;
		    LONG	cs ;
		    cs = BCEIL(fs,512) ;
	            bufsize = (int) MIN(ps,cs) ;
	        } else
		    bufsize = ps ;
	    }
	} /* end if (bufsize) */

	if (rs >= 0) {
	    op->bufsize = bufsize ;
	    if ((rs = uc_valloc(bufsize,&p)) >= 0) {
	        op->buf = p ;
	        op->bp = p ;
	        op->fd = fd ;
	        if (coff < 0) rs = u_tell(fd,&coff) ;
	        if (rs >= 0) {
		    op->off = coff ;
		    if (of & FILEBUF_ONET) op->f.net = TRUE ;
	        }
	        if (rs < 0) {
		    uc_free(op->buf) ;
		    op->buf = NULL ;
	        }	
	    } /* end if (memory-allocation) */
	} /* end if */

	return (rs >= 0) ? bufsize : rs ;
}
/* end subroutine (filebuf_start) */


int filebuf_finish(FILEBUF *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->f.write && (op->len > 0)) {
	    rs1 = uc_writen(op->fd,op->buf,op->len) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->buf != NULL) {
	    rs1 = uc_free(op->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->buf = NULL ;
	}

	op->len = 0 ;
	return rs ;
}
/* end subroutine (filebuf_finish) */


int filebuf_read(FILEBUF *op,void *rbuf,int rlen,int to)
{
	const int	fmo = FM_TIMED ;
	int		rs = SR_OK ;
	int		mlen ;
	int		rc ;
	int		tlen = 0 ;
	int		f_timedout = FALSE ;
	char		*dbp = (char *) rbuf ;
	char		*bp, *lastp ;

	if (op == NULL) return SR_FAULT ;

	rc = (op->f.net) ? FILEBUF_RCNET : 1 ;
	while (tlen < rlen) {

	    while ((op->len <= 0) && (rc-- > 0)) {

	        op->bp = op->buf ;
		if (to >= 0) {
	            rs = uc_reade(op->fd,op->buf,op->bufsize,to,fmo) ;
		} else {
	            rs = u_read(op->fd,op->buf,op->bufsize) ;
		}

	        if ((rs == SR_TIMEDOUT) && (tlen > 0)) {
	            f_timedout = TRUE ;
	            rs = SR_OK ;
	            break ;
	        }

	        if (rs < 0) break ;

	        op->len = rs ;
	    } /* end while (refill) */

	    if ((op->len == 0) || f_timedout)
	        break ;

	    mlen = MIN(op->len,(rlen - tlen)) ;

	    bp = op->bp ;
	    lastp = op->bp + mlen ;
	    while (bp < lastp) {
	        *dbp++ = *bp++ ;
	    }

	    op->bp += mlen ;
	    tlen += mlen ;
	    op->len -= mlen ;

	} /* end while */

	if (rs >= 0)
	    op->off += tlen ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (filebuf_read) */


#ifdef	COMMENT

int filebuf_readp(op,buf,buflen,off,to)
FILEBUF		*op ;
void		*buf ;
int		buflen ;
offset_t	off ;
int		to ;
{
	int		rs = SR_NOSYS ;
	int		tlen = 0 ;

	return (rs >= 0) ? tlen : SR_NOSYS ;
}
/* end subroutine (filebuf_readp) */

#endif /* COMMENT */


int filebuf_readline(FILEBUF *op,char *rbuf,int rlen,int to)
{
	const int	fmo = FM_TIMED ;
	int		rs = SR_OK ;
	int		i, mlen ;
	int		rc ;		/* retry-count */
	int		tlen = 0 ;
	int		f_timedout = FALSE ;
	char		*rbp = rbuf ;
	char		*bp, *lastp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("filebuf_readline: f_net=%u rlen=%d to=%d\n",
		op->f.net,rlen,to) ;
#endif

	rc = (op->f.net) ? FILEBUF_RCNET : 1 ;
	while (tlen < rlen) {

	    while ((op->len == 0) && (rc-- > 0)) {

	        op->bp = op->buf ;
		if (to >= 0) {
	            rs = uc_reade(op->fd,op->buf,op->bufsize,to,fmo) ;
		} else {
	            rs = u_read(op->fd,op->buf,op->bufsize) ;
		}

#if	CF_DEBUGS
		debugprintf("filebuf_readline: u_read() rs=%d eol=%u\n",
		rs,(op->buf[rs-1]=='\n')) ;
		debugprintf("filebuf_readline: l=>%t<\n",
		op->buf,strlinelen(op->buf,rs,40)) ;
#endif

	        if ((rs == SR_TIMEDOUT) && (tlen > 0)) {
	            f_timedout = TRUE ;
	            rs = SR_OK ;
	            break ;
	        }

	        if (rs < 0) break ;

	        op->len = rs ;
	    } /* end while (refilling up buffer) */

#if	CF_DEBUGS
	debugprintf("filebuf_readline: fill-buf rs=%d len=%d f_timedout=%u\n",
		rs,op->len,f_timedout) ;
#endif

	    if (rs < 0) break ;

	    if ((op->len == 0) || f_timedout) break ;

	    mlen = MIN(op->len,(rlen - tlen)) ;

	    bp = op->bp ;
	    lastp = op->bp + mlen ;
	    while (bp < lastp) {
	        if ((*rbp++ = *bp++) == '\n') break ;
	    } /* end while */

	    i = bp - op->bp ;

#if	CF_DEBUGS
	    debugprintf("filebuf_readline: i=%d\n",i) ;
#endif

	    op->bp += i ;
	    tlen += i ;
	    op->len -= i ;
	    if ((i > 0) && (rbp[-1] == '\n'))
	        break ;

#if	CF_DEBUGS
	    debugprintf("filebuf_readline: bottom while\n") ;
#endif

	} /* end while (trying to satisfy request) */

	if (rs >= 0) {
#if	CF_NULTERM
	    *rbp = '\0' ;
#endif
	    op->off += tlen ;
	}

#if	CF_DEBUGS
	debugprintf("filebuf_readline: ret rs=%d tlen=%u\n",rs,tlen) ;
	debugprintf("filebuf_readline: ret roff=%llu\n",op->off) ;
	debugprintf("filebuf_readline: ret rbuf=>%t<\n",
		rbuf,strlinelen(rbuf,tlen,40)) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (filebuf_readline) */


int filebuf_readlines(FILEBUF *fp,char lbuf[],int llen,int to,int *lcp)
{
	int		rs = SR_OK ;
	int		alen ;			/* "add" length */
	int		i = 0 ;
	int		f_first = TRUE ;
	int		f_cont = FALSE ;

	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (lcp != NULL) *lcp = 0 ;

	lbuf[0] = '\0' ;
	while (f_first || (f_cont = ISCONT(lbuf,i))) {

	    f_first = FALSE ;
	    if (f_cont)
	        i -= 2 ;

	    alen = (llen - i) ;
	    rs = filebuf_readline(fp,(lbuf + i),alen,to) ;
	    if (rs <= 0) break ;
	    i += rs ;

	    if (lcp != NULL)
	        *lcp += 1 ;

	} /* end while */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (filebuf_readlines) */


/* update a section of the buffer */
int filebuf_update(FILEBUF *op,offset_t roff,cchar *rbuf,int rlen)
{
	uint		boff, bext ;
	uint		rext = roff + rlen ;
	int		bdiff ;
	int		f_exit = FALSE ;

#ifdef	SUSPECT
	buflen = (op->bp - op->buf) + op->len ;
#endif

	boff = op->off - (op->bp - op->buf) ;
	bext = op->off + op->len ;

	if (roff < boff) {
	    if (rext > boff) {
	        rbuf += (boff - roff) ;
	        rlen -= (boff - roff) ;
	        roff = boff ;
	    } else {
		f_exit = TRUE ;
		rlen = 0 ;
	    }
	}

	if ((! f_exit) && (rext > bext)) {
	    if (roff < bext) {
	        rlen -= (rext - bext) ;
	        rext = bext ;
	    } else {
		f_exit = TRUE ;
		rlen = 0 ;
	    }
	}

	if ((! f_exit) && (rlen > 0)) {
	    bdiff = roff - boff ;
	    memcpy((op->buf + bdiff),rbuf,rlen) ;
	}

	return rlen ;
}
/* end subroutine (filebuf_update) */


int filebuf_write(FILEBUF *op,const void *abuf,int alen)
{
	int		rs = SR_OK ;
	int		alenr ;
	int		blenr ;
	int		mlen ;
	int		len ;
	const char	*abp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("filebuf_write: alen=%d \n",alen) ;
#endif

	op->f.write = TRUE ;
	abp = (const char *) abuf ;
	if (alen < 0)
	    alen = strlen(abp) ;

	alenr = alen ;
	while ((rs >= 0) && (alenr > 0)) {

	    if ((rs >= 0) && (op->len == 0) && (alenr >= op->bufsize)) {

	        mlen = ifloor(alenr,op->bufsize) ;
	        rs = uc_writen(op->fd,abp,mlen) ;
	        len = rs ;

	        if (rs >= 0) {
	            abp += len ;
	            alenr -= len ;
	        }

	    } /* end if */

	    if ((rs >= 0) && (alenr > 0)) {

	        blenr = op->bufsize - op->len ;
	        mlen = MIN(alenr,blenr) ;
	        filebuf_bufcpy(op,abp,mlen) ;
	        len = mlen ;

	        op->len += len ;
	        abp += len ;
	        alenr -= len ;

	        if (op->len == op->bufsize) {
	            rs = filebuf_flush(op) ;
		}

	    } /* end if */

	} /* end while */

	if (rs >= 0) {
	    op->off += alen ;
	}

#if	CF_DEBUGS
	debugprintf("filebuf_write: ret rs=%d alen=%u\n",rs,alen) ;
#endif

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (filebuf_write) */


int filebuf_reserve(FILEBUF *op,int len)
{
	int		rs = SR_OK ;

	if (op->f.write && (len > 0)) {
	    if (len > (op->bufsize - op->len)) {
		rs = filebuf_flush(op) ;
	    }
	}

	return rs ;
}
/* end subroutine (filebuf_reserve) */


int filebuf_print(FILEBUF *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		reslen ;
	int		wlen = 0 ;
	int		f_needeol ;

	if (op == NULL) return SR_FAULT ;

	sl = strnlen(sp,sl) ;

	f_needeol = ((sl == 0) || (sp[sl-1] != '\n')) ;
	reslen = (f_needeol) ? (sl+1) : sl ;

	if (reslen > 1) {
	    rs = filebuf_reserve(op,reslen) ;
	}

	if ((rs >= 0) && (sl > 0)) {
	    rs = filebuf_write(op,sp,sl) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && f_needeol) {
	    char	buf[2] ;
	    buf[0] = '\n' ;
	    buf[1] = '\0' ;
	    rs = filebuf_write(op,buf,1) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_print) */


int filebuf_printf(FILEBUF *op,cchar *fmt,...)
{
	const int	dlen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		len ;
	char		dbuf[LINEBUFLEN+1] ;

	if (op == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = format(dbuf,dlen,0,fmt,ap) ;
	    len = rs ;
	    va_end(ap) ;
	}

	if (rs >= 0) {
	    rs = filebuf_write(op,dbuf,len) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_printf) */


int filebuf_adv(FILEBUF *op,int inc)
{
	int		rs = SR_OK ;
	int		ml ;

	if (op == NULL) return SR_FAULT ;

	if (inc < 0) return SR_INVALID ;

	if (inc > 0) {
	if (op->f.write) {
	    rs = filebuf_flush(op) ;
	} else {
	    ml = MIN(inc,op->len) ;
	    if (ml > 0) {
	        inc -= ml ;
	        op->len -= ml ;
	        op->bp += ml ;
	        op->off += ml ;
	    }
	} /* end if (reading) */
	if ((rs >= 0) && (inc > 0)) {
		op->off += inc ;
		rs = u_seek(op->fd,inc,SEEK_CUR) ;
	} /* end if */
	} /* end if (positive) */

	return rs ;
}
/* end subroutine (filebuf_adv) */


int filebuf_seek(FILEBUF *op,offset_t woff,int w)
{
	offset_t	noff, aoff ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (! op->f.net) {

	if (op->f.write) { /* write */
	    if (op->len > 0) {
	        rs = filebuf_flush(op) ;
	    }
	    if (rs >= 0) {
	        rs = u_seeko(op->fd,woff,w,&noff) ;
	        op->off = noff ;
	    }
	} else { /* read */

#if	CF_DEBUGS
	{
	offset_t	coff ;
	u_tell(op->fd,&coff) ;
	debugprintf("filebuf_seek: before local off=%llu\n",op->off) ;
	debugprintf("filebuf_seek: before u_tell() coff=%llu\n",coff) ;
	}
#endif /* CF_DEBUGS */

	    aoff = 0 ;
	    switch (w) {
	    case SEEK_CUR:
	        aoff = (- op->len) ;
	        break ;
	    case SEEK_SET:
	    case SEEK_END:
	        break ;
	    default:
	        rs = SR_INVALID ;
		break ;
	    } /* end switch */

	    if (rs >= 0) {
	        rs = u_seeko(op->fd,(woff + aoff),w,&noff) ;
	        op->off = noff ;
	    }

	    op->bp = op->buf ;
	    op->len = 0 ;

#if	CF_DEBUGS
	{
	offset_t	coff ;
	u_tell(op->fd,&coff) ;
	debugprintf("filebuf_seek: after local off=%llu\n",op->off) ;
	debugprintf("filebuf_seek: after u_tell() coff=%lldun",coff) ;
	}
#endif /* CF_DEBUGS */

	} /* end if (write or read)  */

#ifdef	COMMENT
	if (! op->f.write) {
	    offset_t	doff = (noff - op->off) ;

	    if ((doff > 0) && (doff < op->len)) {
	        op->bp += doff ;
	        op->len -= doff ;
	    } else {
	        op->bp = op->buf ;
	        op->len = 0 ;
	    }

	} /* end if (not reading) */
#endif /* COMMENT */

	} else {
	    rs = SR_NOTSEEK ;
	}

	return rs ;
}
/* end subroutine (filebuf_seek) */


int filebuf_tell(FILEBUF *op,offset_t *offp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("filebuf_tell: off=%llu\n",op->off) ;
#endif

	if (offp != NULL)
	    *offp = op->off ;

	rs = (op->off & INT_MAX) ;
	return rs ;
}
/* end subroutine (filebuf_tell) */


int filebuf_invalidate(FILEBUF *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if ((! op->f.write) && (op->len > 0)) {
	    if (! op->f.net)
	        rs = u_seek(op->fd,op->off,SEEK_SET) ;
	}

	op->len = 0 ;
	op->bp = op->buf ;
	return rs ;
}
/* end subroutine (filebuf_invalidate) */


int filebuf_flush(FILEBUF *op)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->f.write && (op->len > 0)) {

	    rs = uc_writen(op->fd,op->buf,op->len) ;
	    len = rs ;

	    op->bp = op->buf ;
	    op->len = 0 ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_flush) */


int filebuf_poll(FILEBUF *op,int mto)
{
	struct pollfd	fds[1] ;
	const int	nfds = 1 ;
	int		rs = SR_OK ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

	size = nfds * sizeof(struct pollfd) ;
	memset(fds,0,size) ;
	fds[0].fd = op->fd ;
	fds[0].events = 0 ;
	fds[0].revents = 0 ;

	rs = u_poll(fds,nfds,mto) ;

	return rs ;
}
/* end subroutine (filebuf_poll) */


/* private subroutines */


static int filebuf_bufcpy(FILEBUF *op,cchar *abp,int mlen)
{

	if (mlen > MEMCPYLEN) {
	    memcpy(op->bp,abp,mlen) ;
	} else {
	    int		i ;
	    char	*bp = op->bp ;
	    for (i = 0 ; i < mlen ; i += 1) {
	        *bp++ = *abp++ ;
	    }
	} /* end if */

	op->bp += mlen ;
	return mlen ;
}
/* end subroutine (filebuf_bufcpy) */


