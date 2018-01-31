/* logsys */

/* send log messages to the system logger device */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOGIDTAB	0		/* insert a log-id tab */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This object module was originally written to create a logging mechanism
        for PCS application programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are the subroutines in this module:

		logsys_open
		logsys_write
		logsys_printf
		logsys_vprintf
		logsys_setid
		logsys_check
		logsys_close


*******************************************************************************/


#define	LOGSYS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/log.h>		/* for LOG_MAXPS */
#include	<sys/strlog.h>		/* interface definitions */
#include	<sys/syslog.h>		/* for all other 'LOG_xxx' */
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<strdcpy.h>
#include	<localmisc.h>

#include	"logsys.h"


/* local defines */

#define	LOGSYS_LOGLEN	LOG_MAXPS
#define	LOGSYS_EXTRA	100
#define	LOGSYS_NMSGS	10

#ifndef	LOGDEV
#define	LOGDEV		"/dev/conslog"
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#undef	BUFLEN
#define	BUFLEN		100

#undef	OUTBUFLEN
#define	OUTBUFLEN	(LOGSYS_LINELEN + 2 + LOGSYS_EXTRA)

#undef	LOGBUFLEN
#define	LOGBUFLEN	LOGSYS_LOGLEN

#define	TO_OPEN		(60 * 60)
#define	TO_WRITE	30
#define	TO_FLUSH	5
#define	TO_LOCK		4

#ifndef	F_LOCK
#define	F_ULOCK		0
#define	F_LOCK		1
#define	F_TLOCK		2
#define	F_TEST		3
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	getnodename(char *,int) ;
extern int	vbufprintf(char *,int,cchar *,va_list) ;
extern int	charcols(int,int,int) ;
extern int	isprintlatin(int) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

struct colstate {
	int	ncols ;
	int	ncol ;
} ;


/* forward references */

int		logsys_write(LOGSYS *,int,cchar *,int) ;
int		logsys_vprintf(LOGSYS *,int,cchar *,va_list) ;

static int	logsys_mklogid(LOGSYS *) ;
static int	logsys_fixlogid(LOGSYS *,int) ;
static int	logsys_fileopen(LOGSYS *) ;
static int	logsys_fileclose(LOGSYS *) ;
static int	logsys_iflush(LOGSYS *) ;
static int	logsys_logdevice(LOGSYS *,int,cchar *,int) ;

static int	colstate_load(struct colstate *,int,int) ;
static int	colstate_linecols(struct colstate *,cchar *,int) ;

static int	loadlogid(char *,int,cchar *) ;
static int	mkclean(char *,int,cchar *,int) ;
static int	hasourbad(cchar *,int) ;
static int	isourbad(int) ;
static int	isLogFac(int) ;


/* local variables */

static const int	logfacs[] = {
	LOG_KERN,
	LOG_USER,
	LOG_MAIL,
	LOG_DAEMON,
	LOG_AUTH,
	LOG_SYSLOG,
	LOG_LPR,
	LOG_NEWS,
	LOG_UUCP,
	LOG_CRON,
	LOG_LOCAL0,
	LOG_LOCAL1,
	LOG_LOCAL2,
	LOG_LOCAL3,
	LOG_LOCAL4,
	LOG_LOCAL5,
	LOG_LOCAL6,
	LOG_LOCAL7,
	-1
} ;


/* exported subroutines */


int logsys_open(LOGSYS *op,int logfac,cchar *logtag,cchar *logid,int opts)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("logsys_open: ent\n") ;
	debugprintf("logsys_open: fac=%u\n",logfac) ;
	debugprintf("logsys_open: tag=%s\n",logtag) ;
	if (logid != NULL)
	    debugprintf("logsys_open: ent id=%s\n",logid) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (logtag == NULL) return SR_FAULT ;

	if (logtag[0] == '\0') return SR_INVALID ;
	if (! isLogFac(logfac)) return SR_INVALID ;

	memset(op,0,sizeof(LOGSYS)) ;
	op->n = LOGSYS_NMSGS ;
	op->logfac = logfac ;
	op->opts = opts ;
	op->lfd = -1 ;

	if ((rs = uc_mallocstrw(logtag,-1,&cp)) >= 0) {
	    op->logtag = cp ;

/* the log ID */

	    if ((logid == NULL) || (logid[0] == '\0')) {
	        cl = logsys_mklogid(op) ;
	    } else {
	        cl = loadlogid(op->logid,LOGSYS_LOGIDLEN,logid) ;
	    }

	    logsys_fixlogid(op,cl) ;
	    op->magic = LOGSYS_MAGIC ;

	} else {
	    op->lfd = rs ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (logsys_open) */


/* close out the log file and release any resources */
int logsys_close(LOGSYS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOGSYS_MAGIC) return SR_NOTOPEN ;

	rs1 = logsys_fileclose(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->logtag != NULL) {
	    rs1 = uc_free(op->logtag) ;
	    if (rs >= 0) rs = rs1 ;
	    op->logtag = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (logsys_close) */


/* make a log entry */
int logsys_printf(LOGSYS *op,int logpri,cchar *fmt,...)
{
	int		rs ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = logsys_vprintf(op,logpri,fmt,ap) ;
	    va_end(ap) ;
	} /* end block */

	return rs ;
}
/* end subroutine (logsys_printf) */


/* make a log entry */
int logsys_vprintf(LOGSYS *op,int logpri,cchar *fmt,va_list ap)
{
	int		rs = SR_OK ;
	int		sl ;
	int		ol ;
	int		len = 0 ;
	cchar		*tp, *sp ;
	char		outbuf[OUTBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("logsys_vprintf: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOGSYS_MAGIC) return SR_BADF ;

/* format the user's portion */

	ol = vbufprintf(outbuf,OUTBUFLEN,fmt,ap) ;
	if (ol < 0) ol = 0 ;

#ifdef	COMMENT
	if (ol > LOGSYS_USERLEN)
	    ol = LOGSYS_USERLEN ;
#endif

	sp = outbuf ;
	sl = ol ;
	while ((tp = strnchr(sp,sl,'\n')) != NULL) {

	    rs = logsys_write(op,logpri,sp,(tp - sp)) ;
	    len += rs ;
	    if (rs < 0)
		break ;

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = logsys_write(op,logpri,sp,sl) ;
	    len += rs ;
	}

#if	CF_DEBUGS
	debugprintf("logsys_vprintf: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (logsys_vprintf) */


/* set (or reset) the log ID */
int logsys_setid(LOGSYS *op,cchar *logid)
{
	int		rs = SR_OK ;
	int		cl ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOGSYS_MAGIC) return SR_NOTOPEN ;

	if (logid == NULL)
	    logid = "*null*" ;

	cl = loadlogid(op->logid,LOGSYS_LOGIDLEN,logid) ;

	rs = logsys_fixlogid(op,cl) ;

#if	CF_DEBUGS
	debugprintf("logsys_setid: ret\n") ;
#endif

	return rs ;
}
/* end subroutine (logsys_setid) */


int logsys_check(LOGSYS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOGSYS_MAGIC) return SR_NOTOPEN ;

	if (op->lfd < 0) return SR_OK ;

#if	CF_DEBUGS
	{
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_log(dt,tbuf) ;
	    debugprintf("logsys_check: ent time=%s\n",tbuf) ;
	}
#endif

	f = f || ((dt - op->ti_write) >= TO_WRITE) ;
	f = f || ((dt - op->ti_open) >= TO_OPEN) ;
	if (f) {
	    rs = logsys_fileclose(op) ;
	} else {
	    if ((dt - op->ti_write) >= TO_FLUSH) {
	        if (op->c > 0) {
		    rs = logsys_iflush(op) ;
		}
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (logsys_check) */


int logsys_flush(LOGSYS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LOGSYS_MAGIC) return SR_NOTOPEN ;

	if (op->c > 0) rs = logsys_iflush(op) ;

	return rs ;
}
/* end subroutine (logsys_flush) */


int logsys_write(LOGSYS *op,int logpri,cchar *wbuf,int wlen)
{
	struct colstate	cs ;
	int		rs = SR_OK ;
	int		outlen ;
	int		blen ;

	if (op == NULL) return SR_FAULT ;
	if (wbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("logsys_write: ent\n") ;
#endif

	if (wlen < 0)
	    wlen = strlen(wbuf) ;

	if (wbuf[wlen-1] == '\n')
	    wlen -= 1 ;

	colstate_load(&cs,LOGSYS_LINELEN,(LOGSYS_LOGIDLEN + 1)) ;

	blen = colstate_linecols(&cs,wbuf,wlen) ;

	outlen = (op->logidlen + blen + 1) ;

/* do we need a flush? */

	if (op->c > 0) rs = logsys_iflush(op) ;

	if (rs >= 0) {
	    int		bl = wlen ;
	    cchar	*bp = wbuf ;
	    char	tmpbuf[LOGSYS_USERLEN + 1] ;

	if (hasourbad(wbuf,blen)) {
	    bp = tmpbuf ;
	    bl = mkclean(tmpbuf,LOGSYS_USERLEN,wbuf,wlen) ;
	}

/* OK, put it all together and write it to the log device */

#if	CF_DEBUGS
	debugprintf("logsys_write: logpri=%u\n",logpri) ;
	debugprintf("logsys_write: b=>%t<\n",bp,bl) ;
#endif

	    if ((rs = logsys_logdevice(op,logpri,bp,bl)) >= 0) {
	        op->c += 1 ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("logsys_write: ret rs=%d outlen=%u\n",rs,outlen) ;
#endif

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (logsys_write) */


/* private subroutines */


static int logsys_mklogid(LOGSYS *op)
{
	const int	nlen = NODENAMELEN ;
	int		rs ;
	int		cl = 0 ;
	char		nbuf[NODENAMELEN + 1] ;
	char		*unp = "pid" ;

	if ((rs = getnodename(nbuf,nlen)) >= 0) {
	    const pid_t	pid = getpid() ;

	    unp = nbuf ;
	cl = snsd(op->logid,LOGSYS_LOGIDLEN,unp,(uint) pid) ;

	if (cl < 0) {
	    int		rl, ml ;
	    char	digbuf[LOGSYS_LOGIDLEN + 1] ;
	    char	*cp ;

	    ctdecui(digbuf,LOGSYS_LOGIDLEN,(uint) pid) ;

	    cp = op->logid ;
	    rl = LOGSYS_LOGIDLEN ;
	    ml = 9 ;
	    cp = strwcpy(cp,unp,ml) ;
	    rl -= ml ;

	    ml = 6 ;
	    cp = strwcpy(cp,digbuf,ml) ;
	    rl -= ml ;

	    cl = (cp - op->logid) ;

	} /* end if (first try failed) */

	} /* end if (getnodename) */

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (logsys_mklogid) */


static int logsys_fixlogid(LOGSYS *op,int cl)
{

#if	CF_LOGIDTAB
	if (cl < NTABCOLS) {
	    strwset((op->logid + cl),' ',(NTABCOLS - cl)) ;
	    cl = NTABCOLS ;
	}
	op->logid[cl++] = '\t' ;
#else /* CF_LOGIDTAB */
#endif /* CF_LOGIDTAB */

	op->logid[cl] = '\0' ;
	op->logidlen = cl ;
	return cl ;
}
/* end subroutine (logsys_fixlogid) */


static int logsys_fileopen(LOGSYS *op)
{
	int		rs = SR_OK ;

	if (op->lfd < 0) {
	    const mode_t	om = 0666 ;
	    const int		of = O_WRONLY ;
	    cchar		*logdev = LOGDEV ;
	    if ((rs = u_open(logdev,of,om)) >= 0) {
		op->lfd = rs ;
		rs = uc_closeonexec(op->lfd,TRUE) ;
		op->ti_open = time(NULL) ;
		if (rs < 0) {
		    u_close(op->lfd) ;
		    op->lfd = -1 ;
		}
	    } /* end if (open) */
	} /* end if (need an open) */

	return rs ;
}
/* end subroutine (logsys_fileopen) */


static int logsys_fileclose(LOGSYS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (op->c > 0) {
	    len = logsys_iflush(op) ;
	    if (rs >= 0) rs = len ;
	}

	if (op->lfd >= 0) {
	    rs1 = uc_close(op->lfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lfd = -1 ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (logsys_fileclose) */


/* this is a nuller now w/ the system log device */
static int logsys_iflush(LOGSYS *op)
{
	if (op->c >= op->n) op->c = 0 ;
	return SR_OK ;
}
/* end subroutine (logsys_iflush) */


static int logsys_logdevice(LOGSYS *op,int logpri,cchar *wp,int wl)
{
	struct strbuf	cmsg, dmsg ;
	struct log_ctl	lc ;
	const int	llen = LOGBUFLEN ;
	int		rs = SR_OK ;
	int		ll ;
	char		logbuf[LOGBUFLEN+1], *lp = logbuf ;

#if	CF_DEBUGS
	debugprintf("logsys_logdevice: ent logpri=%u\n",logpri) ;
	debugprintf("logsys_logdevice: logtag=%s\n",op->logtag) ;
#endif

	if (op->lfd < 0) rs = logsys_fileopen(op) ;

	if (rs >= 0) {

	logpri &= LOG_PRIMASK ;		/* truncate any garbage */

/* formulate the string to write */

	    {
	        cchar	*lt = op->logtag ;
	        cchar	*li = op->logid ;
	        ll = strdcpy5w(lp,llen,lt,"-",li,": ",wp,wl) - lp ;
	    }

/* write it to the LOG device */

	memset(&lc,0,sizeof(struct log_ctl)) ;
	lc.flags = SL_CONSOLE ;
	lc.level = 0 ;
	lc.pri = (op->logfac | logpri) ;

	/* set up the strbufs */
	cmsg.maxlen = sizeof(struct log_ctl) ;
	cmsg.len = sizeof(struct log_ctl) ;
	cmsg.buf = (caddr_t) &lc ;

	dmsg.maxlen = (ll+1) ;
	dmsg.len = ll ;
	dmsg.buf = logbuf ;

	/* output the message to the local logger */
	rs = u_putmsg(op->lfd,&cmsg,&dmsg,0) ;

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("logsys_logdevice: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (logsys_logdevice) */


static int colstate_load(struct colstate *csp,int ncols,int ncol)
{

	csp->ncols = ncols ;
	csp->ncol = ncol ;
	return SR_OK ;
}
/* end subroutine (colstate_load) */


/* return the number of characters that will fill the current column limit */
static int colstate_linecols(struct colstate *csp,cchar *lbuf,int llen)
{
	int		i ;
	int		cols ;
	int		rcols ;

	rcols = (csp->ncols - csp->ncol) ;
	for (i = 0 ; (rcols > 0) && (i < llen) ; i += 1) {

	    cols = charcols(NTABCOLS,csp->ncol,lbuf[i]) ;

	    if (cols > rcols)
		break ;

	    csp->ncol += cols ;
	    rcols -= cols ;

	} /* end for */

	return i ;
}
/* end subroutine (colstate_linecols) */


static int loadlogid(char *outbuf,int outlen,cchar *logstr)
{
	int		i ;
	int		len = 0 ;

	for (i = 0 ; (i < outlen) && logstr[i] ; i += 1) {
	    const int	ch = MKCHAR(logstr[i]) ;
	    if (isprintlatin(ch)) {
		outbuf[len++] = ch ;
	    }
	} /* end for */

	return len ;
}
/* end subroutine (loadlogid) */


static int mkclean(char outbuf[],int outlen,cchar *sbuf,int slen)
{
	int		i ;

	for (i = 0 ; (i < outlen) && (i < slen) ; i += 1) {
	    outbuf[i] = sbuf[i] ;
	    if (isourbad(sbuf[i] & 0xff)) outbuf[i] = '­' ;
	} /* end for */

	return i ;
}
/* end subroutine (mkclean) */


static int hasourbad(cchar *sp,int sl)
{
	int		ch ;
	int		f = FALSE ;

	while (sl && (sp[0] != '\0')) {

	    ch = (sp[0] & 0xff) ;
	    f = isourbad(ch) ;
	    if (f) break ;

	    sp += 1 ;
	    sl -= 1 ;

	} /* end if */

	return f ;
}
/* end subroutine (hasourbad) */


static int isourbad(int ch)
{
	int		f = TRUE ;

	switch (ch) {
	case CH_SO:
	case CH_SI:
	case CH_SS2:
	case CH_SS3:
	case '\t':
	    f = FALSE ;
	    break ;
	default:
	    f = (! isprintlatin(ch)) ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isourbad) */


int isLogFac(int fac)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; logfacs[i] >= 0 ; i += 1) {
	    f = (fac == logfacs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (isLogFac) */


