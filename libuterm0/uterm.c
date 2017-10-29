/* uterm */

/* "UNIX Terminal" helper routines */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */
#define	CF_SIGNAL	1	/* 1={local handle}, 0={UNIX handles} */
#define	CF_BADTIME	0	/* Solaris timeout screwup */
#define	CF_FIRSTREAD	0	/* perform an initial 'read()'? */
#define	CF_WRITEATOM	1	/* atomic write */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*************************************************************************

	These routines provide a stand-alone TTY environment.


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stropts.h>
#include	<poll.h>
#include	<termios.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<charq.h>
#include	<vecobj.h>
#include	<buffer.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"uterm.h"


/* local defines */

#define	UTERM_MAGIC	0x33442281

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	TTY_MINCHARS	0	/* minimum characters to get */
#define	TTY_MINTIME	5	/* minimum time (x100 milliseconds) */

#define	TTY_READCHARS	20

#define	WBUFLEN		40	/* maximum output write buffer */

#define	MON_INTERVAL	3	/* the error monitoring interval */
#define	MAXLOOPS	300	/* maximum loops in monitor interval */

#define	TA_SIZE		256
#define	EC_SIZE		256

/* output priorities */

#define	OPM_WRITE	0x01
#define	OPM_READ	0x02
#define	OPM_ECHO	0x04
#define	OPM_ANY		0x07

#define	OPV_WRITE	0
#define	OPV_READ	1
#define	OPV_ECHO	2

#undef	CH_FILTER
#define	CH_FILTER	0x100

#define	FILTER(c)	((c) | CH_FILTER)

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	100
#endif


/* externals subroutines */

extern int	isprintlatin(int) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	uterm_writeproc(UTERM *,const char *,int) ;

static int	tty_wait(), tty_echo(), tty_risr() ;
static int	tty_wps(UTERM *,const char *,int) ;
static int	tty_loadchar(UTERM *,const char *,int) ;

static int	sinotprint(const char *,int) ;


/* static variables */

static const uchar	dterms[] = {
	0xEF, 0xFC, 0xC0, 0xFE,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;


/* exported subroutines */


/* TTY initialization routine */
int uterm_start(op,fd)
UTERM		*op ;
int		fd ;
{
	struct termios	*tp ;

	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (fd < 0)
	    return SR_INVALID ;

	memset(op,0,sizeof(UTERM)) ;

	tp = &op->ts_new ;

	op->fd = fd ;

	op->stat = 0 ;
	op->f.co = FALSE ;
	op->f.cc = FALSE ;

	op->mode = 0 ;
	op->loopcount = 0 ;
	op->basetime = time(NULL) ;

/* initialize the UNIX line */

	rs = uc_tcgetattr(fd,&op->ts_old) ;
	if (rs < 0)
	    goto bad1 ;

	op->ts_new = op->ts_old ;

	tp->c_iflag &= 
	    (~ (INLCR | ICRNL | IUCLC | IXANY | ISTRIP | INPCK | PARMRK)) ;
	tp->c_iflag |= IXON ;

	tp->c_cflag &= (~ (CSIZE)) ;
	tp->c_cflag |= CS8 ;

#if	CF_SIGNAL
	tp->c_lflag &= (~ (ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL)) ;
#else
	tp->c_lflag &= (~ (ICANON | ECHO | ECHOE | ECHOK | ECHONL)) ;
#endif

	tp->c_oflag &= (~ (OCRNL | ONOCR | ONLRET)) ;

#if	CF_BADTIME
	if (u_access("/usr/sbin",R_OK) >= 0) {
	    tp->c_cc[VMIN] = 1 ;
	} else
	    tp->c_cc[VMIN] = TTY_MINCHARS ;
#else
	tp->c_cc[VMIN] = TTY_MINCHARS ;
#endif /* CF_BADTIME */

	tp->c_cc[VTIME] = TTY_MINTIME ;

#if	CF_DEBUGS
	debugprintf("uterm_start: MINCHARS=%u MINTIME=%u\n",
	    tp->c_cc[VEOF],tp->c_cc[VEOL]) ;
#endif

/* set the new attributes */

	rs = uc_tcsetattr(fd,TCSADRAIN,tp) ;
	if (rs < 0)
	    goto bad1 ;

/* initialize the TA buffer */

	rs = charq_start(&op->taq,TA_SIZE) ;
	if (rs < 0)
	    goto bad2 ;

/* initialize the EC buffer */

	rs = charq_start(&op->ecq,EC_SIZE) ;
	if (rs < 0)
	    goto bad3 ;

	op->magic = UTERM_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("uterm_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff comes here */
bad3:
	charq_finish(&op->taq) ;

bad2:
	uc_tcsetattr(fd,TCSADRAIN,&op->ts_old) ;

bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (uterm_start) */


int uterm_finish(op)
UTERM		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = charq_finish(&op->ecq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = charq_finish(&op->taq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uterm_restore(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;

ret0:
	return rs ;
}
/* end subroutine (uterm_finish) */


int uterm_suspend(op)
UTERM		*op ;
{


	return uterm_restore(op) ;
}
/* end subroutine (uterm_suspend) */


int uterm_resume(op)
UTERM		*op ;
{


	return uterm_ensure(op) ;
}
/* end subroutine (uterm_resume) */


/* restore settings */
int uterm_restore(op)
UTERM		*op ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_tcsetattr(op->fd,TCSADRAIN,&op->ts_old) ;

	return rs ;
}
/* end subroutine (uterm_restore) */


/* ensure settings */
int uterm_ensure(op)
UTERM		*op ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_tcsetattr(op->fd,TCSADRAIN,&op->ts_new) ;

	return rs ;
}
/* end subroutine (uterm_ensure) */


/* control function */
int uterm_control(UTERM *op,int cmd,...)
{
	va_list	ap ;

	int	rs = SR_OK ;
	int	iw, *iwp ;
	int	fm ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("uterm_control: cmd=%u\n",
		(cmd & FM_MASK)) ;
#endif

	va_begin(ap,cmd) ;

	fm = (cmd & FM_MASK) ;
	switch (fm) {

	case fm_setmode:
	    iw = (int) va_arg(ap,int) ;
	    op->mode = iw ;
	    break ;

	case fm_getmode:
	    iwp = (int *) va_arg(ap,int *) ;
	    if (iwp != NULL) *iwp = op->mode ;
	    break ;

	case fm_reestablish:
	    rs = uc_tcsetattr(op->fd,TCSADRAIN,&op->ts_new) ;
	    break ;

	default:
	    rs = SR_INVALID ;
	    break ;

	} /* end switch */

	va_end(ap) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uterm_control: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uterm_control) */


/* status function */
int uterm_status(UTERM *op,int cmd,...)
{
	va_list	ap ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	return SR_NOTSUP ;
}
/* end subroutine (uterm_status) */


/* read a line from the terminal with extended parameters */
int uterm_reade(op,rbuf,rlen,timeout,fc,lpp,llp)
UTERM		*op ;
char		rbuf[] ;
int		rlen ;
int		timeout ;
int		fc ;
UTERM_PROMPT	*lpp ;
UTERM_LOAD	*llp ;
{
	int	rs = SR_OK ;
	int	c = -1 ;
	int	count = 0 ;

	uchar	*terms = NULL ;
	char	ch ;			/* MUST! be a character */


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	if (rbuf == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("uterm_reade: to=%d\n",timeout) ;
#endif

	fc |= op->mode ;
	if (fc & fm_rawin)
	    fc |= fm_nofilter ;

	if (fc & fm_noecho)
	    fc |= fm_notecho ;

	if (terms == NULL)
	    terms = (uchar *) dterms ;

	op->f.cc = FALSE ;
	op->f.co = FALSE ;		/* cancel ^O effect */

	op->f.read = TRUE ;		/* read is in progress */

/* top of further access */
top:
	if ((rs >= 0) && (lpp != NULL) && (lpp->plen > 0))
	    rs = tty_wps(op,lpp->pbuf,lpp->plen) ;

	if ((rs >= 0) && (llp != NULL) && (llp->llen > 0))
	    rs = tty_loadchar(op,llp->lbuf,llp->llen) ;

	count = 0 ;

/* check TA buffer first */
next:
	while ((rs >= 0) && (count < rlen)) {
	    int	f_eot = FALSE ;

	    while (charq_rem(&op->taq,&ch) < 0) {

	        rs = tty_wait(op,timeout) ;

#if	CF_DEBUGS
	debugprintf("uterm_reade: tty_wait() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	        if ((timeout >= 0) && (op->timeout <= 0))
	            break ;

	    } /* end while */

	    if (rs < 0)
	        break ;

	    if ((timeout >= 0) && (op->timeout <= 0))
	        break ;

	    if (op->f.cc)
	        break ;

	    c = (ch & 0xff) ;
	    rbuf[count] = c ;

#if	CF_DEBUGS
	    debugprintf("uterm_reade: got a character ch=%c (%02X)\n",
		(isprintlatin(c) ? c : ' '),c) ;
#endif

/* check for terminator */

	    if (BATST(terms,c))
	        break ;

/* check for a filter character */

	    if (! (fc & fm_nofilter)) {

	        switch (c) {

		case CH_EOT:
		    if (count == 0)
		        f_eot = TRUE ;
		    c = -1 ;
		    break ;

	        case CH_NAK:
	            if (! (fc & fm_noecho))
	                rs = tty_echo(op," ^U\r\n",5) ;
	            c = -1 ;
	            break ;

	        case CH_DEL:
	        case CH_BS:
	            if (count > 0) {
	                count -= 1 ;
	                if (! (fc & fm_noecho))
	                    rs = tty_echo(op,"\b \b",3) ;
	            }
	            c = -1 ;
	            break ;

	        case CH_DC2:
	            if (! (fc & fm_noecho)) {
	                rs = tty_echo(op," ^R\r\n",5) ;
	                if ((rs >= 0) && (lpp != NULL) && 
				(lpp->pbuf != NULL) && (lpp->plen > 0))
	                    rs = tty_wps(op,lpp->pbuf,lpp->plen) ;
	                if ((rs >= 0) && (count > 0))
	                    rs = tty_echo(op,rbuf,count) ;
	            }
	            c = -1 ;
	            break ;

	        case CH_CAN:
	            if (! (fc & fm_noecho))
	                rs = tty_echo(op," ^X\r\n",5) ;
	            c = -1 ;
	            break ;

	        } /* end switch */

	    } /* end if (filtering) */

	    if (c >= 0) {
	        if ((! (fc & fm_noecho)) && isprintlatin(c)) {
	            ch = (uchar) c ;
	            rs = tty_echo(op,&ch,1) ;
	        }
	        count += 1 ;
	    } /* end if (normal character) */

	    if (f_eot)
		break ;

	} /* end while */

/* exit processing */

#if	CF_DEBUGS
	debugprintf("uterm_reade: exit processing rs=%d count=%u\n",
		rs,count) ;
#endif

	if ((rs >= 0) && (c >= 0) && BATST(terms,c)) {

	    switch (c) {

	    case CH_CR:
	    case CH_LF:
	        if (! ((fc & fm_notecho) || (fc & fm_noecho)))
	            rs = tty_echo(op,"\r\n",2) ;
	        if (! (fc & fm_nofilter))
	            rbuf[count] = CH_LF ;
	        break ;

	    case CH_ESC:
	    case CH_CSI:
	        if (! ((fc & fm_notecho) || (fc & fm_noecho)))
	            rs = tty_echo(op,"$",1) ;
	        break ;

	    } /* end switch */

	    count += 1 ;

	} /* end if (terminator processing) */

ret0:

#if	CF_DEBUGS
	debugprintf("uterm_reade: ret rs=%d count=%u\n",rs,count) ;
#endif

	op->f.read = FALSE ;
	return (rs >= 0) ? count : rs ;
}
/* end subroutine (uterm_reade) */


int uterm_read(op,rbuf,rlen)
UTERM		*op ;
char		rbuf[] ;
int		rlen ;
{


	return uterm_reade(op,rbuf,rlen,-1,0,NULL,NULL) ;
}
/* end subroutine (uterm_read) */


/* write routine */
int uterm_write(op,wbuf,wlen)
UTERM		*op ;
const char	wbuf[] ;
int		wlen ;
{
	int	rs = SR_OK ;
	int	tlen = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	tlen = 0 ;
	if (op->f.co)
	    goto ret0 ;

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

	if (op->mode & fm_rawout) {

#if	CF_WRITEATOM
	    rs = u_write(op->fd,wbuf,wlen) ;
#else /* CF_WRITEATOM */
	    {
	        int	blen = wlen ;
	        int	mlen ;

	        char	*bp = wbuf ;


	        while ((rs >= 0) && (blen > 0) && (! op->f.co)) {

	            mlen = MIN(blen,WBUFLEN) ;
	            rs = u_write(op->fd,bp,mlen) ;

	            blen -= mlen ;
	            tlen += mlen ;
	            bp += mlen ;

	        } /* end while */

	    }
#endif /* CF_WRITEATOM */

	} else {

	    rs = uterm_writeproc(op,wbuf,wlen) ;
	    tlen = rs ;

	}

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uterm_write) */


/* cancel a terminal request */
int uterm_cancel(op,fc,cparam)
UTERM		*op ;
int		fc, cparam ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	return SR_OK ;
}
/* end subroutine (uterm_cancel) */


/* poll for a character */
int uterm_poll(op)
UTERM		*op ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != UTERM_MAGIC)
	    return SR_NOTOPEN ;

	rs = tty_wait(op,0) ;

	if ((rs >= 0) && op->f.cc)
	    rs = SR_CANCELED ;

	return rs ;
}
/* end subroutine (uterm_poll) */


/* private subroutines */


static int uterm_writeproc(op,buf,buflen)
UTERM		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	sl = -1 ;
	int	tlen = 0 ;

	const char	*tp ;
	const char	*sp ;


	tlen = buflen ;
	if ((tp = strnchr(buf,buflen,'\n')) != NULL) {
	    BUFFER	pb ;

	    if ((rs = buffer_start(&pb,(buflen + 10))) >= 0) {
	        int		bl = buflen ;
	        const char	*bp = buf ;

	        buffer_buf(&pb,bp,(tp - bp)) ;

	        buffer_char(&pb,'\r') ;

	        buffer_char(&pb,'\n') ;

	        bl -= ((tp + 1) - bp) ;
	        bp = (tp + 1) ;

	        while ((tp = strnchr(bp,bl,'\n')) != NULL) {

	            buffer_buf(&pb,bp,(tp - bp)) ;

	            buffer_char(&pb,'\r') ;

	            buffer_char(&pb,'\n') ;

	            bl -= ((tp + 1) - bp) ;
	            bp = (tp + 1) ;

	        } /* end while */

	        if (bl > 0)
	            buffer_buf(&pb,bp,bl) ;

	        rs = buffer_get(&pb,&sp) ;
	        sl = rs ;
	        if ((rs >= 0) && (sl > 0))
	            rs = u_write(op->fd,sp,sl) ;

	        buffer_finish(&pb) ;
	    } /* end if (initialize buffer) */

	} else
	    rs = u_write(op->fd,buf,buflen) ;

ret0:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uterm_writeproc) */


/* write out a prompt string */
static int tty_wps(op,buf,buflen)
UTERM		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	ci ;


	if (buflen < 0)
	    buflen = strlen(buf) ;

	if (buflen == 0)
	    goto ret0 ;

	if ((ci = sinotprint(buf,buflen)) >= 0) {
	    BUFFER	pb ;

	    if ((rs = buffer_start(&pb,buflen)) >= 0) {
	        int		bl = buflen ;
		int		sl = -1 ;
		const char	*sp ;
	        const char	*bp = buf ;

	        if (ci > 0)
	            buffer_buf(&pb,bp,ci) ;

	        bp += (ci + 1) ;
	        bl -= (ci + 1) ;

	        while ((bl > 0) && 
	            ((ci = sinotprint(bp,bl)) >= 0)) {

	            if (ci > 0)
	                buffer_buf(&pb,bp,ci) ;

	            bp += (ci + 1) ;
	            bl -= (ci + 1) ;

	        } /* end while */

	        if (bl > 0)
	            buffer_buf(&pb,bp,bl) ;

	        rs = buffer_get(&pb,&sp) ;
	        sl = rs ;
	        if ((rs >= 0) && (sl > 0))
	            rs = u_write(op->fd,sp,sl) ;

	        buffer_finish(&pb) ;
	    } /* end if (initialize buffer) */

	} else
	    rs = u_write(op->fd,buf,buflen) ;

ret0:
	return (rs >= 0) ? buflen : rs ;
}
/* end subroutine (tty_wps) */


static int tty_loadchar(op,pbuf,pbuflen)
UTERM		*op ;
const char	pbuf[] ;
int		pbuflen ;
{
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


	for (i = 0 ; (rs >= 0) && (i < pbuflen) ; i += 1) {
	    if (isprintlatin(pbuf[i])) {
	        c += 1 ;
	        rs = charq_ins(&op->taq,pbuf[i]) ;
	    }
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (tty_loadchar) */


/* wait for a character to arrive */
static int tty_wait(op,timeout)
UTERM		*op ;
int		timeout ;
{
	struct pollfd	fds[2] ;

	time_t	daytime, lasttime ;

	int	rs ;
	int	i ;
	int	len = 0 ;
	int	nfds ;
	int	polltime ;
	int	f_starting, f_looping ;

	uchar	cbuf[TTY_READCHARS + 1] ;


/* initialize the POLL structure */

	nfds = 0 ;
	fds[nfds].fd = op->fd ;
	fds[nfds].events = POLLIN ;
	nfds += 1 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = POLLIN ;

	if (timeout < 0)
	    timeout = INT_MAX ;

	polltime = MIN(3000,(timeout * 1000)) ;

	op->timeout = timeout ;
	daytime = time(NULL) ;

	lasttime = daytime ;

#if	CF_FIRSTREAD

/* try it out the first time */

	rs = u_read(op->fd,cbuf,TTY_READCHARS) ;
	len = rs ;
	f_starting = TRUE ;
	goto enter ;

#endif /* CF_FIRSTREAD */

/* loop it */
loop:
	f_starting = FALSE ;

#if	CF_DEBUGS
	debugprintf("tty_wait: top loop, timeout=%d\n",op->timeout) ;
#endif

/* zero out the buffer so that we can tell if Solaris screws us later */

	for (i = 0 ; i < TTY_READCHARS ; i += 1)
	    cbuf[i] = '\0' ;

/* wait for read attention */

	rs = u_poll(fds,1,polltime) ;

#if	CF_DEBUGS
	debugprintf("tty_wait: u_poll() rs=%d\n",rs) ;
	if (rs > 0) {
	    if (fds[0].revents & POLLIN) {
	        debugprintf("tty_wait: POLLIN\n") ;
	    } else
	        debugprintf("tty_wait: returned due to %08X\n",
	            fds[0].revents) ;
	} /* end if */
#endif /* CF_DEBUGS */

	if (rs == SR_INTR) {

#if	CF_DEBUGS
	    debugprintf("tty_wait: u_poll() interrupt rs=%d\n",rs) ;
#endif

	    goto loop ;
	}

/* do the system-call read */

	if ((rs >= 0) && (fds[0].revents & POLLIN)) {

	    rs = u_read(op->fd,cbuf,TTY_READCHARS) ;
	    len = rs ;

#if	CF_DEBUGS
	    debugprintf("tty_wait: u_read() rs=%d\n",rs) ;
#endif

	}  /* end if */

#if	CF_DEBUGS
	debugprintf("tty_wait: return or not, rs=%d\n",rs) ;
#endif

enter:
	if (rs < 0)
	    goto ret0 ;

	op->loopcount += 1 ;
	if (len < 0)
	    goto ret0 ;

	if (len == 0) {

	    daytime = time(NULL) ;

	    if (op->timeout >= 0) {

#if	CF_DEBUGS
	        debugprintf("tty_wait: timeout check timeout=%d\n",
	            op->timeout) ;
#endif

	        op->timeout -= (daytime - lasttime) ;
	        if (op->timeout <= 0)
	            return SR_OK ;

	    }

	    if ((! f_starting) && 
	        ((daytime - op->basetime) >= MON_INTERVAL)) {

#if	CF_DEBUGS
	        debugprintf("tty_wait: loop check\n") ;
#endif

	        f_looping = (op->loopcount > MAXLOOPS) ;
	        op->basetime = daytime ;
	        op->loopcount = 0 ;
	        if (f_looping) {

#if	CF_DEBUGS
	            debugprintf("tty_wait: looping failure\n") ;
#endif

	            rs = SR_NOANODE ;
	            goto ret0 ;
	        }

	    } /* end if (looping check) */

	} else {

/* skip over leading zero characters fabricated by Solaris SVR4 */

	    for (i = 0 ; (i < len) && (cbuf[i] == '\0') ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("tty_wait: %d zero characters from Solaris!\n",
	            i) ;
#endif

	    } /* end for */

/* call the Receive Interrupt Service Routine with what we do have */

	    if ((len - i) > 0)
	        rs = tty_risr(op,(cbuf + i),(len - i)) ;

	} /* end if */

/* should we go around again? */

	lasttime = daytime ;
	if ((rs >= 0) &&
	    (len <= 0) && ((op->timeout < 0) || (op->timeout > 0)))
	    goto loop ;

/* we-re out of here */
ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (tty_wait) */


/* check for receiver got some thing */
static int tty_risr(op,buf,buflen)
UTERM		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	c ;
	int	i ;
	int	f_dle = FALSE ;

	char	ch ;


	for (i = 0 ; i < buflen ; i += 1) {

	    c = (buf[i] & 0xff) ;
	    switch (c) {

	    case CH_XOFF:
	        op->f.suspend = TRUE ;
	        break ;

	    case CH_XON:
	        op->f.suspend = FALSE ;
	        break ;

	    case CH_SO:
	        if (op->f.co) {
	            op->f.co = FALSE ;
	        } else {
	            op->f.co = TRUE ;
	            rs = tty_echo(op," ^O\r\n",5) ;
	        }
	        break ;

	    case CH_ETX:
	        rs = tty_echo(op," ^C\r\n",5) ;
	        op->f.cc = TRUE ;
	        op->f.rw = TRUE ;
	        break ;

	    case CH_CY:
	        rs = tty_echo(op," ^Y\r\n",5) ;
	        op->f.cc = TRUE ;
	        op->f.rw = TRUE ;
	        break ;

	    case CH_DLE:
	        f_dle = TRUE ;
	        break ;

	    default:
	        if ((! op->f.read) && (c == CH_CAN))  {
	            while (charq_rem(&op->taq,&ch) >= 0) ;
	        } else {
	            rs = charq_ins(&op->taq,c) ;
	            op->f.rw = TRUE ;
	        }
		break ;

	    } /* end switch */

	    if (rs < 0)
	        break ;

	} /* end for */

	return (rs >= 0) ? f_dle : rs ;
}
/* end subroutine (tty_risr) */


/* echo */
static int tty_echo(op,buf,buflen)
UTERM		*op ;
char		buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;

#if	CF_DEBUGS
	char	hexbuf[HEXBUFLEN + 1] ;
#endif


	if (buflen < 0)
	    buflen = strlen(buf) ;

#if	CF_DEBUGS
	{
		int	sl ;
	debugprintf("tty_echo: buflen=%d\n",buflen) ;
	sl = mkhexstr(hexbuf,HEXBUFLEN,buf,MIN(buflen,20)) ;
	debugprintf("tty_echo: buf= %t\n",hexbuf,sl) ;
	}
#endif

	if (buflen > 0)
	    rs = u_write(op->fd,buf,buflen) ;

	return rs ;
}
/* end subroutine (tty_echo) */


static int sinotprint(buf,buflen)
const char	buf[] ;
int		buflen ;
{
	int	c ;
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; i < buflen ; i += 1) {

	    c = (buf[i] & 0xff) ;
	    f = isprintlatin(c) ;
	    f = f || (c == CH_SI) || (c == CH_SO) ;
	    if (! f)
	        break ;

	} /* end for */

	return (f) ? -1 : i ;
}
/* end subroutine (sinotprint) */



