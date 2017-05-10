/* utty */

/* "UNIX Terminal" helper routines */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_SIGNAL	1	/* 1=I handle, 0=UNIX handles */
#define	CF_BADTIME	0	/* Solaris timeout screwup */
#define	CF_FIRSTREAD	1	/* perform an initial 'read()' ? */


/* revision history:

	= 1985-02-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines provide a stand-alone TTY environment.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stropts.h>
#include	<poll.h>
#include	<termios.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<charq.h>
#include	<localmisc.h>

#include	"vs.h"
#include	"ucb.h"


/* local defines */

#define	TTY_MINCHARS	0	/* minimum characters to get */
#define	TTY_MINTIME	5	/* minimum time (x100 milliseconds) */

#define	TTY_READCHARS	20
#define	WBUFLEN		40	/* maximum output write buffer */

#define	MON_INTERVAL	3	/* the error monitoring interval */
#define	MAXLOOPS	300	/* maximum loops in monitor interval */

#define	TA_SIZE		256
#define	EC_SIZE		256

/* status mask bits */

#define	USM_RI		0x10	/* read indication ? */
#define	USM_TI		0x20	/* terminal interrupt ? */
#define	USM_SUS		0x40	/* suspend */
#define	USM_READ	0x80	/* read */

/* output priorities */

#define	OPM_WRITE	0x01
#define	OPM_READ	0x02
#define	OPM_ECHO	0x04
#define	OPM_ANY		0x07

#define	OPV_WRITE	0
#define	OPV_READ	1
#define	OPV_ECHO	2


/* externals subroutines */


/* external variables */


/* local program structures */


/* forward references */

static int	tty_wps(), tty_wait(), tty_echo(), tty_risr(), tty_tisr() ;


/* static data */

static uchar	dterms[] = {
	0xFF, 0xF8, 0xC0, 0xFE,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;





/* TTY initialization routine */
int ut_init(fd)
int	fd ;
{
	struct termios	*tp ;

	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("ut_init: entered\n") ;
#endif

	if ((rs = vs_initfd(fd,&fdhp)) <= 0)
	    return rs ;

	if ((rs = uc_malloc(sizeof(struct ucb),&ucbp)) < 0)
	    return rs ;

	fdhp->ucbp = ucbp ;
	tp = &ucbp->ts_new ;

#if	CF_DEBUGS
	debugprintf("ut_init: passed 'vs_initfd()'\n") ;

	malloctest(26) ;
#endif

	ucbp->fd = fd ;

	ucbp->stat = 0 ;
	ucbp->f_co = FALSE ;
	ucbp->f_cc = FALSE ;

	ucbp->mode = 0 ;
	ucbp->loopcount = 0 ;
	(void) time(&ucbp->basetime) ;

/* initialize the UNIX line */

#if	CF_DEBUGS
	debugprintf("ut_init: about to call 'ioctl'\n") ;

	malloctest(26) ;
#endif

#ifdef	COMMENT
	if ((rs = u_ioctl(fd,TCGETS,&ucbp->ts_old)) < 0)
	    return rs ;
#else
	if ((rs = uc_tcgetattr(fd,&ucbp->ts_old)) < 0)
	    return rs ;
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("ut_init: called 'ioctl'\n") ;

	malloctest(26) ;
#endif

	ucbp->ts_new = ucbp->ts_old ;

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
	if (access("/usr/sbin",R_OK) >= 0)
	    tp->c_cc[VMIN] = 1 ;

	else
	    tp->c_cc[VMIN] = TTY_MINCHARS ;
#else
	tp->c_cc[VMIN] = TTY_MINCHARS ;
#endif /* CF_BADTIME */

	tp->c_cc[VTIME] = TTY_MINTIME ;

#if	CF_DEBUGS
	debugprintf("ut_init: MINCHARS=%d MINTIME=%d\n",
	    tp->c_cc[VEOF],tp->c_cc[VEOL]) ;
#endif

/* set the new attributes */

#if	CF_DEBUGS
	debugprintf("ut_init: about to call 'ioctl' 2\n") ;
#endif

	u_ioctl(fd,TCSETSW,tp) ;

#if	CF_DEBUGS
	debugprintf("ut_init: called 'ioctl' 2\n") ;
#endif


/* initialize the TA buffer */

#if	CF_DEBUGS
	debugprintf("ut_init: TA buffer\n") ;
#endif

	cqinit(&ucbp->taq,TA_SIZE) ;

/* initialize the EC buffer */

#if	CF_DEBUGS
	debugprintf("ut_init: EC buffer\n") ;

	malloctest(26) ;
#endif

	cqinit(&ucbp->ecq,EC_SIZE) ;

#if	CF_DEBUGS
	debugprintf("ut_init: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (ut_init) */


/* ensure settings */
int ut_ensure(fd)
int	fd ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs ;


	if ((rs = vs_initfd(fd,&fdhp)) < 0)
	    return rs ;

	ucbp = fdhp->ucbp ;

#ifdef	COMMENT
	rs = u_ioctl(fd,TCSETSW,&ucbp->ts_new) ;
#else
	rs = uc_tcsetattr(fd,TCSADRAIN,&ucbp->ts_new) ;
#endif

	return rs ;
}
/* end subroutine (ut_ensure) */


/* restore settings */
int ut_restore(fd)
int	fd ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs ;


	if ((rs = vs_initfd(fd,&fdhp)) < 0)
	    return rs ;

	ucbp = fdhp->ucbp ;

#ifdef	COMMENT
	rs = u_ioctl(fd,TCSETSW,&ucbp->ts_old) ;
#else
	rs = uc_tcsetattr(fd,TCSADRAIN,&ucbp->ts_old) ;
#endif

	return rs ;
}
/* end subroutine (ut_restore) */


/* close this file */
int ut_close(fd)
int	fd ;
{
	int	rs ;


	if ((rs = ut_restore(fd)) < 0) 
		return rs ;

	if ((rs = vs_freefd(fd)) < 0) 
		return rs ;

	return u_close(fd) ;
}
/* end subroutine (ut_close) */


/* control function */
int ut_control(fd,arg1,arg2)
int	fd ;
int	arg1, arg2 ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs ;
	int	fc ;


#if	CF_DEBUGS
	debugprintf("ut_control: entered\n") ;
#endif

	if ((rs = vs_initfd(fd,&fdhp)) < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("ut_control: entered 2\n") ;
#endif

	ucbp = fdhp->ucbp ;
	switch (arg1 & FM_MASK) {

	case fm_setmode:
	    ucbp->mode = (int) arg2 ;
	    break ;

	case fm_getmode:
	    *((int *) arg2) = ucbp->mode ;
	    break ;

	case fm_reestablish:
	    rs = u_ioctl(fd,TCSETSW,&ucbp->ts_new) ;

	    break ;

	default:
	    return SR_INVAL ;

	} /* end switch */

#if	CF_DEBUGS
	debugprintf("ut_control: exiting\n") ;
#endif

	return rs ;
}
/* end subroutine (ut_control) */


/* status function */
int ut_status(fd,arg1,arg2)
int	fd ;
int arg1, arg2 ;
{


	return SR_NOENT ;
}
/* end subroutine (ut_status) */


/* subroutine to read a line from terminal */
int ut_read(fd,buf,len)
int	fd ;
uchar	*buf ;
int	len ;
{


	return ut_reade(fd,0,buf,len,0,NULL,NULL,0) ;
}
/* end subroutine (ut_read) */


/* read a line from the terminal with extended parameters */
int ut_reade(fd,fc,buf,ulen,timeout,terms,pbuf,plen)
int	fd ;
int	fc ;
uchar	*buf ;
int	ulen ;
int	timeout ;
char	*terms ;
uchar	*pbuf ;
int	plen ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs, count ;

	uchar	c ;			/* MUST !! be a character */


#if	CF_DEBUGS
	debugprintf("ut_reade: entered, timeout=%d\n",timeout) ;
#endif

	if ((rs = vs_initfd(fd,&fdhp)) < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("ut_reade: rs=%d\n",rs) ;
#endif

	ucbp = fdhp->ucbp ;
	fc |= ucbp->mode ;
	if (fc & fm_rawin) 
		fc |= fm_nofilter ;

	if (fc & fm_noecho) 
		fc |= fm_notecho ;

	if (! (fc & fm_timed)) 
		timeout = -1 ;

	if (terms == NULL) 
		terms = (char *) dterms ;

	if (pbuf == NULL) 
		plen = 0 ;

	ucbp->f_cc = FALSE ;
	ucbp->f_co = FALSE ;		/* cancel ^O effect */

	ucbp->stat |= USM_READ ;

/* top of further access */
top:
	if (plen > 0)
	    tty_wps(ucbp,pbuf,plen) ; /* write out prompt if present */

	count = 0 ;

/* check TA buffer first */
next:

#if	CF_DEBUGS
	debugprint("ut_reade: at next\n") ;
#endif

	if (charq_rem(&ucbp->taq,&c) != CHARQ_RUNDER)
	    goto read_got ;

/* wait for character while timing */

#if	CF_DEBUGS
	debugprint("ut_reade: in front of wait\n") ;
#endif

	if ((rs = tty_wait(ucbp,timeout)) < 0)
	    return rs ;

	if (ucbp->f_cc)
	    return SR_CANCELED ;

	if ((timeout >= 0) && (ucbp->timeout <= 0))
	    return count ;

	if (ucbp->f_rw)
	    goto next ;

	return count ;

/* got a character */
read_got:
	buf[count] = c ;

#if	CF_DEBUGS
	debugprint("ut_reade: got a character\n") ;
#endif

/* check for terminator */

	if (BATST(terms,c))
	    goto read_term ;

/* check for a filter character */

	if (fc & fm_nofilter) goto norm ;

	if (c == CH_NAK) goto read_nak ;

	if (c == CH_DEL) goto read_del ;

	if (c == CH_BS) goto read_del ;

	if (c == CH_DC2) goto read_ref ;

	if (c == CH_CAN) goto read_can ;

/* normal character */
norm:
	if ((! (fc & fm_noecho)) && 
	    (((c & 0x60) && ((c & 0x7F) != 0x7F)) || (c == '\t')))
	    tty_echo(ucbp,&c,1) ;

	count += 1 ;
	if (count < ulen) goto next ;

/* this read operation is done */
read_done:
	ucbp->stat &= (~ USM_READ) ;
	return count ;

/* handle a terminator character */
read_term:

#if	CF_DEBUGS
	debugprint("ut_reade: got a terminator\n") ;
#endif

	if (c == CH_CR) goto read_cr ;

	if (c == CH_ESC) goto read_esc ;

/* we have a normal terminator */
read_tstore:

#if	CF_DEBUGS
	debugprint("ut_reade: storing\n") ;
#endif

	buf[count++] = c ;
	goto read_done ;


/* we read a CR */
read_cr:
	if (! ((fc & fm_notecho) || (fc & fm_noecho)))
	    tty_echo(ucbp,"\r\n",2) ;

	if (fc & fm_nofilter) goto read_tstore ;

	c = CH_LF ;
	goto read_tstore ;

/* we got an ESC character */
read_esc:
	if (! ((fc & fm_notecho) || (fc & fm_noecho)))
	    tty_echo(ucbp,"$",1) ;

	goto read_tstore ;

/* handle a restart line */
read_nak:
	if (! (fc & fm_noecho)) tty_echo(ucbp," ^U\r\n",5) ;

	goto top ;

/* handle a delete character */
read_del:
	if (count == 0) goto next ;

	count -= 1 ;
	if (! (fc & fm_noecho)) tty_echo(ucbp,"\b \b",3) ;

	goto next ;

/* we got a ^R */
read_ref:
	if (fc & fm_noecho) goto next ;

	tty_echo(ucbp," ^R\r\n",5) ;

	tty_wps(ucbp,pbuf,plen) ;/* re-print out the prompt string */

	tty_wps(ucbp,buf,count) ;	/* print out the input line */

	goto next ;

read_can:
	if (! (fc & fm_noecho)) tty_echo(ucbp," ^X\r\n",5) ;

	goto top ;
}
/* end subroutine (ut_reade) */


/* write routine */
int ut_write(fd,ubuf,ulen)
int	fd ;
uchar	*ubuf ;
int	ulen ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	struct termios	*tp ;

	int	rs ;
	int	lenr, blen ;

	uchar	buf[WBUFLEN], *bp ;
	uchar	*ubp = ubuf ;
	char	dbuf[100], *dbp ;


	if ((rs = vs_initfd(fd,&fdhp)) <= 0)
	    return rs ;

	ucbp = fdhp->ucbp ;
	if (ucbp->f_co) return 0 ;

	if (ucbp->mode & fm_rawout) {

	    lenr = ulen ;
	    while (lenr > 0) {

	        blen = (lenr < (WBUFLEN - 2)) ? lenr : (WBUFLEN - 2) ;

	        rs = write(ucbp->fd,ubp,blen) ;

	        if (rs < 0) break ;

	        lenr -= blen ;
	        ubp += blen ;
	    }

	} else {

	    bp = buf ;
	    lenr = ulen ;
	    blen = 0 ;
	    while (lenr > 0) {

	        if (*ubp == '\n') {

	            *bp++ = '\r' ;
	            blen += 1 ;

	        }

	        *bp++ = *ubp++ ;
	        lenr -= 1 ;
	        blen += 1 ;

	        if (blen >= (WBUFLEN - 2)) {

	            rs = write(ucbp->fd,buf,blen) ;

	            if (rs < 0) break ;

	            bp = buf ;
	            blen = 0 ;
	        }

	    } /* end while */

	    if (blen > 0) write(ucbp->fd,buf,blen) ;

	} /* end outer if */

	return ulen ;
}
/* end subroutine (ut_write) */


/* cancel a terminal request */
int ut_cancel(fd,fc,cparam)
int	fd ;
int	fc, cparam ;
{


	return SR_OK ;
}
/* end subroutine (ut_cancel) */


/* poll for a character */
int ut_poll(fd)
int	fd ;
{
	struct vsfd_header	*fdhp ;

	struct ucb	*ucbp ;

	int	rs ;


	if ((rs = vs_initfd(fd,&fdhp)) <= 0)
	    return rs ;

	ucbp = fdhp->ucbp ;
	if ((rs = tty_wait(ucbp,0)) < 0)
	    return rs ;

	if (ucbp->f_cc)
	    return SR_CANCELED ;

	return SR_OK ;
}
/* end subroutine (ut_poll) */



/* INTERNAL SUBROUTINES */



/* write out a prompt string */
static int tty_wps(ucbp,buf,len)
struct ucb	*ucbp ;
uchar	*buf ;
int	len ;
{

	if (len > 0)
	    return u_write(ucbp->fd,buf,(int) len) ;

	return 0 ;
}
/* end subroutine (tty_wps) */


/* wait for a character to arrive */
static int tty_wait(ucbp,timeout)
struct ucb	*ucbp ;
int		timeout ;
{
	struct pollfd	fds[2] ;

	time_t	daytime, lasttime ;

	int	i, len ;
	int	rs, nfds ;
	int	polltime ;
	int	f_looping ;

	uchar	cbuf[TTY_READCHARS + 1] ;


/* initialize the POLL structure */

	fds[0].fd = ucbp->fd ;
	fds[0].events = POLLIN ;
	fds[1].fd = -1 ;
	fds[1].events = POLLIN ;
	nfds = 1 ;

	polltime = MIN(3000,(timeout * 1000)) ;

	len = 0 ;
	ucbp->timeout = timeout ;
	time(&daytime) ;

	lasttime = daytime ;

#if	CF_FIRSTREAD

/* try it out the first time */

	    rs = len = u_read(ucbp->fd,cbuf,TTY_READCHARS) ;

#if	CF_DEBUGS
	    debugprintf("tty_wait: read returned, len=%d\n",len) ;
#endif

	goto enter ;

#endif /* CF_FIRSTREAD */


/* loop it */
loop:

#if	CF_DEBUGS
	debugprintf("tty_wait: top loop, timeout=%d\n",ucbp->timeout) ;
#endif

/* zero out the buffer so that we can tell if Solaris screws us later */

	for (i = 0 ; i < TTY_READCHARS ; i += 1)
	    cbuf[i] = '\0' ;

/* wait for read attention */

	rs = u_poll(fds,1L,polltime) ;

#if	CF_DEBUGS
	debugprintf("tty_wait: poll rs=%d\n",rs) ;

	if (rs > 0) {

	    if (fds[0].revents & POLLIN)
	        debugprintf("tty_wait: POLLIN\n") ;

	    else
	        debugprintf("tty_wait: returned due to %08X\n",
	            fds[0].revents) ;

	}
#endif /* CF_DEBUGS */

/* do the system call read */

	if ((rs >= 0) && (fds[0].revents & POLLIN)) {

	    len = u_read(ucbp->fd,cbuf,TTY_READCHARS) ;

#if	CF_DEBUGS
	    debugprintf("tty_wait: read returned, len=%d\n",len) ;
#endif

	    if (len < 0)
	        rs = len ;

	}  /* end if */

#if	CF_DEBUGS
	debugprintf("tty_wait: return or not, rs=%d\n",rs) ;
#endif

/* enter here is we already have some characters ! */
enter:
	if (rs < 0)
	    return rs ;

	ucbp->loopcount += 1 ;
	if (len < 0)
	    return len ;

	if (len == 0) {

	    time(&daytime) ;

	    if (ucbp->timeout >= 0) {

#if	CF_DEBUGS
	    debugprintf("tty_wait: timeout check, timeout=%d\n",ucbp->timeout) ;
#endif

	        ucbp->timeout -= (daytime - lasttime) ;
	        if (ucbp->timeout <= 0)
	            return SR_OK ;

	    }

	    if ((daytime - ucbp->basetime) >= MON_INTERVAL) {

	        f_looping = (ucbp->loopcount > MAXLOOPS) ;
	        ucbp->basetime = daytime ;
	        ucbp->loopcount = 0 ;
	        if (f_looping)
	            return SR_NOENT ;

	    }

	} else {

/* skip over leading zero characters fabricated by Solaris SVR4 */

	    for (i = 0 ; (i < len) && (cbuf[i] == '\0') ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("tty_wait: %d zero characters from Solaris !\n",
		i) ;
#endif

		} /* end for */

/* call the Receive Interrupt Service Routine with what we do have */

	    if ((len - i) > 0)
	        tty_risr(ucbp,cbuf + i,len - i) ;

	} /* end if */

/* should we go around again ? */

	lasttime = daytime ;
	if ((len <= 0) &&
		((ucbp->timeout < 0) || (ucbp->timeout > 0))) goto loop ;

/* we-re out of here */

	return SR_OK ;
}
/* end subroutine (tty_wait) */


/* check for receiver got some thing */
static int tty_risr(ucbp,buf,len)
struct ucb	*ucbp ;
uchar	buf[] ;
int	len ;
{
	int	i = 0 ;
	int	c ;


next:
	if (i >= len) return ;

	c = buf[i++] ;

	if (c == CH_XOFF) goto che_xoff ;

	if (c == CH_XON) goto che_xon ;

	if (c == CH_SO) goto che_so ;

	if (c == CH_ETX) goto che_cc ;

	if (c == CH_CY) goto che_cy ;

	if (c == CH_DLE) goto che_dle ;

	if ((! (ucbp->stat & USM_READ)) && (c == CH_CAN)) 
		goto che_can ;

/* store the character in the type-ahead queue */

	charq_ins(&ucbp->taq,c) ;

	ucbp->f_rw = TRUE ;
	goto	done ;

che_xoff:
	ucbp->stat |= USM_SUS ;
	ucbp->stat &= (~ USM_TI) ;
	goto	done ;

che_xon:
	ucbp->stat &= (~ USM_SUS) ;
	ucbp->stat |= USM_TI ;
	goto	done ;

che_so:
	if (ucbp->f_co) {

	    ucbp->f_co = FALSE ;

	} else {

	    tty_echo(ucbp," ^O\r\n",5) ;

	    ucbp->f_co = TRUE ;

	}

	goto	done ;

/* handle control C */
che_cc:
	tty_echo(ucbp," ^C\r\n",5) ;

	ucbp->f_cc = TRUE ;
	ucbp->f_rw = TRUE ;
	goto	done ;

/* handle control Y */
che_cy:
	tty_echo(ucbp," ^Y\r\n",5) ;

	ucbp->f_cc = TRUE ;
	ucbp->f_rw = TRUE ;
	goto	done ;

/* data link excape */
che_dle:
	return ;

/* handle a line cancel */
che_can:
	while (charq_rem(&ucbp->taq,&c) != CHARQ_RUNDER) ;

done:
	goto	next ;
}
/* end subroutine (tty_risr) */


/* echo */
static int tty_echo(ucbp,buf,len)
struct ucb	*ucbp ;
uchar	buf[] ;
int	len ;
{


	return write(ucbp->fd,buf,len) ;
}
/* end subroutine (echo) */



