/* fmq */

/* File Message Queue (FMQ) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safer */
#define	CF_ALWAYSCREATE	0		/* always create file */
#define	CF_SENDCREATE	0		/* sender creates also */
#define	CF_SIGFILLSET	1		/* signal mask on interrupt */


/* revision history:

	= 1999-07-23, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements a file message queue facility.

	Enjoy!


*******************************************************************************/


#define	FMQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"fmq.h"


/* local defines */

#define	FMQ_OPENTIME	30	/* seconds */

#define	FMQ_IDLEN	(16 + 4)
#define	FMQ_HEADLEN	(8 * 4)
#define	FMQ_TOPLEN	(FMQ_IDLEN + FMQ_HEADLEN)

#define	FMQ_IDOFF	0
#define	FMQ_HEADOFF	FMQ_IDLEN
#define	FMQ_BUFOFF	(FMQ_HEADOFF + FMQ_HEADLEN)

#define	FMQ_BUFSIZE	(64 * 1024)
#define	FMQ_RBUFSIZE	((1 * 1024) + 4)
#define	FMQ_WBUFSIZE	((1 * 1024) + 4)

#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		10		/* seconds */

#define	TI_MINUPDATE	4		/* minimum time between updates */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NENTRIES	100
#define	FBUFLEN		(FMQ_TOPLEN + 9)

#ifndef	ENDIAN
#if	defined(OSNAME_SunOS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif


/* external subroutines */

extern uint	uceil(uint,uint) ;

extern int	matstr(const char **,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	msleep(uint) ;
extern int	isfsremote(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

int		fmq_close(FMQ *) ;
int		fmq_sende(FMQ *,const void *,int,int,int) ;
int		fmq_recve(FMQ *,void *,int,int,int) ;

static int	fmq_isend(FMQ *,const void *,int,int) ;
static int	fmq_irecv(FMQ *,void *,int,int) ;
static int	fmq_filecheck(FMQ *,time_t,int,int) ;
static int	fmq_fileinit(FMQ *,time_t) ;
static int	fmq_filechanged(FMQ *) ;
static int	fmq_lockget(FMQ *,time_t,int) ;
static int	fmq_lockrelease(FMQ *) ;
static int	fmq_fileopen(FMQ *,time_t) ;
static int	fmq_fileclose(FMQ *) ;
static int	fmq_bufinit(FMQ *) ;
static int	fmq_buffree(FMQ *) ;
static int	fmq_headwrite(FMQ *) ;
static int	fmq_di(FMQ *,sigset_t *) ;
static int	fmq_ei(FMQ *,sigset_t *) ;

#ifdef	COMMENT
static int	fmq_headread(FMQ *) ;
#endif

static int	filemagic(char *,int,FMQ_FM *) ;
static int	filehead(char *,int,FMQ_FH *) ;

#if	CF_DEBUGS
static int	debugprintstat(const char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int fmq_open(FMQ *op,cchar *fname,int oflags,mode_t operm,int bufsize)
{
	struct ustat	sb ;
	time_t		dt = time(NULL) ;
	int		rs ;
	int		amode ;
	int		f_create = FALSE ;

#if	CF_DEBUGS
	debugprintf("fmq_open: ent fname=%s, bufsize=%d\n",
		fname,bufsize) ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	if (bufsize < 8)
	    bufsize = FMQ_BUFSIZE ;
#else
	if (bufsize < FMQ_BUFSIZE)
	    bufsize = FMQ_BUFSIZE ;
#endif

#if	CF_DEBUGS
	debugprintf("fmq_open: oflags=%08x operm=%06o bufsize=%u\n",
	    oflags,operm,bufsize) ;
	if (oflags & O_CREAT)
	    debugprintf("fmq_open: creating as needed\n") ;
#endif

	memset(op,0,sizeof(FMQ)) ;
	op->magic = 0 ;
	op->fname = NULL ;

#if	CF_ALWAYSCREATE
	oflags |= O_CREAT ;
#endif

	oflags = (oflags & (~ O_TRUNC)) ;

	op->f.create = (oflags & O_CREAT) ? TRUE : FALSE ;
	op->f.ndelay = (oflags & O_NDELAY) ? TRUE : FALSE ;
	op->f.nonblock = (oflags & O_NONBLOCK) ? TRUE : FALSE ;

#if	CF_DEBUGS
	debugprintf("fmq_open: ndelay=%u nonblock=%u\n",
	    op->f.ndelay,op->f.nonblock) ;
#endif

	oflags = (oflags & (~ (O_NDELAY | O_NONBLOCK))) ;

	op->oflags = oflags ;
	op->operm = operm ;

	{
	    const char	*cp ;
	    rs = uc_mallocstrw(fname,-1,&cp) ;
	    if (rs >= 0) op->fname = cp ;
	}
	if (rs < 0) goto bad0 ;

/* initialize the buffer structure */

	rs = fmq_bufinit(op) ;
	if (rs < 0)
	    goto bad1 ;

/* try to open the file */

	oflags = (oflags & (~ O_CREAT)) ;
	rs = u_open(op->fname,oflags,operm) ;

#if	CF_DEBUGS
	debugprintf("fmq_open: u_open() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (op->oflags & O_CREAT)) {

	    f_create = TRUE ;
	    oflags = op->oflags ;
	    rs = u_open(op->fname,oflags,operm) ;

#if	CF_DEBUGS
	    debugprintf("fmq_open: u_open() rs=%d\n",rs) ;
#endif

	} /* end if (creating file) */

	op->fd = rs ;
	if (rs < 0)
	    goto bad2 ;

	amode = (operm & O_ACCMODE) ;
	op->f.writable = ((amode == O_WRONLY) || (amode == O_RDWR)) ;

#if	CF_DEBUGS
	debugprintf("fmq_open: f_writable=%d\n",op->f.writable) ;
#endif

	op->opentime = dt ;
	op->accesstime = dt ;
	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("fmq_open: u_fstat() rs=%d\n",rs) ;
	debugprintf("fmq_open: size=%08x\n",sb.st_size) ;
	debugprintf("fmq_open: mtime=%08x\n",sb.st_mtime) ;
#endif

	if (rs < 0)
	    goto bad3 ;

	op->mtime = sb.st_mtime ;
	op->filesize = sb.st_size ;
	op->pagesize = getpagesize() ;

/* local or remote */

	rs = isfsremote(op->fd) ;
	op->f.remote = (rs > 0) ;
	if (rs < 0)
	    goto bad3 ;

/* determine some operational parameters (size of buffer space) */

	op->bufsize = uceil(bufsize,sizeof(int)) ;

/* setup for disabling signals */

#if	CF_SIGFILLSET
	uc_sigsetfill(&op->sigmask) ;
#else
	uc_sigsetempty(&op->sigmask) ;
#endif

/* header processing */

	rs = fmq_fileinit(op,dt) ;

#if	CF_DEBUGS
	debugprintf("fmq_open: fmq_fileinit() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (rs != SR_AGAIN))
	    goto bad3 ;

	if ((rs == SR_AGAIN) && (! op->f.create))
	    rs = SR_OK ;

/* out of here */

	op->magic = FMQ_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("fmq_open: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_create : rs ;

/* bad things */
bad4:
bad3:
	u_close(op->fd) ;

bad2:
	fmq_buffree(op) ;

bad1:
	if (op->fname != NULL)
	    uc_free(op->fname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (fmq_open) */


int fmq_close(FMQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	rs1 = fmq_buffree(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	rs1 = uc_free(op->fname) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fmq_close) */


/* get a count of the number of entries */
int fmq_count(FMQ *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (fmq_count) */


/* send a message */
int fmq_send(FMQ *op,const void *buf,int buflen)
{
	int		rs ;
	int		to ;
	int		tlen = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("fmq_send: ent buflen=%u\n", buflen) ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

/* continue with normal operation */

	to = -1 ;
	rs = fmq_sende(op,buf,buflen,to,0) ;
	tlen = rs ;

#if	CF_DEBUGS
	debugprintf("fmq_send: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (fmq_send) */


/* send a message */
int fmq_sende(FMQ *op,const void *buf,int buflen,int to,int opts)
{
	ulong		starttime, endtime, dt ;
	int		rs ;
	int		f_infinite = FALSE ;
	int		tlen = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("fmq_sende: ent buflen=%u\n",
	    buflen) ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

/* continue with the real work */

	if (to < 0) {
	    f_infinite = TRUE ;
	    to = INT_MAX ;
	}

#if	CF_DEBUGS
	debugprintf("fmq_sende: before to=%d f_infinite=%u\n",
	    to,f_infinite) ;
#endif

	starttime = time(NULL) ;

	endtime = starttime + to ;
	if (endtime < starttime)
	    endtime = INT_MAX ;

/* CONSTCOND */

	while (TRUE) {

#if	CF_DEBUGS
	    debugprintf("fmq_sende: loop to=%d \n",to) ;
#endif

	    rs = fmq_isend(op,buf,buflen,opts) ;
	    tlen = rs ;
	    if (rs >= 0)
	        break ;

	    if (rs != SR_AGAIN)
	        break ;

	    if (f_infinite && (op->f.ndelay || op->f.nonblock))
	        break ;

	    if (to <= 0)
	        break ;

	    msleep(1000) ;

	    dt = time(NULL) ;

	    if (dt >= endtime) break ;
	} /* end while */

	if ((rs == SR_AGAIN) && op->f.ndelay) {
	    tlen = 0 ;
	    rs = SR_OK ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("fmq_sende: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (fmq_sende) */


/* receive a message */
int fmq_recv(FMQ *op,void *buf,int buflen)
{
	int		rs = SR_OK ;
	int		to ;
	int		tlen = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("fmq_recv: ent buflen=%u\n", buflen) ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

/* continue with normal operation */

	to = -1 ;
	rs = fmq_recve(op,buf,buflen,to,0) ;
	tlen = rs ;

#if	CF_DEBUGS
	debugprintf("fmq_recv: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (fmq_recv) */


/* receive a message */
int fmq_recve(FMQ *op,void *buf,int buflen,int to,int opts)
{
	int		rs = SR_OK ;
	int		tlen = 0 ;
	int		f_infinite = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (buf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("fmq_recve: ent buflen=%u to=%d opts=%04x\n",
	    buflen,to,opts) ;
	debugprintf("fmq_recve: ndelay=%u nonblock=%u\n",
	    op->f.ndelay,op->f.nonblock) ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

/* continue with the real work */

	if (to < 0) {
	    f_infinite = TRUE ;
	    to = INT_MAX ;
	}

#if	CF_DEBUGS
	debugprintf("fmq_recve: before to=%d f_infinite=%u\n",
	    to,f_infinite) ;
#endif

	while (to >= 0) {

#if	CF_DEBUGS
	    debugprintf("fmq_recve: loop to=%d \n",to) ;
#endif

	    rs = fmq_irecv(op,buf,buflen,opts) ;
	    tlen = rs ;
	    if (rs >= 0)
	        break ;

	    if (rs != SR_AGAIN)
	        break ;

	    if (f_infinite && (op->f.ndelay || op->f.nonblock))
	        break ;

	    if (to <= 0)
	        break ;

	    to -= 1 ;
	    sleep(1) ;

	} /* end while */

	if ((rs == SR_AGAIN) && op->f.ndelay) {
	    tlen = 0 ;
	    rs = SR_OK ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("fmq_recve: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (fmq_recve) */


/* do some checking */
int fmq_check(FMQ *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != FMQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->fd >= 0) {
	    if ((! op->f.readlocked) && (! op->f.writelocked)) {
	        if ((dt - op->accesstime) >= TO_ACCESS) {
	            f = TRUE ;
		    rs = fmq_fileclose(op) ;
		}
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (fmq_check) */


/* private subroutines */


/* send a message */
/* ARGSUSED */
static int fmq_isend(FMQ *op,const void *buf,int buflen,int opts)
{
	struct iovec	v[3] ;
	sigset_t	oldsigmask ;
	offset_t	uoff ;
	time_t		dt = time(NULL) ;
	uint		eoff ;
	uint		llen, dlen, len ;
	int		rs ;
	char		lenbuf[4 + 1] ;
	char		*cbuf = (char *) buf ;

#if	CF_DEBUGS
	debugprintf("fmq_isend: ent buflen=%u\n", buflen) ;
#endif

	fmq_di(op,&oldsigmask) ;

/* do we have proper file access? */

	rs = fmq_filecheck(op,dt,0,0) ;
	if (rs < 0)
	    goto ret1 ;

/* is the file initialized? */

#if	CF_DEBUGS
	debugprintf("fmq_isend: fileinit=%u\n",op->f.fileinit) ;
#endif

	if (! op->f.fileinit) {

#if	CF_SENDCREATE
	    rs = SR_ACCESS ;
	    if (! op->f.create)
	        goto ret1 ;
#endif /* CF_SENDCREATE */

	    if (dt == 0)
	        dt = time(NULL) ;

	    rs = fmq_fileinit(op,dt) ;
	    if (rs < 0)
	        goto ret1 ;

	}

/* prepare the message header (the length) */

	llen = stdorder_wuint(lenbuf,buflen) ;

/* can we even write this message? */

	if ((buflen + llen) > (op->h.size - op->h.blen)) {
	    rs = ((buflen + llen) > op->h.size) ? SR_TOOBIG : SR_AGAIN ;
	    goto ret1 ;
	}

/* prepare to write */

	if (op->h.wi < op->h.ri) {
	    dlen = op->h.ri - op->h.wi ;
	} else {
	    dlen = op->h.size - op->h.wi ;
	}

	dlen -= llen ;
	if (buflen < dlen)
	    dlen = buflen ;

#if	CF_DEBUGS
	debugprintf("fmq_isend: dlen=%u\n",dlen) ;
#endif

/* set up the write buffers */

	v[0].iov_base = lenbuf ;
	v[0].iov_len = llen ;

	v[1].iov_base = (caddr_t) cbuf ;
	v[1].iov_len = dlen ;

	v[2].iov_base = NULL ;
	v[2].iov_len = 0 ;

	eoff = FMQ_BUFOFF + op->h.wi ;
	uoff = eoff ;
	rs = u_seek(op->fd,uoff,SEEK_SET) ;

	if (rs >= 0)
	    rs = u_writev(op->fd,v,2) ;

#if	CF_DEBUGS
	debugprintf("fmq_isend: 1 u_writev() rs=%d off=%lu\n",
	    rs,eoff) ;
#endif

	if ((rs >= 0) && (dlen < buflen)) {

	    v[0].iov_base = (caddr_t) (cbuf + dlen) ;
	    v[0].iov_len = (buflen - dlen) ;

	    v[1].iov_base = NULL ;
	    v[1].iov_len = 0 ;

	    eoff = FMQ_BUFOFF ;
	    uoff = eoff ;
	    if ((rs = u_seek(op->fd,uoff,SEEK_SET)) >= 0) {
	        rs = u_writev(op->fd,v,1) ;
	    }

#if	CF_DEBUGS
	    debugprintf("fmq_isend: 2 u_writev() rs=%d off=%lu\n",
	        rs,eoff) ;
#endif

	} /* end if (wrapped around) */

	if (rs > 0) {

	    len = uceil((llen + buflen),llen) ;

	    op->h.wi = (op->h.wi + len) % op->h.size ;
	    op->h.blen += len ;
	    op->h.len += buflen ;
	    op->h.nmsg += 1 ;

	    if (dt == 0)
	        dt = time(NULL) ;

	    op->h.wcount += 1 ;
	    op->h.wtime = dt ;

	    rs = fmq_headwrite(op) ;

#if	CF_DEBUGS
	    debugprintf("fmq_isend: fmq_headwrite() rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && op->f.remote)
	        u_fsync(op->fd) ;

	} /* end if (data write was successful) */

ret1:
	fmq_lockrelease(op) ;

ret0:
	fmq_ei(op,&oldsigmask) ;

#if	CF_DEBUGS
	debugprintf("fmq_isend: ret rs=%d buflen=%u\n",rs,buflen) ;
#endif

	return (rs >= 0) ? buflen : rs ;
}
/* end subroutine (fmq_isend) */


/* receive a message */
static int fmq_irecv(FMQ *op,void *buf,int buflen,int opts)
{
	struct iovec	v[3] ;
	struct ustat	sb ;
	sigset_t	oldsigmask ;
	offset_t	uoff ;
	time_t		dt = 0 ;
	uint		eoff ;
	uint		llen, dlen, mlen, len ;
	int		rs = SR_OK ;
	int		tlen = 0 ;
	int		f_changed ;
	char		lenbuf[4 + 1] ;
	char		*cbuf = (char *) buf ;

/* are we in optional slow-poll mode? */

	if ((opts & FM_SLOWPOLL) && (op->h.nmsg == 0)) {

#if	CF_DEBUGS
	    debugprintf("fmq_irecv: slowpoll\n") ;
#endif

	    rs = u_fstat(op->fd,&sb) ;
	    if (rs < 0)
	        goto ret0 ;

	    f_changed = (sb.st_size != op->filesize) ||
	        (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	    debugprintf("fmq_irecv: f_changed=%u\n",f_changed) ;
#endif

	    rs = SR_AGAIN ;
	    if (! f_changed)
	        goto ret0 ;

	} /* end if (optional slow-poll mode) */

/* continue with a regular operation */

	fmq_di(op,&oldsigmask) ;

/* do we have proper file access? */

	if (dt == 0)
	    dt = time(NULL) ;

	rs = fmq_filecheck(op,dt,0,opts) ;
	if (rs < 0)
	    goto ret2 ;

/* is the file initialized? */

#if	CF_DEBUGS
	debugprintf("fmq_irecv: fileinit=%u\n",op->f.fileinit) ;
#endif

	if (! op->f.fileinit) {

	    if (dt == 0)
	        dt = time(NULL) ;

	    rs = fmq_fileinit(op,dt) ;
	    if (rs < 0)
	        goto ret2 ;

	}

/* are there any messages to receive? */

	if (op->h.nmsg == 0) {
	    rs = SR_AGAIN ;
	    goto ret2 ;
	}

	llen = sizeof(uint) ;

/* prepare to read */

	if (op->h.ri >= op->h.wi) {
	    dlen = op->h.wi - op->h.ri ;
	} else {
	    dlen = op->h.size - op->h.ri ;
	}

	dlen -= llen ;
	if (buflen < dlen)
	    dlen = buflen ;

#if	CF_DEBUGS
	debugprintf("fmq_irecv: dlen=%u\n",dlen) ;
#endif

/* set up the read buffers */

	v[0].iov_base = lenbuf ;
	v[0].iov_len = llen ;

	v[1].iov_base = (caddr_t) cbuf ;
	v[1].iov_len = dlen ;

	v[2].iov_base = NULL ;
	v[2].iov_len = 0 ;

	eoff = FMQ_BUFOFF + op->h.ri ;
	uoff = eoff ;
	rs = u_seek(op->fd,uoff,SEEK_SET) ;

	if (rs >= 0)
	    rs = u_readv(op->fd,v,2) ;

#if	CF_DEBUGS
	debugprintf("fmq_irecv: 1 u_readv() rs=%d off=%lu\n",rs,eoff) ;
#endif

	if (rs >= llen) {

	    tlen = rs - llen ;
	    stdorder_ruint(lenbuf,&mlen) ;

#if	CF_DEBUGS
	    debugprintf("fmq_irecv: tlen=%u mlen=%u\n",tlen,mlen) ;
#endif

	    if (mlen > tlen) {

	        if (buflen >= mlen) {

	            v[0].iov_base = (caddr_t) (cbuf + tlen) ;
	            v[0].iov_len = (mlen - tlen) ;

	            v[1].iov_base = NULL ;
	            v[1].iov_len = 0 ;

	            eoff = FMQ_BUFOFF ;
		    uoff = eoff ;
	            rs = u_seek(op->fd,uoff,SEEK_SET) ;

	            if (rs >= 0)
	                rs = u_readv(op->fd,v,1) ;

#if	CF_DEBUGS
	            debugprintf("fmq_irecv: 2 u_readv() rs=%d off=%lu\n",
			rs,eoff) ;
#endif

	            if (rs >= 0)
	                tlen += rs ;

#if	CF_DEBUGS
	            debugprintf("fmq_irecv: new tlen=%u\n",tlen) ;
#endif

	        } else
	            rs = SR_OVERFLOW ;

	    } else
	        tlen = mlen ;

	} else
	    rs = SR_BADFMT ;

	if (rs >= 0) {

	    len = uceil((llen + mlen),llen) ;

#if	CF_DEBUGS
	    debugprintf("fmq_irecv: buffer turn len=%u\n",len) ;
#endif

	    op->h.ri = (op->h.ri + len) % op->h.size ;
	    op->h.blen -= len ;
	    op->h.len -= mlen ;
	    op->h.nmsg -= 1 ;

	    if (dt == 0)
	        dt = time(NULL) ;

	    op->h.wcount += 1 ;
	    op->h.wtime = dt ;

	    rs = fmq_headwrite(op) ;

#if	CF_DEBUGS
	    debugprintf("fmq_irecv: fmq_headwrite() rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && op->f.remote)
	        u_fsync(op->fd) ;

	} /* end if (we were able to turn the buffer) */

ret2:
	fmq_lockrelease(op) ;

ret1:
	fmq_ei(op,&oldsigmask) ;

#if	CF_DEBUGS
	debugprintf("fmq_irecv: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (fmq_irecv) */


/* check the file for coherency */
/* ARGSUSED */
static int fmq_filecheck(FMQ *op,time_t dt,int f_read,int opts)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

/* is the file open */

	if (op->fd < 0) {

	    if (dt == 0)
	        dt = time(NULL) ;

	    rs = fmq_fileopen(op,dt) ;
	    if (rs < 0)
	        goto ret0 ;

	}

/* check for optional slow-poll mode */

#ifdef	COMMENT
	if ((opts & FM_SLOWPOLL) && (op->h.nmsg <= 0)) {

#if	CF_DEBUGS
	    debugprintf("fmq_filecheck: slowpoll\n") ;
#endif

	    rs = u_fstat(op->fd,&sb) ;
	    if (rs < 0)
	        goto bad0 ;

	    f_changed = (sb.st_size != op->filesize) ||
	        (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	    debugprintf("fmq_filecheck: f_changed=%u\n",f_changed) ;
#endif

	    if (! f_changed)
	        goto ret0 ;

	} /* end if (optional slow-poll mode) */
#endif /* COMMENT */

/* capture the lock if we do not already have it */

	if ((! op->f.readlocked) && (! op->f.writelocked)) {

	    if (dt == 0)
	        dt = time(NULL) ;

	    rs = fmq_lockget(op,dt,f_read) ;
	    if (rs < 0)
	        goto ret0 ;

	    rs = fmq_filechanged(op) ;
	    if (rs < 0)
	        goto bad1 ;

	    f_changed = (rs > 0) ;

	} /* end if (capture lock) */

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad1:
	fmq_lockrelease(op) ;

bad0:
	return rs ;
}
/* end subroutine (fmq_filecheck) */


/* has the file changed at all? */
static int fmq_filechanged(FMQ *op)
{
	struct ustat	sb ;
	int		rs ;
	int		f_statchanged = FALSE ;
	int		f_headchanged = FALSE ;

/* has the file changed at all? */

	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("fmq_filechanged: u_fstat() rs=%d filesize=%u\n",
	    rs,sb.st_size) ;
#endif

#ifdef	COMMENT
	if (rs == SR_NOENT)
	    rs = SR_OK ;
#endif /* COMMENT */

	if (rs < 0)
	    goto bad2 ;

	if (sb.st_size < FMQ_BUFOFF)
	    op->f.fileinit = FALSE ;

	f_statchanged = (! op->f.fileinit) ||
	    (sb.st_size != op->filesize) ||
	    (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	debugprintf("fmq_filechanged: fileinit=%u\n",op->f.fileinit) ;
	debugprintf("fmq_filechanged: f_size=%08x o_size=%08x\n",
	    sb.st_size,op->filesize) ;
	debugprintf("fmq_filechanged: f_mtime=%08x o_mtime=%08x\n",
	    sb.st_mtime,op->mtime) ;
	debugprintf("fmq_filechanged: f_statchanged=%u\n",f_statchanged) ;
#endif /* CF_DEBUGS */

/* read the file header for write indications */

	if (op->f.fileinit) {
	    FMQ_FH	h ;
	    char	hbuf[FMQ_TOPLEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("fmq_filechanged: file is inited\n") ;
#endif

	    rs = u_pread(op->fd,hbuf,FMQ_TOPLEN,0L) ;

#if	CF_DEBUGS
	    debugprintf("fmq_filechanged: u_pread() rs=%d fd=%d\n",
		rs,op->fd) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	    if (rs < FMQ_TOPLEN)
	        op->f.fileinit = FALSE ;

#if	CF_DEBUGS
	    debugprintf("fmq_filechanged: fileinit=%u\n",op->f.fileinit) ;
#endif

	    if (rs > 0) {

	        filehead((hbuf + FMQ_HEADOFF),1,&h) ;

	        f_headchanged = (op->h.wtime != h.wtime) ||
	            (op->h.wcount != h.wcount) ||
	            (op->h.nmsg != h.nmsg) ;

#if	CF_DEBUGS
	        debugprintf("fmq_filechanged: o_wtime=%08x f_wtime=%08x\n",
	            op->h.wtime,h.wtime) ;
	        debugprintf("fmq_filechanged: o_wcount=%08x f_wcount=%08x\n",
	            op->h.wcount,h.wcount) ;
	        debugprintf("fmq_filechanged: o_nmsg=%u f_nmsg=%u\n",
	            op->h.nmsg,h.nmsg) ;
	        debugprintf("fmq_filechanged: f_headchanged=%u\n",
	            f_headchanged) ;
#endif /* CF_DEBUGS */

	        if (f_headchanged)
	            op->h = h ;

	    }

	} /* end if (reading file header) */

/* OK, we're done */

	if (f_statchanged) {
	    op->f.bufvalid = FALSE ;
	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("fmq_filechanged: ret rs=%d f_headchanged=%u\n",
	    rs,f_headchanged) ;
#endif

	return (rs >= 0) ? f_headchanged : rs ;

/* bad stuff */
bad2:
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (fmq_filechanged) */


static int fmq_bufinit(FMQ *op)
{
	int		rs ;

	op->f.bufvalid = FALSE ;

	memset(&op->b,0,sizeof(struct fmq_bufdesc)) ;

	op->b.size = FMQ_BUFSIZE ;
	rs = uc_malloc(op->b.size,&op->b.buf) ;

	return rs ;
}
/* end subroutine (fmq_bufinit) */


static int fmq_buffree(FMQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->b.buf != NULL) {
	    rs1 = uc_free(op->b.buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->b.buf = NULL ;
	}

	op->b.size = 0 ;
	op->b.len = 0 ;
	op->b.i = 0 ;
	return rs ;
}
/* end subroutine (fmq_buffree) */


/* initialize the file header (either read it only or write it) */
static int fmq_fileinit(FMQ *op,time_t dt)
{
	FMQ_FM	fm ;
	sigset_t	oldsigmask ;
	int		rs = SR_OK ;
	int		bl ;
	int		f_locked = FALSE ;
	char		fbuf[FBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("fmq_fileinit: ent filesize=%u\n",op->filesize) ;
	debugprintstat("fmq_fileinit: ",op->fd) ;
#endif

	fmq_di(op,&oldsigmask) ;

	if (op->filesize == 0) {

#if	CF_DEBUGS
	    debugprintf("fmq_fileinit: writable=%u\n",op->f.writable) ;
#endif

	    u_seek(op->fd,0L,SEEK_SET) ;

	    op->f.fileinit = FALSE ;
	    if (op->f.writable && op->f.create) {

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: want to write header\n") ;
#endif

	        if (! op->f.writelocked) {

	            rs = fmq_lockget(op,dt,0) ;
	            if (rs < 0)
	                goto ret0 ;

	            f_locked = TRUE ;
	        }

/* write the file header stuff */

	        strwcpy(fm.magic,FMQ_FILEMAGIC,14) ;

	        fm.vetu[0] = FMQ_FILEVERSION ;
	        fm.vetu[1] = FMQ_ENDIAN ;
	        fm.vetu[2] = 0 ;
	        fm.vetu[3] = 0 ;

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),0,&fm) ;

	        memset(&op->h,0,sizeof(FMQ_FH)) ;

	        op->h.size = op->bufsize ;

	        bl += filehead((fbuf + bl),0,&op->h) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: u_pwrite() wlen=%d\n",bl) ;
#endif

	        rs = u_pwrite(op->fd,fbuf,bl,0L) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: u_pwrite() rs=%d\n",rs) ;
#endif

	        if (rs > 0) {
	            op->filesize = rs ;
	            op->mtime = dt ;
	            if (op->f.remote) {
	                u_fsync(op->fd) ;
		    }
	        }

	        op->f.fileinit = (rs >= 0) ;

	    } /* end if (writing) */

	} else if (op->filesize >= FMQ_BUFOFF) {
	    int	f ;

#if	CF_DEBUGS
	    debugprintf("fmq_fileinit: non-zero file size=%lu\n",
	        op->filesize) ;
#endif

/* read the file header */

	    if (! op->f.readlocked) {

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: need READ lock\n") ;
#endif

	        rs = fmq_lockget(op,dt,0) ;
	        if (rs < 0)
	            goto ret0 ;

	        f_locked = TRUE ;
	    }

#if	CF_DEBUGS
	    debugprintf("fmq_fileinit: about to read \n") ;
	    debugprintstat("fmq_fileinit: ",op->fd) ;
#endif

	    rs = u_pread(op->fd,fbuf,FBUFLEN,0L) ;

#if	CF_DEBUGS
	    debugprintf("fmq_fileinit: u_pread() rs=%d\n",rs) ;
#endif

	    if (rs >= FMQ_TOPLEN) {

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),1,&fm) ;

	        filehead((fbuf + bl),1,&op->h) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: f_wtime=%08x\n",
	            op->h.wtime) ;
	        debugprintf("fmq_fileinit: f_wcount=%08x\n",
	            op->h.wcount) ;
	        debugprintf("fmq_fileinit: f_nmsg=%u\n",
	            op->h.nmsg) ;
#endif

	        f = (strcmp(fm.magic,FMQ_FILEMAGIC) == 0) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: fm.magic=%s\n",fm.magic) ;
	        debugprintf("fmq_fileinit: magic cmp f=%d\n",f) ;
#endif

	        f = f && (fm.vetu[0] <= FMQ_FILEVERSION) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: version cmp f=%d\n",f) ;
#endif

	        f = f && (fm.vetu[1] == FMQ_ENDIAN) ;

#if	CF_DEBUGS
	        debugprintf("fmq_fileinit: endian cmp f=%d\n",f) ;
#endif

	        if (! f)
	            rs = SR_BADFMT ;

	        op->f.fileinit = f ;

	    } /* end if (read was big enough) */

	} /* end if */

/* if we locked, we unlock it, otherwise leave it ! */

	if (f_locked)
	    fmq_lockrelease(op) ;

/* we're out of here */
ret0:
	fmq_ei(op,&oldsigmask) ;

#if	CF_DEBUGS
	debugprintf("fmq_fileinit: ret rs=%d fileinit=%u\n",
	    rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (fmq_fileinit) */


/* acquire access to the file */
static int fmq_lockget(FMQ *op,time_t dt,int f_read)
{
	int		rs = SR_OK ;
	int		lockcmd ;
	int		f_already = FALSE ;

#if	CF_DEBUGS
	debugprintf("fmq_lockget: ent fd=%d f_read=%d\n",
	    op->fd,f_read) ;
	debugprintstat("fmq_lockget: ",op->fd) ;
#endif

	if (op->fd < 0) {

	    rs = fmq_fileopen(op,dt) ;

#if	CF_DEBUGS
	    debugprintf("fmq_lockget: fmq_fileopen() rs=%d fd=%d\n",
	        rs,op->fd) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (f_read || (! op->f.writable)) {

#if	CF_DEBUGS
	    debugprintf("fmq_lockget: need READ lock\n") ;
#endif

	    f_already = op->f.readlocked ;
	    op->f.readlocked = TRUE ;
	    op->f.writelocked = FALSE ;
	    lockcmd = F_RLOCK ;

	} else {

#if	CF_DEBUGS
	    debugprintf("fmq_lockget: need WRITE lock\n") ;
#endif

	    f_already = op->f.writelocked ;
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = TRUE ;
	    lockcmd = F_WLOCK ;

	}

/* get out if we have the lock that we want already */

	if (f_already) {
	    rs = SR_OK ;
	    goto ret0 ;
	}

/* we need to actually do the lock */

#if	CF_DEBUGS
	debugprintf("fmq_lockget: lockfile() fd=%d cmd=%u\n",
		op->fd,lockcmd) ;
	debugprintstat("fmq_lockget: ",op->fd) ;
#endif

	{
	    offset_t	fs = op->filesize ;
	    rs = lockfile(op->fd,lockcmd,0L,fs,TO_LOCK) ;
	}

#if	CF_DEBUGS
	debugprintf("fmq_lockget: lockfile() rs=%d\n",rs) ;
#endif

ret0:

#if	CF_DEBUGS
	debugprintf("fmq_lockget: ret rs=%d fd=%d fileinit=%u\n",
	    rs,op->fd,op->f.fileinit) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	op->f.readlocked = FALSE ;
	op->f.writelocked = FALSE ;

bad0:
	goto ret0 ;
}
/* end subroutine (fmq_lockget) */


static int fmq_lockrelease(FMQ *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("fmq_lockrelease: ent\n") ;
#endif

	if ((op->f.readlocked || op->f.writelocked)) {
	    if (op->fd >= 0) {
		offset_t	fs = op->filesize ;
	        rs = lockfile(op->fd,F_ULOCK,0L,fs,TO_LOCK) ;
	    }
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = FALSE ;
	}

#if	CF_DEBUGS
	debugprintf("fmq_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fmq_lockrelease) */


static int fmq_fileopen(FMQ *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("fmq_fileopen: ent fname=%s\n",op->fname) ;
#endif

	if (op->fd < 0) {
	    if ((rs = u_open(op->fname,op->oflags,op->operm)) >= 0) {
		op->fd = rs ;
		uc_closeonexec(op->fd,TRUE) ;
		op->opentime = dt ;
	    }
	}

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (fmq_fileopen) */


int fmq_fileclose(FMQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (fmq_fileclose) */


/* write out the file header */
static int fmq_headwrite(FMQ *op)
{
	offset_t	uoff ;
	int		rs ;
	int		bl ;
	char		fbuf[FBUFLEN + 1] ;

	bl = filehead(fbuf,0,&op->h) ;

	uoff = FMQ_HEADOFF ;
	rs = u_pwrite(op->fd,fbuf,bl,uoff) ;

	return rs ;
}
/* end subroutine (fmq_headwrite) */


static int fmq_di(FMQ *op,sigset_t *smp)
{

#if	defined(PTHREAD) && PTHREAD
	pthread_sigmask(SIG_BLOCK,&op->sigmask,smp) ;
#else
	sigprocmask(SIG_BLOCK,&op->sigmask,smp) ;
#endif

	return SR_OK ;
}
/* end subroutine (fmq_di) */


/* ARGSUSED */
static int fmq_ei(FMQ *op,sigset_t *smp)
{


#if	defined(PTHREAD) && PTHREAD
	pthread_sigmask(SIG_SETMASK,smp,NULL) ;
#else
	sigprocmask(SIG_SETMASK,smp,NULL) ;
#endif

	return SR_OK ;
}
/* end subroutine (fmq_ei) */


static int filemagic(buf,f_read,mp)
char			*buf ;
int			f_read ;
FMQ_FM		*mp ;
{
	int		rs = 20 ;
	char		*bp = buf ;
	char		*cp ;

	if (buf == NULL) return SR_BADFMT ;

	if (f_read) {

	    bp[15] = '\0' ;
	    strncpy(mp->magic,bp,15) ;

	    if ((cp = strchr(mp->magic,'\n')) != NULL)
	        *cp = '\0' ;

	    bp += 16 ;
	    memcpy(mp->vetu,bp,4) ;

	} else {

	    bp = strwcpy(bp,mp->magic,15) ;

	    *bp++ = '\n' ;
	    memset(bp,0,(16 - (bp - buf))) ;

	    bp = buf + 16 ;
	    memcpy(bp,mp->vetu,4) ;

	} /* end if */

	return rs ;
}
/* end subroutine (filemagic) */


/* encode or decode the file header */
static int filehead(char *buf,int f_read,FMQ_FH *hp)
{
	uint		*table = (uint *) buf ;

	if (buf == NULL) return SR_BADFMT ;

	if (f_read) {

	    stdorder_ruint((table + 0),&hp->wcount) ;

	    stdorder_ruint((table + 1),&hp->wtime) ;

	    stdorder_ruint((table + 2),&hp->nmsg) ;

	    stdorder_ruint((table + 3),&hp->size) ;

	    stdorder_ruint((table + 4),&hp->blen) ;

	    stdorder_ruint((table + 5),&hp->len) ;

	    stdorder_ruint((table + 6),&hp->ri) ;

	    stdorder_ruint((table + 7),&hp->wi) ;

	} else {

	    stdorder_wuint((table + 0),hp->wcount) ;

	    stdorder_wuint((table + 1),hp->wtime) ;

	    stdorder_wuint((table + 2),hp->nmsg) ;

	    stdorder_wuint((table + 3),hp->size) ;

	    stdorder_wuint((table + 4),hp->blen) ;

	    stdorder_wuint((table + 5),hp->len) ;

	    stdorder_wuint((table + 6),hp->ri) ;

	    stdorder_wuint((table + 7),hp->wi) ;

	} /* end if */

	return FMQ_HEADLEN ;
}
/* end subroutine (filehead) */


#if	CF_DEBUGS
static int debugprintstat(s,fd)
const char	s[] ;
int		fd ;
{
	struct ustat	sb ;
	int		rs ;
	int		sl ;
	rs = u_fstat(fd,&sb) ;
	sl = debugprintf("%s fd=%d rs=%d size=%ld perm=%04o\n",
	    s,fd,rs,sb.st_size,sb.st_mode) ;
	return sl ;
}
/* end subroutine (debugprintstat) */
#endif /* CF_DEBUGS */


