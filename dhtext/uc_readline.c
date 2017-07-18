/* uc_readline */

/* interface component for UNIX® library-3c */
/* read a line from a file descriptor but time it */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_STREAM	1		/* optimization for STREAM */


/* revision history:

	= 1998-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes! Note that this subroutine depends on another
        little subroutine ('uc_reade()') that is used to provide an underlying
        extended 'read(2)' like capability.

	= 2003-11-25, David A­D­ Morano
        I have to laugh at how long some of these subroutines go before
        maintenance (looking at the date above)! I am adding the "peeking" type
        method of grabbing a line. This is superior to the old method in
        performance. In fairness, this subroutine was never used very much in
        performance-critical places but at least now I won't have to be
        embarassed about reading one character at a time (like many Internet
        daemons already do -- see the Berkeley remote-type protocols). For
        consistency with other similar subroutines, the option of FM_TIMED is
        assumed (check the name of this subroutine!).

*/

/* Copyright © 1998,2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get a line code amount of data (data ending in an NL) and time it also
        so that we can abort if it times out.

	Synopsis:

	int uc_readlinetimed(fd,lbuf,llen,to)
	int	fd ;
	char	lbuf[] ;
	int	llen ;
	int	to ;

	Arguments:

	fd		file descriptor
	lbuf		user buffer to receive daa
	llen		maximum amount of data the user wants
	to		time in seconds to wait

	Returns:

	>=0		amount of data returned
	<0		error

	Note 2003-11-25, David A­D­ Morano

        I am going to try to use one or more peeking techniques to speed this
        up. If it is a socket, then we are good to go and we will use 'recv(3n)'
        with the "PEEK" option. If it is a STREAM (who knows what is and what
        isn't now-a-days) we will try to use 'ioctl(2) with "I_PEEK". Else, we
        punt back to reading a character at a time.

        This subroutine performs like other subroutines, that can time the
        operation, with the option FM_TIMED set.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	(! defined(SYSHAS_STREAMS)) || (SYSHAS_STREAMS == 0)
#undef	CF_STREAM
#define	CF_STREAM	0
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>

#if	CF_STREAM
#include	<stropts.h>
#endif

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	BUFLEN		2048

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#define	POLLEVENTS	(POLLIN | POLLPRI)

#define	MAXEOF		3


/* external subroutines */

extern int	sichr(const char *,int,int) ;
extern int	isasocket(int) ;

extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */

static int	readline_socket(int,char *,int,int) ;
static int	readline_stream(int,char *,int,int) ;
static int	readline_seekable(int,char *,int,int,offset_t) ;
static int	readline_default(int,char *,int,int) ;

static int 	isseekable(int,offset_t *) ;


/* local variables */


/* exported subroutines */


int uc_readlinetimed(int fd,char *lbuf,int llen,int to)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    struct ustat	sb ;
	    debugprintf("uc_readlinetimed: fd=%d llen=%d to=%d\n",fd,llen,to) ;
	    rs = u_fstat(fd,&sb) ;
	    debugprintf("uc_readlinetimed: mode=%08o rdev=%08x\n",
	        sb.st_mode,sb.st_rdev) ;
	    debugprintf("uc_readlinetimed: ino=%llu dev=%08x\n",
	        sb.st_ino,sb.st_dev) ;
	    debugprintf("uc_readlinetimed: issock=%u isfifo=%u\n",
	        S_ISSOCK(sb.st_mode),S_ISFIFO(sb.st_mode)) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	{
	    int	rs1 ;
	    rs1 = u_fcntl(fd,F_GETFL,0) ;
	    debugprintf("uc_readlinetimed: rs=%d flags=%08x\n",
	        rs1,(rs1 & O_ACCMODE)) ;
	    debugprintf("uc_readlinetimed: ndelay=%u nonblock=%u\n",
	        (rs1 & O_NDELAY),(rs1 & O_NONBLOCK)) ;
	}
#endif /* CF_DEBUGS */

	if (lbuf == NULL) return SR_FAULT ;

	if (fd < 0) return SR_BADF ;

	if (llen < 0) return SR_INVALID ;

	if (llen > 0) {
	    offset_t	fo ;
	    if (isasocket(fd)) {
	       rs = readline_socket(fd,lbuf,llen,to) ;
	    } else if (isastream(fd)) {
	        rs = readline_stream(fd,lbuf,llen,to) ;
	    } else if (isseekable(fd,&fo)) {
	        rs = readline_seekable(fd,lbuf,llen,to,fo) ;
	    } else {
	        rs = readline_default(fd,lbuf,llen,to) ;
	    }
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("uc_readlinetimed: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_readlinetimed) */


int uc_readline(int fd,char *lbuf,int llen)
{

	return uc_readlinetimed(fd,lbuf,llen,-1) ;
}
/* end subroutine (uc_readline) */


/* private subroutines */


static int readline_socket(int fd,char *lbuf,int llen,int to)
{
	int		rs = SR_OK ;
	int		mopts = MSG_PEEK ;
	int		opts = (FM_TIMED | FM_EXACT) ;
	int		rlen ;
	int		len ;
	int		ch ;
	int		lbl ;
	int		tlen = 0 ;
	const char	*tp ;
	char		*lbp ;

#if	CF_DEBUGS
	debugprintf("uc_readlinetimed: method=SOCKET\n") ;
	debugprintf("uc_readlinetimed: llen=%u to=%d\n",llen,to) ;
#endif

	while ((rs >= 0) && (tlen < llen)) {
	    lbp = (lbuf + tlen) ;
	    lbl = (llen - tlen) ;
	    if ((rs = uc_recve(fd,lbp,lbl,mopts,to,opts)) >= 0) {
	        len = rs ;
		if (len > 0) {
	            tp = strnchr(lbp,len,'\n') ; /* NL present? */
	            rlen = (tp != NULL) ? ((tp+1)-lbp) : len ;
	    	    if ((rs = u_read(fd,lbp,rlen)) > 0) {
	    		len = rs ;
	    		tlen += len ;
	    		ch = MKCHAR(lbuf[tlen-1]) ;
	    		if ((ch == '\n') || (ch == '\0')) break ;
		    } else if (rs == 0) {
			break ;
		    }
		} else if (len == 0) {
		    break ;
		}
	    } /* end if (uc_recve) */
	} /* end while */

#if	CF_DEBUGS
	debugprintf("uc_readlinetimed/socket: ret rs=%d wlen=%u\n",rs,tlen) ;
#endif
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (readline_socket) */


static int readline_stream(int fd,char *lbuf,int llen,int to)
{
	struct pollfd	fds[2] ;
	struct strpeek	pd ;
	time_t		ti_now = time(NULL) ;
	time_t		ti_start ;
	int		rs = SR_OK ;
	int		events = POLLEVENTS ;
	int		rlen ;
	int		len ;
	int		ch ;
	int		tlen = 0 ;
	int		f_eof = FALSE ;
	int		f_to = FALSE ;
	const char	*tp ;
	char		cbuf[BUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("uc_readlinetimed: method=STREAM\n") ;
	debugprintf("uc_readlinetimed: to=%d\n",to) ;
#endif

	if (to < 0) to = INT_MAX ;

#ifdef	POLLRDNORM
	events |= POLLRDNORM ;
#endif
#ifdef	POLLRDBAND
	events |= POLLRDBAND ;
#endif

	memset(fds,0,sizeof(fds)) ;
	fds[0].fd = fd ;
	fds[0].events = events ;
	fds[1].fd = -1 ;
	fds[1].events = 0 ;

	memset(&pd,0,sizeof(struct strpeek)) ;
	pd.flags = 0 ;
	pd.ctlbuf.buf = cbuf ;
	pd.ctlbuf.maxlen = BUFLEN ;

	ti_start = ti_now ;
	while ((rs >= 0) && (tlen < llen) && (to >= 0)) {

	    pd.databuf.buf = (lbuf + tlen) ;
	    pd.databuf.maxlen = (llen - tlen) ;
	    if ((rs = u_poll(fds,1,POLLINTMULT)) > 0) {
	        if ((rs = u_ioctl(fd,I_PEEK,&pd)) >= 0) {
	            len = pd.databuf.len ;
/* is there a NL present? */
	            tp = strnchr((lbuf + tlen),len,'\n') ;
	            rlen = (tp != NULL) ? ((tp + 1) - (lbuf + tlen)) : len ;
	            if ((rs = u_read(fd,(lbuf + tlen),rlen)) >= 0) {
	                len = rs ;
	                f_eof = (len == 0) ;
			if (len > 0) {
	                    tlen += len ;
	                    ch = MKCHAR(lbuf[tlen-1]) ;
	                    if ((ch == '\n') || (ch == '\0')) break ;
			} else {
			    break ;
			}
		    } /* end if (u_read) */
		} /* end if (u_ioctl) */
	    } else if ((rs == 0) && (to >= 0)) {
	        ti_now = time(NULL) ;
	        f_to = ((ti_now - ti_start) >= to) ;
	        if (f_to) break ;
	    } else if (rs == SR_INTR) {
		rs = SR_OK ;
	    }

	} /* end while */

	if ((rs >= 0) && (tlen == 0) && f_to && (! f_eof) && (llen > 0)) {
	    rs = SR_TIMEDOUT ;
	} /* end if */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (readline_stream) */


/* ARGSUSED */
static int readline_seekable(int fd,char *lbuf,int llen,int to,offset_t fo)
{
	int		rs ;

	if ((rs = u_pread(fd,lbuf,llen,fo)) > 0) {
	    int	rlen = rs ;
	    int	si ;
	    if ((si = sichr(lbuf,rlen,'\n')) > 0) {
		rlen = (si+1) ;
	    }
	    rs = u_read(fd,lbuf,rlen) ;
	} /* end if (u_pread) */

	return rs ;
}
/* end subroutine (readline_seekable) */


static int readline_default(int fd,char *lbuf,int llen,int to)
{
	int		rs = SR_OK ;
	int		opts = (FM_TIMED | FM_EXACT) ;
	int		len ;
	int		ch ;
	int		tlen = 0 ;

#if	CF_DEBUGS
	debugprintf("uc_readlinetimed: method=DEFAULT\n") ;
#endif

	while ((rs >= 0) && (tlen < llen)) {
	    if ((rs = uc_reade(fd,(lbuf + tlen),1,to,opts)) >= 0) {
	        len = rs ;
	        if (len > 0) {
	            tlen += len ;
	            ch = MKCHAR(lbuf[tlen-1]) ;
	            if ((ch == '\n') || (ch == '\0')) break ;
	        } else {
		    break ;
	        }
	    } /* end if (uc_reade) */
	} /* end while */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (readline_default) */


static int isseekable(int fd,offset_t *fop)
{
	int		rs ;
	int		f = FALSE ;

	if ((rs = u_seeko(fd,0L,SEEK_CUR,fop)) >= 0) {
	    f = TRUE ;
	} else if (rs == SR_SPIPE) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (isseekable) */


