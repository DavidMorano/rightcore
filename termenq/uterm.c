/* uterm */

/* "UNIX Terminal" helper routines */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */
#define	CF_SIGNAL	1	/* 1={local handle}, 0={UNIX® handles} */
#define	CF_FIRSTREAD	0	/* perform an initial 'read()'? */
#define	CF_WRITEATOM	1	/* atomic write */
#define	CF_SUBUNIX	1	/* allow UNIX® to handle Control-Z */


/* revision history:

	= 1998-02-01, David A­D­ Morano
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
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<charq.h>
#include	<vecobj.h>
#include	<buffer.h>
#include	<sbuf.h>
#include	<sigign.h>
#include	<localmisc.h>

#include	"uterm.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#define	UTERM_MAGIC	0x33442281

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	TTY_MINCHARS	0	/* minimum characters to get */
#define	TTY_MINTIME	5	/* minimum time (x100 milliseconds) */

#define	TTY_READCHARS	20

#define	WBUFLEN		40	/* maximum output write buffer */

#define	TO_HANGUP	3	/* the error monitoring interval */
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

extern int	tcsetmesg(int,int) ;
extern int	tcsetbiff(int,int) ;
extern int	tcgetlines(int) ;
extern int	tcsetlines(int,int) ;
extern int	isprintlatin(int) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	uterm_attrbegin(UTERM *) ;
static int	uterm_attrend(UTERM *) ;
static int	uterm_qbegin(UTERM *) ;
static int	uterm_qend(UTERM *) ;
static int	uterm_controlmode(UTERM *) ;
static int	uterm_writeproc(UTERM *,const char *,int) ;

static int	tty_wait() ;
static int	tty_echo() ;
static int	tty_risr(UTERM *,const char *,int) ;
static int	tty_wps(UTERM *,const char *,int) ;
static int	tty_loadchar(UTERM *,const char *,int) ;

static int	sinotprint(const char *,int) ;


/* local variables */

static const uchar	uterms[] = {
	0xEF, 0xFC, 0xC0, 0xFE,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;

static const int	sigouts[] = {
	SIGTTOU,
	0
} ;


/* exported subroutines */


/* TTY initialization routine */
int uterm_start(UTERM *op,int fd)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (fd < 0) return SR_INVALID ;

	memset(op,0,sizeof(UTERM)) ;
	op->ti_start = time(NULL) ;
	op->fd = fd ;
	op->uid = -1 ;
	memcpy(op->rterms,uterms,32) ;

	if ((rs = uc_tcgetpgrp(fd)) == SR_NOTTY) {
	    op->f.noctty = TRUE ;
	    rs = SR_OK ;
	}

	if (rs >= 0) {
	    if ((rs = uterm_attrbegin(op)) >= 0) {
		if ((rs = uterm_qbegin(op)) >= 0) {
		    op->magic = UTERM_MAGIC ;
		}
	 	if (rs < 0)
		    uterm_attrend(op) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("uterm_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uterm_start) */


int uterm_finish(UTERM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	rs1 = uterm_qend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uterm_attrend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (uterm_finish) */


int uterm_suspend(UTERM *op)
{

	return uterm_restore(op) ;
}
/* end subroutine (uterm_suspend) */


int uterm_resume(UTERM *op)
{

	return uterm_ensure(op) ;
}
/* end subroutine (uterm_resume) */


/* restore settings */
int uterm_restore(UTERM *op)
{
	struct termios	*attrp ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	attrp = &op->ts_old ;
	if ((rs = uc_tcsetattr(op->fd,TCSADRAIN,attrp)) >= 0) {
	    rs = op->fd ;
	}

	return rs ;
}
/* end subroutine (uterm_restore) */


/* ensure settings */
int uterm_ensure(UTERM *op)
{
	struct termios	*attrp ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	attrp = &op->ts_new ;
	rs = uc_tcsetattr(op->fd,TCSADRAIN,attrp) ;

	return rs ;
}
/* end subroutine (uterm_ensure) */


/* control function */
int uterm_control(UTERM *op,int cmd,...)
{
	int		rs = SR_OK ;
	int		iw, *iwp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("uterm_control: cmd=%u\n",
	    (cmd & FM_MASK)) ;
#endif

	{
	    va_list	ap ;
	    const int	fc = (cmd & FM_CMDMASK) ;
	    va_begin(ap,cmd) ;
	    switch (fc) {
	    case utermcmd_noop:
		break ;
	    case utermcmd_getuid:
		if (op->uid < 0) {
		    struct ustat	sb ;
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
			op->uid = sb.st_uid ;
		    }
		}
		if (rs >= 0) rs = op->uid ;
	        break ;
	    case utermcmd_getsid:
	        rs = uc_tcgetsid(op->fd) ;
	        break ;
	    case utermcmd_getmode:
	        iwp = (int *) va_arg(ap,int *) ;
	        if (iwp != NULL) *iwp = op->mode ;
	        break ;
	    case utermcmd_setmode:
	        iw = (int) va_arg(ap,int) ;
	        op->mode = iw ;
		rs = uterm_controlmode(op) ;
	        break ;
	    case utermcmd_setrterms:
		{
	            const uchar	*rp = (uchar *) va_arg(ap,uchar *) ;
		    memcpy(op->rterms,rp,32) ;
		}
		break ;
	    case utermcmd_reestablish:
		{
		    struct termios	*attrp = &op->ts_new ;
	            rs = uc_tcsetattr(op->fd,TCSADRAIN,attrp) ;
		}
	        break ;
	    case utermcmd_getmesg:
		rs = uterm_getmesg(op) ;
		break ;
	    case utermcmd_setmesg:
		{
		    int	f = (int) va_arg(ap,int) ;
	            rs = tcsetmesg(op->fd,f) ;
		}
		break ;
	    case utermcmd_getbiff:
		rs = uterm_getbiff(op) ;
		break ;
	    case utermcmd_setbiff:
		{
		    const int	f = (int) va_arg(ap,int) ;
	            rs = tcsetbiff(op->fd,f) ;
		}
		break ;
	    case utermcmd_getpgrp:
	        rs = uc_tcgetpgrp(op->fd) ;
	        break ;
	    case utermcmd_setpgrp:
		{
		    SIGIGN	si ;
		    if ((rs = sigign_start(&si,sigouts)) >= 0) {
		        const pid_t	pgrp = (const pid_t) va_arg(ap,pid_t) ;
	                rs = uc_tcsetpgrp(op->fd,pgrp) ;
		        sigign_finish(&si) ;
		    } /* end if (sigign) */
		}
	        break ;
	    case utermcmd_getpop:
		rs = uterm_getpop(op) ;
		break ;
	    case utermcmd_setpop:
		{
		    const int	v = (int) va_arg(ap,int) ;
	            rs = uterm_setpop(op,v) ;
		}
		break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    va_end(ap) ;
	} /* end block (variable arguments) */

#if	CF_DEBUGS
	debugprintf("uterm_control: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uterm_control) */


/* status function */
int uterm_status(UTERM *op,int cmd,...)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	{
	    va_list	ap ;
	    va_begin(ap,cmd) ;
	    switch (cmd) {
	    case utermcmd_noop:
	        break ;
	    case utermcmd_setmesg:
	        {
	            int	v = (int) va_arg(ap,int) ;
	            rs = tcsetmesg(op->fd,v) ;
	        }
	        break ;
	    case utermcmd_getlines:
	        rs = tcgetlines(op->fd) ;
	        break ;
	    case utermcmd_setlines:
	        {
	            const int	v = (int) va_arg(ap,int) ;
	            rs = tcsetlines(op->fd,v) ;
	        }
	        break ;
	    case utermcmd_getsid:
	        rs = uc_tcgetsid(op->fd) ;
	        break ;
	    case utermcmd_getpgrp:
	        rs = uc_tcgetpgrp(op->fd) ;
	        break ;
	    case utermcmd_setpgrp:
	        {
	            const pid_t	pgrp = (pid_t) va_arg(ap,pid_t) ;
	            rs = uc_tcsetpgrp(op->fd,pgrp) ;
	        }
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    va_end(ap) ;
	} /* end block (variable arguments) */

	return rs ;
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
	int		rs = SR_OK ;
	int		ch = -1 ;
	int		count = 0 ;
	const uchar	*terms ;
	char		qbuf[2] ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("uterm_reade: ent to=%d\n",timeout) ;
#endif

	fc |= op->mode ;
	if (fc & fm_rawin) fc |= fm_nofilter ;
	if (fc & fm_noecho) fc |= fm_notecho ;

	terms = op->rterms ;
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

	    while (charq_rem(&op->taq,qbuf) == SR_EMPTY) {

	        rs = tty_wait(op,timeout) ;

#if	CF_DEBUGS
	        debugprintf("uterm_reade: tty_wait() rs=%d\n",rs) ;
#endif

	        if ((timeout >= 0) && (op->timeout <= 0)) break ;
	        if (rs < 0) break ;
	    } /* end while */

	    if (rs < 0)
	        break ;

	    if ((timeout >= 0) && (op->timeout <= 0))
	        break ;

	    if (op->f.cc)
	        break ;

	    ch = MKCHAR(qbuf[0]) ;
	    rbuf[count] = ch ;

#if	CF_DEBUGS
	    debugprintf("uterm_reade: got a character ch=%c (%02X)\n",
	        (isprintlatin(ch) ? ch : ' '),ch) ;
#endif

/* check for terminator */

	    if (BATST(terms,ch))
	        break ;

/* check for a filter character */

	    if (! (fc & fm_nofilter)) {
	        switch (ch) {
	        case CH_EOT:
	            if (count == 0)
	                f_eot = TRUE ;
	            ch = -1 ;
	            break ;
	        case CH_NAK:
	            if (! (fc & fm_noecho))
	                rs = tty_echo(op," ^U\r\n",5) ;
	            ch = -1 ;
	            break ;
	        case CH_DEL:
	        case CH_BS:
	            if (count > 0) {
	                count -= 1 ;
	                if (! (fc & fm_noecho))
	                    rs = tty_echo(op,"\b \b",3) ;
	            }
	            ch = -1 ;
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
	            ch = -1 ;
	            break ;
	        case CH_CAN:
	            if (! (fc & fm_noecho))
	                rs = tty_echo(op," ^X\r\n",5) ;
	            ch = -1 ;
	            break ;
	        } /* end switch */
	    } /* end if (filtering) */

	    if (ch >= 0) {
	        if ((! (fc & fm_noecho)) && isprintlatin(ch)) {
		    char	ebuf[2] ;
		    ebuf[0] = ch ;
	            rs = tty_echo(op,ebuf,1) ;
	        }
	        count += 1 ;
	    } /* end if (normal character) */

	    if (f_eot) break ;
	} /* end while */

/* exit processing */

#if	CF_DEBUGS
	debugprintf("uterm_reade: exit processing rs=%d count=%u\n",
	    rs,count) ;
#endif

	if ((rs >= 0) && (ch >= 0) && BATST(terms,ch)) {
	    switch (ch) {
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

#if	CF_DEBUGS
	debugprintf("uterm_reade: ret rs=%d count=%u\n",rs,count) ;
#endif

	op->f.read = FALSE ;
	return (rs >= 0) ? count : rs ;
}
/* end subroutine (uterm_reade) */


int uterm_read(UTERM *op,char *rbuf,int rlen)
{

	return uterm_reade(op,rbuf,rlen,-1,0,NULL,NULL) ;
}
/* end subroutine (uterm_read) */


/* write routine */
int uterm_write(UTERM *op,cchar *wbuf,int wlen)
{
	int		rs = SR_OK ;
	int		tlen = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

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
/* ARGSUSED */
int uterm_cancel(UTERM *op,int fc,int cparam)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	return rs ;
}
/* end subroutine (uterm_cancel) */


/* poll for a character */
int uterm_poll(UTERM *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;

	if ((rs = tty_wait(op,0)) >= 0) {
	    if (op->f.cc) rs = SR_CANCELED ;
	}

	return rs ;
}
/* end subroutine (uterm_poll) */


int uterm_getmesg(UTERM *op)
{
	struct ustat	sb ;
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;
	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    rs = ((sb.st_mode&S_IWGRP) != 0) ;
	}
	return rs ;
}
/* end subroutine (uterm_getmesg) */


int uterm_getbiff(UTERM *op)
{
	struct ustat	sb ;
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;
	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    rs = ((sb.st_mode&S_IXUSR) != 0) ;
	}
	return rs ;
}
/* end subroutine (uterm_getbiff) */


int uterm_getpop(UTERM *op)
{
	struct ustat	sb ;
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;
	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    int	v = 0 ;
	    if (sb.st_mode&S_IXUSR) v |= 0x01 ;
	    if (sb.st_mode&S_IWGRP) v |= 0x02 ;
	    rs = v ;
	}
	return rs ;
}
/* end subroutine (uterm_getpop) */


int uterm_setpop(UTERM *op,int v)
{
	struct ustat	sb ;
	int		rs ;
	int		rv = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != UTERM_MAGIC) return SR_NOTOPEN ;
	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    mode_t	fm = sb.st_mode ;
	    if (sb.st_mode&S_IXUSR) rv |= 0x01 ;
	    if (sb.st_mode&S_IWGRP) rv |= 0x02 ;
	    if (v != rv) {
		fm &= S_IXUSR ;
		if (v & 0x01) fm |= S_IXUSR ;
		fm &= S_IWGRP ;
		if (v & 0x02) fm |= S_IWGRP ;
		rs = u_fchmod(op->fd,fm) ;
	    } /* end if (needed update) */
	} /* end if (stat) */
	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (uterm_setpop) */


/* private subroutines */


static int uterm_attrbegin(UTERM *op)
{
	const int	fd = op->fd ;
	int		rs ;

	if ((rs = uc_tcgetattr(fd,&op->ts_old)) >= 0) {
	    struct termios	*tp = &op->ts_new ;
	    op->ts_new = op->ts_old ;

	tp->c_iflag &= 
	    (~ (INLCR | ICRNL | IUCLC | IXANY | ISTRIP | INPCK | PARMRK)) ;
	tp->c_iflag |= IXON ;

	tp->c_cflag &= (~ (CSIZE)) ;
	tp->c_cflag |= CS8 ;

	tp->c_lflag &= (~ (ICANON | ECHO | ECHOE | ECHOK | ECHONL)) ;
#if	CF_SIGNAL
	tp->c_lflag &= (~ (ISIG)) ;
#endif

	tp->c_oflag &= (~ (OCRNL | ONOCR | ONLRET)) ;

	tp->c_cc[VMIN] = TTY_MINCHARS ;
	tp->c_cc[VTIME] = TTY_MINTIME ;

	tp->c_cc[VINTR] = CH_ETX ;	/* Control-C */
	tp->c_cc[VQUIT] = CH_EM ;	/* Control-Y */
	tp->c_cc[VERASE] = CH_DEL ;	/* Delete */
	tp->c_cc[VKILL] = CH_NAK ;	/* Control-U */
	tp->c_cc[VSTART] = CH_DC1 ;	/* Control-Q */
	tp->c_cc[VSTOP] = CH_DC3 ;	/* Control-S */
	tp->c_cc[VSUSP] = CH_SUB ;	/* Control-Z */
	tp->c_cc[VREPRINT] = CH_DC2 ;	/* Control-R */
	tp->c_cc[VDISCARD] = CH_SO ;	/* Control-O */

#if	CF_DEBUGS
	debugprintf("uterm_start: MINCHARS=%u MINTIME=%u\n",
	    tp->c_cc[VEOF],tp->c_cc[VEOL]) ;
#endif

/* set the new attributes */

	    rs = uc_tcsetattr(fd,TCSADRAIN,tp) ;
	    if (rs < 0) {
		uc_tcsetattr(fd,TCSADRAIN,&op->ts_old) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (uterm_attrbegin) */


static int uterm_attrend(UTERM *op)
{
	const int	fd = op->fd ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = uc_tcsetattr(fd,TCSADRAIN,&op->ts_old) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (uterm_attrend) */


static int uterm_qbegin(UTERM *op)
{
	int		rs ;

	if ((rs = charq_start(&op->taq,TA_SIZE)) >= 0) {
	    rs = charq_start(&op->ecq,EC_SIZE) ;
	    if (rs < 0)
		charq_finish(&op->taq) ;
	}

	return rs ;
}
/* end subroutine (uterm_qbegin) */


static int uterm_qend(UTERM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = charq_finish(&op->taq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = charq_finish(&op->ecq) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (uterm_qend) */


static int uterm_controlmode(UTERM *op)
{
	const int	mode = op->mode ;
	int		rs = SR_OK ;
	op->f.nosig = ((mode & fm_nosig) != 0) ;
	return rs ;
}
/* end subroutine (uterm_controlmode) */


static int uterm_writeproc(UTERM *op,cchar *buf,int buflen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl = -1 ;
	int		tlen = 0 ;
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

	        if (bl > 0) {
	            buffer_buf(&pb,bp,bl) ;
	        }

	        rs = buffer_get(&pb,&sp) ;
	        sl = rs ;
	        if ((rs >= 0) && (sl > 0)) {
	            rs = u_write(op->fd,sp,sl) ;
		}

	        rs1 = buffer_finish(&pb) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (initialize buffer) */

	} else {
	    rs = u_write(op->fd,buf,buflen) ;
	}

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (uterm_writeproc) */


/* write out a prompt string */
static int tty_wps(UTERM *op,cchar *buf,int buflen)
{
	int		rs = SR_OK ;
	int		ci ;

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


static int tty_loadchar(UTERM *op,cchar *pbuf,int pbuflen)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

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
static int tty_wait(UTERM *op,int timeout)
{
	struct pollfd	fds[2] ;
	time_t		daytime, lasttime ;
	int		rs ;
	int		i ;
	int		len = 0 ;
	int		nfds ;
	int		polltime ;
	int		f_starting, f_looping ;
	char		cbuf[TTY_READCHARS+1] ;

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
	        ((daytime - op->ti_start) >= TO_HANGUP)) {

#if	CF_DEBUGS
	        debugprintf("tty_wait: loop check\n") ;
#endif

	        f_looping = (op->loopcount > MAXLOOPS) ;
	        op->ti_start = daytime ;
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

/* call the Receive-Interrupt-Service-Routine with what we do have */

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


/* check if the receiver got some thing */
static int tty_risr(UTERM *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i ;
	int		f_dle = FALSE ;

	for (i = 0 ; i < sl ; i += 1) {
	    int	ch = MKCHAR(sp[i]) ;
#if	CF_DEBUGS
	debugprintf("uterm/tty_risr: ch=%02X\n",ch) ;
#endif
	    if (op->f.nosig) {
	        rs = charq_ins(&op->taq,ch) ;
	        op->f.rw = TRUE ;
	    } else {
	        switch (ch) {
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
		    	if (! op->f.nosigecho)
	                rs = tty_echo(op," ^O\r\n",5) ;
	            }
	            break ;
	        case CH_ETX:
		    if (! op->f.nosigecho)
	            rs = tty_echo(op," ^C\r\n",5) ;
	            op->f.cc = TRUE ;
	            op->f.rw = TRUE ;
	            break ;
	        case CH_EM:
		    if (! op->f.nosigecho)
	            rs = tty_echo(op," ^Y\r\n",5) ;
	            op->f.cc = TRUE ;
	            op->f.rw = TRUE ;
	            break ;
	        case CH_DLE:
	            f_dle = TRUE ;
	            break ;
/* Control-Z (substitute) */
	        case CH_SUB:
		    if (op->f.noctty) {
		        op->f.cz = TRUE ;
		    } else {
#if	CF_SUBUNIX
		        rs = uc_raise(SIGTSTP) ;
#else
		        op->f.cz = TRUE ;
#endif
		    }
	            break ;
	        default:
	            if ((! op->f.read) && (ch == CH_CAN))  {
	                char	dch ;
	                while (charq_rem(&op->taq,&dch) >= 0) ;
	            } else {
	                rs = charq_ins(&op->taq,ch) ;
	                op->f.rw = TRUE ;
	            }
	            break ;
	        } /* end switch */
	    } /* end if (signal-generation or not) */
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? f_dle : rs ;
}
/* end subroutine (tty_risr) */


/* echo */
static int tty_echo(UTERM *op,cchar *buf,int buflen)
{
	int		rs = SR_OK ;

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

	if (buflen > 0) {
	    rs = u_write(op->fd,buf,buflen) ;
	}

	return rs ;
}
/* end subroutine (tty_echo) */


static int sinotprint(cchar *sp,int sl)
{
	int		ch ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (i < sl) && sp[0] ; i += 1) {
	    ch = MKCHAR(sp[0]) ;
	    f = isprintlatin(ch) ;
	    f = f || (ch == CH_SI) || (ch == CH_SO) ;
	    if (! f) break ;
	} /* end for */

	return (f) ? -1 : i ;
}
/* end subroutine (sinotprint) */


