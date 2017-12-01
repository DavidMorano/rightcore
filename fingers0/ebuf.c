/* ebuf */

/* manage entry buffering of a file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safer? */


/* revision history:

	= 2003-10-22, David A­D­ Morano
        This code module is an attempt to make a better entry-buffer mamager
        than was used in the old (quite old) SRVREG and MSGID objects.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module manages the entry-buffering of a read-write file database.
        We ignore what the entries actually are and just (really) cache them.
        Our purpose is to try to avoid (or minimize) excess accesses to the
        actual underlying database file.


*******************************************************************************/


#define	EBUF_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ebuf.h"


/* local defines */

#define	WAY	struct ebuf_way


/* external subroutines */

extern uint	hashelf(const void *,int) ;

extern int	matstr(const char **,const char *,int) ;
extern int	ifloor(int,int) ;
extern int	isfsremote(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

struct oldentry {
	time_t		utime ;
	int		ei ;
} ;


/* forward references */

static int	ebuf_waybegin(EBUF *,int) ;
static int	ebuf_wayend(EBUF *,int) ;
static int	ebuf_search(EBUF *,int,char **) ;
static int	ebuf_get(EBUF *,int,int,char **) ;
static int	ebuf_load(EBUF *,int,char **) ;
static int	ebuf_wayfindfin(EBUF *) ;
static int	ebuf_wayfindevict(EBUF *) ;
static int	ebuf_wayevict(EBUF *,int) ;
static int	ebuf_wayloadread(EBUF *,int,int,char **) ;


/* local variables */


/* exported subroutines */


int ebuf_start(EBUF *op,int fd,uint soff,int esize,int nways,int npw)
{
	struct ustat	sb ;
	int		rs ;

	if (nways < 0) nways = 1 ;

	if (npw < EBUF_NENTS) npw = EBUF_NENTS ;

	memset(op,0,sizeof(EBUF)) ;
	op->fd = fd ;
	op->soff = soff ;
	op->esize = esize ;
	op->nways = nways ;
	op->iways = 0 ;
	op->npw = npw ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    int		size ;
	    void	*p ;
	    if (sb.st_size > soff) {
	        op->nentries = ((sb.st_size - soff) / esize) ;
	    }
	    size = nways * sizeof(WAY) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		op->ways = p ;
		memset(op->ways,0,size) ;
		op->magic = EBUF_MAGIC ;
	    } /* end if (m-a) */
	} /* end if (fstat) */

	return rs ;
}
/* end subroutine (ebuf_start) */


int ebuf_finish(EBUF *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; i < op->nways ; i += 1) {
	    rs1 = ebuf_wayend(op,i) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	if (op->ways != NULL) {
	    rs1 = uc_free(op->ways) ;
	    if (rs >= 0) rs = rs1 ;
	    op->ways = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (ebuf_finish) */


int ebuf_read(EBUF *op,int ei,char **rpp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		n = 1 ;

	if ((rs = ebuf_search(op,ei,rpp)) == rsn) {
	    rs = ebuf_load(op,ei,rpp) ;
	    n = rs ;
	}

#if	CF_DEBUGS
	debugprintf("ebuf_read: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ebuf_read) */


/* update the file buffer from a user supplied source */
int ebuf_write(EBUF *op,int ei,const void *ebuf)
{
	offset_t	poff ;
	int		rs = SR_OK ;
	char		*rp = NULL ;

#if	CF_DEBUGS
	debugprintf("ebuf_write: ei=%d op=%s\n",ei,
	    ((ebuf != NULL) ? "write" : "sync")) ;
#endif

	if (ei >= 0) {
	    if ((rs = ebuf_search(op,ei,&rp)) >= 0) {
	        if (ebuf != NULL) {
	            memcpy(rp,ebuf,op->esize) ;
	        }
	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	        if (ebuf != NULL) {
	            rp = (char *) ebuf ;
	        }
	    }

	if (rs >= 0) {

	    if (ei < 0) ei = op->nentries ;

	    if (rp != NULL) {

	        poff = op->soff + (ei * op->esize) ;

#if	CF_DEBUGS
	debugprintf("ebuf_write: poff=%llu\n",poff) ;
#endif

	        rs = u_pwrite(op->fd,rp,op->esize,poff) ;

#if	CF_DEBUGS
	debugprintf("ebuf_write: u_pwrite() rs=%d \n",rs) ;
#endif

	        if (rs >= 0) {
		    if (ei >= op->nentries)
		        op->nentries = (ei + 1) ;
	        }

	    } /* end if */

	} /* end if (ok) */

	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("ebuf_write: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ebuf_write) */


int ebuf_invalidate(EBUF *op,int n)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; (rs >= 0) && (i < op->iways) ; i += 1) {
	    rs1 = ebuf_wayevict(op,i) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	if ((rs >= 0) && (n >= 0)) {
	    op->nentries = n ;
	}

	return rs ;
}
/* end subroutine (ebuf_invalidate) */


int ebuf_count(EBUF *op)
{

	return op->nentries ;
}
/* end subroutine (ebuf_count) */


int ebuf_sync(EBUF *op)
{
	int		rs ;

	rs = uc_fsync(op->fd) ;

	return rs ;
}
/* end subroutine (ebuf_count) */


/* private subroutines */


static int ebuf_waybegin(EBUF *op,int wi)
{
	WAY		*wp = (op->ways + wi) ;
	int		rs ;
	int		wsize ;

	wsize = (op->npw * op->esize) ;
	rs = uc_malloc(wsize,&wp->wbuf) ;

	return rs ;
}
/* end subroutine (ebuf_waybegin) */


static int ebuf_wayend(EBUF *op,int wi)
{
	WAY		*wp = (op->ways + wi) ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (wp->wbuf != NULL) {
	    rs1 = uc_free(wp->wbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    wp->wbuf = NULL ;
	}

	memset(wp,0,sizeof(WAY)) ;

	return rs ;
}
/* end subroutine (ebuf_wayend) */


static int ebuf_search(EBUF *op,int ei,char **rpp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_NOTFOUND ;
	int		wi ;

	for (wi = 0 ; wi < op->iways ; wi += 1) {
	    rs = ebuf_get(op,wi,ei,rpp) ;
	    if ((rs >= 0) || (rs != rsn)) break ;
	} /* end if */

	if ((rs < 0) && (rpp != NULL)) {
	    *rpp = NULL ;
	}

	return rs ;
}
/* end subroutine (ebuf_search) */


static int ebuf_get(EBUF *op,int wi,int ei,char **rpp)
{
	WAY		*wp = (op->ways + wi) ;
	uint		roff = 0 ;
	int		rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("ebuf_get: wi=%u ei=%u\n",wi,ei) ;
#endif

	if ((wp->wbuf != NULL) && (wp->wlen > 0)) {
	    uint	eoff = (op->soff + (ei * op->esize)) ;
	    if ((eoff >= wp->woff) && (eoff < (wp->woff + wp->wlen))) {
		rs = SR_OK ;
		wp->utime = ++op->utimer ;
		roff = eoff - wp->woff ;
	    }
	} /* end if */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? (wp->wbuf + roff) : NULL ;
	}

	return rs ;
}
/* end subroutine (ebuf_get) */


static int ebuf_load(EBUF *op,int ei,char **rpp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		wi ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("ebuf_load: ent ei=%d\n",ei) ;
#endif

	if ((rs = ebuf_wayfindfin(op)) >= 0) {
	    wi = rs ;
	} else if (rs == rsn) {
	    rs = ebuf_wayfindevict(op) ;
	    wi = rs ;
	} /* end if */

	if (rs >= 0) {
	    rs = ebuf_wayloadread(op,wi,ei,rpp) ;
	    n = rs ;
	}

#if	CF_DEBUGS
	debugprintf("ebuf_load: _wayloadread() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("ebuf_load: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ebuf_load) */


static int ebuf_wayfindfin(EBUF *op)
{
	WAY		*wp ;
	int		rs = SR_OK ;
	int		wi ;

	for (wi = 0 ; wi < op->iways ; wi += 1) {
	    wp = (op->ways + wi) ;
	    if (wp->wlen == 0) break ;
	} /* end if */

	if (wi >= op->iways) {

/* look for un-initialized and not used once yet */

	    if (op->iways >= op->nways) {

/* look for un-initialized anywhere (if this even ever occurs) */

	        for (wi = 0 ; wi < op->iways ; wi += 1) {
	            wp = (op->ways + wi) ;
	            if (wp->wbuf == NULL) break ;
	        } /* end if */

	        if (wi >= op->iways) {
	            rs = SR_NOTFOUND ;
	        }

	    } else {
	        wi = op->iways ;
	    }

	} /* end if */

	return (rs >= 0) ? wi : rs ;
}
/* end subroutine (ebuf_wayfindfin) */


static int ebuf_wayfindevict(EBUF *op)
{
	WAY		*wp ;
	uint		otime = INT_MAX ;
	int		rs = SR_OK ;
	int		i ;
	int		wi = 0 ;

	for (i = 0 ; i < op->nways ; i += 1) {
	    wp = (op->ways + i) ;
	    if (wp->utime < otime) {
		wi = i ;
		otime = wp->utime ;
	    }
	} /* end for */

	wp = (op->ways + wi) ;
	if (wp->wlen != 0) {
	    rs = ebuf_wayevict(op,wi) ;
	}

	return (rs >= 0) ? wi : rs ;
}
/* end subroutine (ebuf_wayfindevict) */


static int ebuf_wayevict(EBUF *op,int wi)
{
	WAY		*wp = (op->ways + wi) ;
	int		rs = SR_OK ;

	wp->woff = 0 ;
	wp->wlen = 0 ;
	wp->nvalid = 0 ;
	wp->utime = 0 ;

	return rs ;
}
/* end subroutine (ebuf_wayevict) */


static int ebuf_wayloadread(EBUF *op,int wi,int ei,char **rpp)
{
	WAY		*wp = (op->ways + wi) ;
	offset_t	poff ;
	uint		woff ;
	int		rs = SR_OK ;
	int		wsize = (op->npw * op->esize) ;
	int		len ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("ebuf_wayloadread: ent wi=%d ei=%d\n",wi,ei) ;
#endif

	if (wp->wbuf == NULL) {
	    rs = ebuf_waybegin(op,wi) ;
	}

	if (rs >= 0) {
	    woff = op->soff + (ei * op->esize) ;
	    poff = woff ;
	    rs = u_pread(op->fd,wp->wbuf,wsize,poff) ;
	    len = rs ;

#if	CF_DEBUGS
	debugprintf("ebuf_wayloadread: u_pread() rs=%d \n",rs) ;
#endif

	    if ((rs >= 0) && (len > 0)) {
		n = (len / op->esize) ;
		wp->woff = woff ;
		wp->wlen = ifloor(len,op->esize) ;
		wp->utime = ++op->utimer ;
		wp->nvalid = n ;
		if (op->iways < op->nways) op->iways += 1 ;
	    }
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? wp->wbuf : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("ebuf_wayloadread: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ebuf_wayloadread) */


