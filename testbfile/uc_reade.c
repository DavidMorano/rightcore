/* uc_reade */

/* interface component for UNIX® library-3c */
/* extended read */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_NONBLOCK	1		/* use nonblocking mode */


/* revision history:

	= 1998-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get some amount of data and time it also so that we can abort if it
        times out.

	Synopsis:

	int uc_reade(fd,rbuf,rlen,to,opts)
	int		fd ;
	void		*rbuf ;
	int		rlen ;
	int		to ;
	int		opts ;

	Arguments:

	fd		file descriptor
	rbuf		user buffer to receive daa
	rlen		maximum amount of data the user wants
	to		time in seconds to wait
	opts		user options for time-out handling

	Returns:

	>=0		amount of data returned
	<0		error

	= The question:

        What do we want to return on a timeout? This is the big unanswered
        question of the ages? Do we want to treat the input FD like a STREAM or
        a SOCKET (returning SR_AGAIN) or do we want to treat it like a FIFO or
        TERMINAL (returning SR_OK == 0)? We will let this be determined by the
        caller by setting (or not setting) 'FM_AGAIN' in the options!

        If the caller sets 'FM_AGAIN' in the options, we return SR_AGAIN if
        there is no data (it timed out). If the caller sets 'FM_TIMED', then we
        return SR_TIMEDOUT if it times out. Finally, if the caller doesn't set
        that, we will return the amount of data received at the time of the
        timeout (which can inlucde the value ZERO).

        An explicit read of 0 bytes (EOF) always return 0 (EOF). If FM_EXACT was
        specified and the requested number of bytes has not yet arrived an EOF
        will be ignored and an attempt will be made to read more data in. Also
        if FM_EXACT is specified and the required number of bytes has not
        arrived (but some have), we continue reading until the required number
        of bytes arrives of if a time-out occurs.

	Read sematics are as follow:

	1. The default semantic (neither "FM_AGAIN" nor "FM_TIMED"):

        + If a non-negative timeout value is given and the timeout occurs when
        no data has arrived, then we return ZERO.

        + If a non-negative timeout value is given and the timeout occurs when
        some data has arrived, then we return the amount of data received.

	2. The "FM_AGAIN" semantic:

        + If a non-negative timeout value is given and the timeout occurs when
        no data has arrived, then we return SR_AGAIN.

        + If a non-negative timeout value is given and the timeout occurs when
        some data has arrived, then we return the amount of data received.

	3. The "FM_TIMED" semantic:

        + If a non-negative timeout value is given and the timeout occurs when
        no data has arrived, then we return SR_TIMEDOUT.

        + If a non-negative timeout value is given and the timeout occurs when
        some data has arrived, then we return the amount of data received.

	= Some notes:

        Watch out for receiving hang-ups! This can happen when the
        file-descriptor used for these reads is also used for some writes
        elsewhere. How should we handle a hang-up? That is a good question.
        Since input is not supposed to be affected by a hang-up, we just
        continue on a hang-up unless there was data read. If we get a hang-up
        and no data was read, then it is an EOF condition and we will return as
        if we received an EOF (like we assume that we must have).

	= The observation:

        Is it poossible to receive an EOF condition on the input *without*
        receiving a POLLIN? Amazingly, the answer is YES! If a hang-up is
        present on the input, then a hang-up condition will be received
        (POLLHUP) *rather* than a usual EOF condition. Amazing as it is, this is
        possible. In my opinion, this should not be possible (an EOF should
        always create a POLLIN) but believe it or not that is not what happens
        in real life.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000		/* poll-time multiplier */
#endif

#ifndef	POLLTIMEINT
#define	POLLTIMEINT	10		/* seconds */
#endif

#define	EBUFLEN		100

#define	MAXEOF		3

#define	POLLEVENTS	(POLLIN | POLLPRI)

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
#endif


/* local structures */

struct subinfo_flags {
	uint		again:1 ;
	uint		timed:1 ;
	uint		exact:1 ;
	uint		timeint:1 ;
	uint		nonblock:1 ;
	uint		isfifo:1 ;
	uint		ischar:1 ;
	uint		isdir:1 ;
	uint		isblock:1 ;
	uint		isreg:1 ;
	uint		issocket:1 ;
	uint		isother:1 ;
	uint		isnonblock:1 ; /* was non-blocking */
} ;

struct subinfo {
	char		*ubuf ;
	char		*bp ;
	SUBINFO_FL	f ;
	int		fd ;
	int		ulen ;
	int		uto ;
	int		tlen ;
	int		to ;		/* down-counter */
	int		opts ;
	int		neof ;
	int		maxeof ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,int,char *,int,int,int) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_setmode(SUBINFO *,mode_t) ;
static int	subinfo_readreg(SUBINFO *) ;
static int	subinfo_readslow(SUBINFO *) ;
static int	subinfo_readpoll(SUBINFO *) ;

#if	CF_DEBUGS
static char	*d_reventstr() ;
#endif


/* local variables */


/* exported subroutines */


int uc_reade(int fd,void *vbuf,int ulen,int to,int opts)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		tlen = 0 ;
	char		*ubuf = (char *) vbuf ;

	if (fd < 0) return SR_BADF ;

#if	CF_DEBUGS
	debugprintf("uc_reade: ent fd=%d ulen=%d to=%d opts=%04x\n",
	    fd,ulen,to,opts) ;
	debugprintf("uc_reade: FM_TIMED=%u\n",
	    ((opts & FM_TIMED) ? 1 : 0)) ;
	debugprintf("uc_reade: FM_EXACT=%u\n",
	    ((opts & FM_EXACT) ? 1 : 0)) ;
#endif

	if (to < 0) to = INT_MAX ;

	if ((rs = subinfo_start(sip,fd,ubuf,ulen,to,opts)) >= 0) {
	    int	f = FALSE ;

	    f = f || sip->f.isdir || sip->f.isblock ;
	    f = f || sip->f.isreg || sip->f.isnonblock ;
	    if (f) {
	        rs = subinfo_readreg(sip) ;
	    } else {
	        rs = subinfo_readslow(sip) ;
	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("uc_reade: subinfo_readxxx() rs=%d\n",rs) ;
#endif

	    tlen = subinfo_finish(sip) ;
	    if (rs >= 0) rs = tlen ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("uc_reade: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uc_reade) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,int fd,char *ubuf,int ulen,int to,int ro)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->fd = fd ;
	sip->ubuf = ubuf ;
	sip->bp = ubuf ;
	sip->ulen = ulen ;
	sip->uto = to ;
	sip->to = to ;
	sip->opts = ro ;

	sip->f.again = MKBOOL(ro & FM_AGAIN) ;
	sip->f.timed = MKBOOL(ro & FM_TIMED) ;
	sip->f.timeint = MKBOOL(ro & FM_TIMEINT) ;
	sip->f.exact = MKBOOL(ro & FM_EXACT) ;

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_start: again=%u timed=%u\n",
	    sip->f.again,sip->f.timed) ;
	debugprintf("uc_reade/subinfo_start: timeint=%u exact=%u\n",
	    sip->f.timeint,sip->f.exact) ;
#endif

	if ((rs = u_fstat(fd,&sb)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("uc_reade/subinfo_start: u_fstat() rs=%d\n",rs) ;
	    debugprintf("uc_reade/subinfo_start: mode=\\o%012o\n",
		sb.st_mode) ;
#endif

	    if ((rs = subinfo_setmode(sip,sb.st_mode)) >= 0) {

#if	CF_DEBUGS
	        {
	            cchar	*filetype ;
	            if (sip->f.isfifo) filetype = "pipe" ;
	            else if (sip->f.ischar) filetype = "char" ;
	            else if (sip->f.isdir) filetype = "dir" ;
	            else if (sip->f.isblock) filetype = "block" ;
	            else if (sip->f.isreg) filetype = "reg" ;
	            else if (sip->f.issocket) filetype = "socket" ;
	            else filetype = "other" ;
	            debugprintf("uc_reade/subinfo_start: filetype=%s\n",
		        filetype) ;
	        }
#endif /* CF_DEBUGS */

		if (sip->f.isother) {

#if	CF_DEBUGS
	        debugprintf("uc_reade/subinfo_start: OTHER!\n") ;
#endif

/* yes! some (maybe many) files do *not* support non-blocking mode */

#if	CF_NONBLOCK
	    if (! sip->f.isreg) {
	        sip->f.nonblock = TRUE ;
	        rs1 = uc_nonblock(fd,TRUE) ;
	        sip->f.isnonblock = (rs1 > 0) ;
	        if (rs1 == SR_NOSYS) {
	            sip->f.isnonblock = TRUE ;
	        } else {
	            rs = rs1 ;
	        }

#if	CF_DEBUGS
	        debugprintf("uc_reade/subinfo_start: uc_nonblock() rs=%d\n",
			rs1) ;
	        debugprintf("uc_reade/subinfo_start: f_isnonblock=%u\n",
	            sip->f.isnonblock) ;
	        debugprintf("uc_reade/subinfo_start: f_nonblock=%u\n",
	            sip->f.nonblock) ;
#endif

	    }
#endif /* CF_NONBLOCK */

	       } /* end if (other file type) */
	   } /* end if (subinfo_setmode) */
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		tlen = sip->tlen ;

#if	CF_NONBLOCK
	if (sip->f.nonblock)
	    rs = uc_nonblock(sip->fd,FALSE) ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_finish: tlen=%d to=%d neof=%d ulen=%d\n",
	    sip->tlen,sip->to,sip->neof,sip->ulen) ;
#endif

	if ((rs >= 0) && (tlen == 0) && (sip->to == 0) && (sip->ulen > 0)) {
	    int	f = FALSE ;

	    if (sip->f.issocket) {
	        f = (sip->neof < sip->maxeof) ;
	    } else {
	        f = (sip->neof == 0) ;
	    }

	    if (f) {
	        if (sip->opts & FM_AGAIN) {
	            rs = SR_AGAIN ;
	        } else if (sip->opts & FM_TIMED) {
	            rs = SR_TIMEDOUT ;
	        }
	    }

	} /* end if (had a timeout) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_setmode(SUBINFO *sip,mode_t fm)
{
	if (S_ISFIFO(fm)) {
	    sip->f.isfifo = TRUE ;
	} else if (S_ISCHR(fm)) {
	    sip->f.ischar = TRUE ;
	} else if (S_ISDIR(fm)) {
	    sip->f.isdir = TRUE ;
	} else if (S_ISBLK(fm)) {
	    sip->f.isblock = TRUE ;
	} else if (S_ISREG(fm)) {
	    sip->f.isreg = TRUE ;
	} else if (S_ISSOCK(fm)) {
	    sip->f.issocket = TRUE ;
	} else {
	    sip->f.isother = TRUE ;
	}
	return SR_OK ;
}
/* end subroutine (subinfo_setmode) */


static int subinfo_readreg(SUBINFO *sip)
{
	int		rs ;
	int		tlen = 0 ;

	sip->maxeof = 0 ;
	if ((rs = u_read(sip->fd,sip->ubuf,sip->ulen)) >= 0) {
	    tlen = rs ;
	    if (tlen > 0) {
	        sip->tlen += tlen ;
	        sip->neof = 0 ;
	    } else {
	        sip->neof += 1 ;
	    }
	} /* end if (u_read) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (subinfo_readreg) */


static int subinfo_readslow(SUBINFO *sip)
{
	struct pollfd	fds[2] ;
	int		rs = SR_OK ;
	int		events = POLLEVENTS ;

#if	CF_DEBUGS
	char		ebuf[EBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_readslow: ent to=%d\n",sip->to) ;
#endif

	sip->maxeof = MAXEOF ;

#if	defined(POLLRDNORM)
	events |= POLLRDNORM ;
#endif
#if	defined(POLLRDBAND)
	events |= POLLRDBAND ;
#endif

/* initialization for 'u_poll(2u)' */

	memset(fds,0,sizeof(fds)) ;
	fds[0].fd = sip->fd ;
	fds[0].events = events ;
	fds[1].fd = -1 ;

/* go */

	while ((rs >= 0) && ((sip->ulen - sip->tlen) > 0)) {
	    int	f_break = FALSE ;

	    if ((rs = u_poll(fds,1,POLLINTMULT)) > 0) {
	        const int	re = fds[0].revents ;

#if	CF_DEBUGS
	        debugprintf("uc_reade/subinfo_readslow: events %s\n",
	            d_reventstr(re,ebuf,EBUFLEN)) ;
	        debugprintf("uc_reade/subinfo_readslow: about to 'read'\n") ;
#endif /* CF_DEBUGS */

	        if ((re & POLLIN) || (re & POLLPRI)) {
	            rs = subinfo_readpoll(sip) ;
	            f_break = (rs > 0) ;
	        } else if (re & POLLNVAL) {
	            rs = SR_NOTOPEN ;
	        } else if (re & POLLERR) {
	            rs = SR_POLLERR ;
	        } else if (re & POLLHUP) {
	            if (sip->tlen == 0) break ;
	            msleep(1) ;
	        }

	    } else if (rs == SR_INTR) {
	        rs = SR_OK ;
	    } else if (rs == 0) { /* u_poll() returned w/ nothing */
	        if (sip->to > 0) {
	            sip->to -= 1 ;
	        } else {
	            f_break = TRUE ;
	        }
	    } /* end if (otherwise it must be an error) */

	    if (sip->f.isnonblock) break ;
	    if (f_break) break ;
	} /* end while (looping on poll) */

#if	CF_DEBUGS
	debugprintf("uc_reade/readslow: ret tlen=%u\n",sip->tlen) ;
	debugprintf("uc_reade/readslow: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_readslow) */


static int subinfo_readpoll(SUBINFO *sip)
{
	int		rs ;
	int		rlen ;
	int		len ;
	int		f_break = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_readpoll: ent\n") ;
#endif

	rlen = (sip->ulen - sip->tlen) ;
	if ((rs = u_read(sip->fd,sip->bp,rlen)) >= 0) {
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("uc_reade/subinfo_readpoll: u_read() rs=%d\n",rs) ;
#endif

	    if (len == 0) {
	        sip->neof += 1 ;
	        if ((! sip->f.issocket) || (sip->neof >= sip->maxeof)) {
	            f_break = TRUE ;
	        }
	    } else {
	        sip->neof = 0 ;		/* reset */
	    }

	    sip->tlen += len ;
	    sip->bp += len ;
	    if ((! f_break) && (len > 0) && (! sip->f.exact)) {
	        f_break = TRUE ;
	    }

	    if ((! f_break) && (len > 0) && sip->f.timeint) {
	        sip->to = sip->uto ;	/* reset */
	    }

	} else if (rs == SR_AGAIN) {
	    if (! sip->f.isnonblock) rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("uc_reade/subinfo_readpoll: ret tlen=%u\n",sip->tlen) ;
	debugprintf("uc_reade/subinfo_readpoll: ret rs=%d f_break=%u\n",
	    rs,f_break) ;
#endif

	return (rs >= 0) ? f_break : rs ;
}
/* end subroutine (subinfo_readpoll) */


#if	CF_DEBUGS
static char *d_reventstr(int revents,char *dbuf,int dlen)
{
	dbuf[0] = '\0' ;
	bufprintf(dbuf,dlen,"%s %s %s %s %s %s %s %s %s",
	    (revents & POLLIN) ? "I " : "  ",
	    (revents & POLLRDNORM) ? "IN" : "  ",
	    (revents & POLLRDBAND) ? "IB" : "  ",
	    (revents & POLLPRI) ? "PR" : "  ",
	    (revents & POLLWRNORM) ? "WN" : "  ",
	    (revents & POLLWRBAND) ? "WB" : "  ",
	    (revents & POLLERR) ? "ER" : "  ",
	    (revents & POLLHUP) ? "HU" : "  ",
	    (revents & POLLNVAL) ? "NV" : "  ") ;
	return dbuf ;
}
/* end subroutine (d_reventstr) */
#endif /* CF_DEBUGS */


