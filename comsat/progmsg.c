/* progcsmsg */

/* process a COMSAT message */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_OUTINFO	1		/* object OUTINFO */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we process a COMSAT message.

	Notes:

	= What is the format of the note printed out to the user terminals?

¶ <username> <time> « <from> - <subj>

	where:

	field		characters	sum
	--------------------------------------------
	¶		1		1
	_		1		2
	<username)	8		10
	_		1		11
	<time>		5		16
	_		1		17
	«		1		18
	_		1		19
	<from>		35		54
	_		1		55
	-		1		56
	_		1		57
	<subj>		80-everything_previous	80-57=23


	= What kind of concurrency do we get here?

	Not as much as I would have liked!  We do not get nearly as much
	concurrency as I would have liked.  Why not?  Because the object
	TERMNOTE is not thread-safe.  It is reentrant (as all of our code is
	and always has been) but that does not make it thread-safe
	subroutine-by-subroutine.  It is not thread-safe because it uses an
	underlying object named TMPX which is not thread-safe itself.  Both of
	these objects could be made -- rather easily -- thread-safe but it just
	has not been done.  If someday they (TMPX and TERMNOTE) become
	thread-safe, then this whole enchilada would become almost completely
	concurrent.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>		/* for 'struct timeb' */
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>		/* for 'strftime(3c)' */

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<mailmsghdrs.h>
#include	<termnote.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"progcs.h"
#include	"comsatmsg.h"


/* local defines */

#ifndef	SUBJBUFLEN
#define	SUBJBUFLEN	(2*COLUMNS)
#endif

#ifndef	NOTEBUFLEN
#define	NOTEBUFLEN	COLUMNS
#endif

#define	DATEBUFLEN	5		/* "HH:MM" */
#define	MAXOVERLEN	22
#define	MAXFROMLEN	35

#define	OUTINFO		struct outinfo


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	snwcpywidehdr(char *,int,const wchar_t *,int) ;
extern int	snwcpycompact(char *,int,cchar *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	wsichr(const wchar_t *,int,int) ;
extern int	wchar_iswhite(wchar_t) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	compactstr(char *,int) ;
extern int	mkaddrname(char *,int,cchar *,int) ;
extern int	mkdisphdr(char *,int,cchar *,int) ;
extern int	mkbestfrom(char *,int,cchar *,int) ;
extern int	mkcleanline(char *,int,int) ;
extern int	mailmsg_loadfd(MAILMSG *,int,offset_t) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

extern int	progerr_printf(PROGINFO *,cchar *,...) ;

extern int	progloglock_printf(PROGINFO *,cchar *,...) ;

extern int	prognote_write(PROGINFO *,cchar **,int,int,cchar *,int) ;

extern int	strlinelen(const char *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

#if	CF_OUTINFO
struct outinfo {
	PROGINFO	*pip ;
	MAILMSG		*mmp ;
	cchar		*un ;		/* supplied by caller */
	wchar_t		*fbuf ;		/* wide-char */
	wchar_t		*sbuf ;		/* wide-char */
	char		*ofbuf ;
	char		*osbuf ;
	int		dl, ul ;
	int		flen, fl ;
	int		slen, sl ;
	int		oflen ;
	int		oslen ;
	int		ll ;
	int		tl ;		/* "total" length */
	int		dlen ;
	char		dbuf[DATEBUFLEN+1] ;
} ;
#endif /* CF_OUTINFO */


/* forward references */

static int	progcsmsger(PROGINFO *,int,offset_t,cchar *) ;
static int	proclogmsg(PROGINFO *,COMSATMSG_MO *) ;
static int	procmsginfo(PROGINFO *,MAILMSG *,const char *) ;
static int	procmsgbad(PROGINFO *,cchar *,offset_t,int) ;
static int	getdateinfo(PROGINFO *,char *,int,const char *,int,int) ;

#if	CF_OUTINFO
static int	outinfo_start(OUTINFO *,PROGINFO *,MAILMSG *,cchar *) ;
static int	outinfo_finish(OUTINFO *) ;
static int	outinfo_hdrs(OUTINFO *) ;
static int	outinfo_getfrom(OUTINFO *,cchar **) ;
static int	outinfo_mkfrom(OUTINFO *) ;
static int	outinfo_cvtfrom(OUTINFO *,cchar *,int) ;
static int	outinfo_getsubj(OUTINFO *,cchar **) ;
static int	outinfo_mksubj(OUTINFO *) ;
static int	outinfo_cvtsubj(OUTINFO *,cchar *,int) ;
static int	outinfo_mkdate(OUTINFO *) ;
static int	outinfo_adjust(OUTINFO *) ;
static int	outinfo_cols(OUTINFO *) ;
static int	outinfo_termbegin(OUTINFO *) ;
static int	outinfo_termend(OUTINFO *) ;
static int	outinfo_termbeginfrom(OUTINFO *) ;
static int	outinfo_termbeginsubj(OUTINFO *) ;
static int	outinfo_print(OUTINFO *) ;
#endif /* CF_OUTINFO */

static int	wsfnormfrom(const wchar_t *,int) ;

static int	isNoMsg(int) ;
static int	isBadMsg(int) ;
static int	isBadTime(int) ;


/* global variables */


/* local variables */

static const int	rsnomsg[] = {
	SR_NOMSG,
	SR_NOENT,
	0
} ;

static const int	rsbadmsg[] = {
	SR_BADMSG,
	SR_INVALID,
	SR_DOM,
	SR_RANGE,
	SR_OVERFLOW,
	0
} ;

static const int	rsbadtime[] = {
	SR_BADMSG,
	SR_INVALID,
	SR_DOM,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int progcsmsg(PROGINFO *pip,cchar *mbuf,int mlen)
{
	COMSATMSG_MO	m0 ;
	int		rs ;
	int		ml = mlen ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	const char	*cn ;
	char		*mp = (char *) mbuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progcsmsg: ent\n") ;
	    debugprintf("progcsmsg: msg=>%t<\n",mbuf,strlinelen(mbuf,mlen,40)) ;
	}
#endif

	if (mbuf == NULL) return SR_FAULT ;
	if (mbuf[0] == '\0') return SR_INVALID ;

#ifdef	COMMENT
	if (pip->debuglevel > 0) {
	    progerr_printf(pip,"%s: msg=>%t<\n",pn,
	        mbuf,strlinelen(mbuf,mlen,40)) ;
	}
#endif /* COMMENT */

	if ((rs = comsatmsg_mailoff(&m0,1,mp,ml)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("progcsmsg: comsatmsg_mailoff() rs=%d\n",rs) ;
	        debugprintf("progcsmsg: m0.username=%s\n",m0.username) ;
	        debugprintf("progcsmsg: m0.fname=%s\n",m0.fname) ;
	        debugprintf("progcsmsg: m0.offset=%lu\n",m0.offset) ;
	    }
#endif /* CF_DEBUG */

	    if ((rs = proclogmsg(pip,&m0)) >= 0) {
	        cchar	*maildname = pip->maildname ;
	        char	mailfname[MAXPATHLEN+1] ;

/* continue to process this in the usual way */

	        cn = m0.username ;
	        if (m0.fname[0] != '\0') cn = m0.fname ;

	        if ((rs = mkpath2(mailfname,maildname,cn)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progcsmsg: mailfname=%s\n",mailfname) ;
#endif

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                struct ustat	sb ;
	                int		rs1 = u_stat(mailfname,&sb) ;
	                debugprintf("progcsmsg: u_stat() rs1=%d msize=%llu\n",
	                    rs1,sb.st_size) ;
	            }
#endif /* CF_DEBUG */

	            if ((rs = uc_open(mailfname,O_RDONLY,0666)) >= 0) {
	                offset_t	fo = (offset_t) m0.offset ;
	                const int	mfd = rs ;
	                const char	*un = m0.username ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    struct ustat	sb ;
	                    cchar		*fmt ;
	                    int		rs1 = u_fstat(mfd,&sb) ;
	                    debugprintf("progcsmsg: mfd=%u\n",mfd) ;
	                    fmt = "progcsmsg: u_fstat() rs1=%d msize=%llu\n" ;
	                    debugprintf(fmt,rs1,sb.st_size) ;
	                }
#endif /* CF_DEBUG */

	                if (fo > 0) rs = u_seek(mfd,fo,SEEK_SET) ;

	                if (rs >= 0) {
			    rs = progcsmsger(pip,mfd,fo,un) ;
			    wlen = rs ;
	                } /* end if (seek) */

	                u_close(mfd) ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
	            } /* end if (opened mail file) */

	        } /* end if (mkpath) */

	    } /* end if (proclogmsg) */

	} else if (isBadMsg(rs)) {
	    if (pip->debuglevel > 0) {
	        progerr_printf(pip,"%s: bad-msg (%d)\n",pn,rs) ;
	    }
	    if (pip->open.logprog) {
	        progloglock_printf(pip,"bad-msg (%d)",rs) ;
	    }
	    rs = SR_OK ;
	} /* end if (comsatmsg_mailoff) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcsmsg: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcsmsg) */


/* local subroutines */


static int progcsmsger(PROGINFO *pip,int mfd,offset_t fo,cchar *un)
{
	MAILMSG		mm ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcsmsger: ent\n") ;
#endif
	if ((rs = mailmsg_start(&mm)) >= 0) {
	    const int	rsi = SR_INVALID ;
	    if ((rs = mailmsg_loadfd(&mm,mfd,fo)) > 0) {
		rs = procmsginfo(pip,&mm,un) ;
		wlen = rs ;
	    } else if ((rs == SR_OK) || (rs == rsi)) {
		rs = procmsgbad(pip,un,fo,rs) ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	    rs1 = mailmsg_finish(&mm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailmsg) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcsmsger: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcsmsger) */


static int proclogmsg(PROGINFO *pip,COMSATMSG_MO *m0p)
{
	int		rs = SR_OK ;
	const char	*pn = pip->progname ;

	progerr_printf(pip,"%s: comsat-msg¬\n",pn) ;
	progerr_printf(pip,"%s:   u=%s\n",pn,m0p->username) ;
	progerr_printf(pip,"%s:   f=%s\n",pn,m0p->fname) ;
	progerr_printf(pip,"%s:   o=%lu\n",pn,m0p->offset) ;

	progloglock_printf(pip,"comsat-msg¬") ;
	progloglock_printf(pip,"  u=%s",m0p->username) ;
	progloglock_printf(pip,"  f=%s",m0p->fname) ;
	progloglock_printf(pip,"  o=%lu",m0p->offset) ;

	return rs ;
}
/* end subroutine (proclogmsg) */


#if	CF_OUTINFO
static int procmsginfo(PROGINFO *pip,MAILMSG *mmp,cchar *un)
{
	OUTINFO		oi ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = outinfo_start(&oi,pip,mmp,un)) >= 0) {
	    if ((rs = outinfo_hdrs(&oi)) >= 0) {
		if ((rs = outinfo_adjust(&oi)) >= 0) {
		    if ((rs = outinfo_termbegin(&oi)) >= 0) {
			{
		            rs = outinfo_print(&oi) ;
			    wlen = rs ;
			}
			rs1 = outinfo_termend(&oi) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (outinfo_termtrans) */
		} /* end if (outinfo_adjust) */
	    } /* end if (outinfo_hdrs) */
	    rs1 = outinfo_finish(&oi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (outinfo) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsginfo) */
#else /* CF_OUTINFO */
static int procmsginfo(PROGINFO *pip,MAILMSG *mmp,cchar *un)
{
	const int	dt = pip->daytime ;
	const int	fromlen = MAILADDRLEN ;
	const int	datelen = TIMEBUFLEN ;
	const int	subjlen = TIMEBUFLEN ;
	const int	notelen = NOTEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	int		fl = 0 ;
	int		sl = 0 ;
	int		rc = 0 ;
	const char	*hp ;
	char		tbuf[TIMEBUFLEN+1] ;
	char		datebuf[TIMEBUFLEN+1] = { 0 } ;
	char		frombuf[MAILADDRLEN+1] = { 0 } ;
	char		subjbuf[SUBJBUFLEN+1] = { 0 } ;
	char		notebuf[NOTEBUFLEN+1] = { 0 } ;

	if (rs >= 0) {
	    int	f_edate = FALSE ;
	    rs1 = mailmsg_hdrival(mmp,HN_DATE,0,&hp) ;
	    hl = rs1 ;
	    if (rs1 == SR_NOENT) {
	        f_edate = TRUE ;
	        rs1 = mailmsg_envdate(mmp,0,&hp) ;
	        hl = rs1 ;
	    }
	    if (rs1 >= 0) {
	        rs = getdateinfo(pip,datebuf,datelen,hp,hl,f_edate) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progcsmsg/procmsginfo: rs=%d date=%s\n",rs,datebuf) ;
#endif

	if (rs >= 0) {
	    rs1 = mailmsg_hdrival(mmp,HN_FROM,0,&hp) ;
	    hl = rs1 ;
	    if (rs1 >= 0) {
	        rs = mkaddrname(frombuf,fromlen,hp,hl) ;
	        fl = rs ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progcsmsg/procmsginfo: rs=%d from=>%t<\n",
	        rs,frombuf,strlinelen(frombuf,-1,40)) ;
#endif

	if (rs >= 0) {
	    rs1 = mailmsg_hdrival(mmp,HN_SUBJECT,0,&hp) ;
	    hl = rs1 ;
	    if (rs1 >= 0) {
	        sl = strdcpy1w(subjbuf,subjlen,hp,hl) - subjbuf ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progcsmsg/procmsginfo: rs=%d subj=>%t<\n",
	        rs,subjbuf,strlinelen(subjbuf,-1,40)) ;
#endif

	if (rs >= 0) {
	    fl = compactstr(frombuf,fl) ;
	    fl = mkcleanline(frombuf,fl,1) ;
	    sl = compactstr(subjbuf,sl) ;
	    sl = mkcleanline(subjbuf,sl,1) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progcsmsg/procmsginfo: compaction rs=%d\n",rs) ;
	    debugprintf("progcsmsg/procmsginfo: rs=%d subj=>%t<\n",
	        rs,subjbuf,strlinelen(subjbuf,-1,40)) ;
	    debugprintf("progcsmsg/procmsginfo: rs=%d from=>%t<\n",
	        rs,frombuf,strlinelen(frombuf,-1,40)) ;
	}
#endif

	if (rs >= 0) {
	    int	nl = (COLUMNS - MAXOVERLEN) ;

	    if (fl > MAXOVERLEN) {
	        const char	*tp = strnchr(frombuf,fl,',') ;
	        if (tp != NULL) {
	            fl = (tp-frombuf) ;
	            while (fl && CHAR_ISWHITE(frombuf[fl-1])) fl -= 1 ;
	        }
	        if (fl > MAXOVERLEN) fl = MAXOVERLEN ;
	        frombuf[fl] = '\0' ;
	    }
	    nl -= fl ;
	    if (sl > nl) {
	        sl = nl ;
	        subjbuf[sl] = '\0' ;
	    }

	    timestr_logz(dt,tbuf) ;
	    progerr_printf(pip,"%s: %s u=%s time=%s\n",
	        pip->progname,tbuf,un,datebuf) ;

	    timestr_logz(dt,timebuf) ;
	    progloglock_printf(pip,"%s u=%s time=%s",timebuf,un,datebuf) ;
	    progloglock_printf(pip,"  from=»%s«",frombuf) ;
	    progloglock_printf(pip,"  subj=»%s«",subjbuf) ;

	    {
	        const char	*fmt = "¶ %s %s « %t - %t" ;
	        const char	*db = datebuf ;
	        const char	*fb = frombuf ;
	        const char	*sb = subjbuf ;
	        rs = bufprintf(notebuf,notelen,fmt,db,un,fb,fl,sb,sl) ;
	        nl = rs ;
	    }

	    if (rs >= 0) {
	        const int	max = 3 ;
	        const int	o = (TERMNOTE_OBIFF | TERMNOTE_OBELL) ;
	        int		i = 0 ;
	        const char	*recips[2] ;
	        recips[i++] = un ;
	        recips[i] = NULL ;
	        rs = prognote_write(pip,recips,max,o,notebuf,nl) ;
	        rc = rs ;
	        if (rs >= 0) {
	            rs = progloglock_printf(pip,"  termnote (%d)",rs) ;
	        }
	    } /* end if (ok) */

	} /* end if (ok) */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (procmsginfo) */
#endif /* CF_OUTINFO */


static int procmsgbad(PROGINFO *pip,cchar *un,offset_t fo,int rsl)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if (rsl == SR_OK) {
	    fmt = "missing msg u=%s offset=%llu" ;
	} else {
	    fmt = "invalid msg u=%s offset=%llu (%d)" ;
	}
	progloglock_printf(pip,fmt,un,fo,rsl) ;
	if (pip->debuglevel > 0) {
	    if (rsl == SR_OK) {
	        fmt = "%s: missing msg u=%s offset=%llu" ;
	    } else {
	        fmt = "%s: invalid msg u=%s offset=%llu (%d)" ;
	    }
	    progerr_printf(pip,fmt,pn,un,fo,rsl) ;
	}
	return rs ;
}
/* end subroutine (procmsgbad) */


static int getdateinfo(PROGINFO *pip,char *abuf,int alen,cchar *ap,int al,
		int f_edate)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (abuf == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;

	abuf[0] = '\0' ;
	if (al < 0) al = strlen(ap) ;

	if (al >= 0) {
	    if (f_edate) {
	        rs = dater_setstd(&pip->d,ap,al) ;
	    } else {
	        rs = dater_setmsg(&pip->d,ap,al) ;
	    }
	    if (rs >= 0) {
	        time_t	mt ;
	        if ((rs = dater_gettime(&pip->d,&mt)) >= 0) {
	            struct tm	ts ;
	            if ((rs = uc_localtime(&mt,&ts)) >= 0) {
	                len = strftime(abuf,(alen+1),"%R",&ts) ;
	                if (len == 0) abuf[0] = '\0' ;
#if	CF_DEBUGS
	debugprintf("getdateinfo: dl=%d db=>%t<\n",len,abuf,len) ;
#endif
	            }
	        }
	    } else if (isBadTime(rs)) {
		rs = SR_OK ;
		len = sncpy1(abuf,alen,"99:99") ;
	    } /* end if (dater) */
	} else {
	    abuf[0] = '\0' ;
	} /* end if (non-zero) */

#if	CF_DEBUGS
	debugprintf("getdateinfo: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getdateinfo) */


#if	CF_OUTINFO

static int outinfo_start(OUTINFO *op,PROGINFO *pip,MAILMSG *mmp,cchar *un)
{
	int		rs = SR_OK ;

	memset(op,0,sizeof(OUTINFO)) ;
	op->pip = pip ;
	op->mmp = mmp ;
	op->un = un ;
	op->dlen = DATEBUFLEN ;
	op->ul = strlen(un) ;
	op->ll = COLUMNS ;

	return rs ;
}
/* end subroutine (outinfo_start) */


static int outinfo_finish(OUTINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->sbuf != NULL) {
	    rs1 = uc_free(op->sbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->sbuf = NULL ;
	}

	if (op->fbuf != NULL) {
	    rs1 = uc_free(op->fbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fbuf = NULL ;
	}

	op->pip = NULL ;
	return rs ;
}
/* end subroutine (outinfo_finish) */


static int outinfo_hdrs(OUTINFO *oip)
{
	int		rs ;
	if ((rs = outinfo_mkdate(oip)) >= 0) {
	    if ((rs = outinfo_mkfrom(oip)) >= 0) {
	        rs = outinfo_mksubj(oip) ;
	    }
	}
	return rs ;
}
/* end subroutine (outinfo_hdrs) */


static int outinfo_mkdate(OUTINFO *oip)
{
	MAILMSG		*mmp = oip->mmp ;
	int		rs ;
	int		hl = 0 ;
	int		f_edate = FALSE ;
	cchar		*hp ;
	if ((rs = mailmsg_hdrival(mmp,HN_DATE,0,&hp)) > 0) {
	    hl = rs ;
	} else if ((rs == 0) || isNoMsg(rs)) {
	    if ((rs = mailmsg_envdate(mmp,0,&hp)) > 0) {
	        f_edate = TRUE ;
	        hl = rs ;
	    } else if (isNoMsg(rs)) {
		rs = SR_OK ;
	    }
	}
	if ((rs >= 0) && (hl > 0)) {
	    PROGINFO	*pip = oip->pip ;
	    const int	dlen = oip->dlen ;
	    char	*dbuf = oip->dbuf ;
	    rs = getdateinfo(pip,dbuf,dlen,hp,hl,f_edate) ;
	}
	return rs ;
}
/* end subroutine (outinfo_mkdate) */


static int outinfo_getfrom(OUTINFO *oip,cchar **rpp)
{
	MAILMSG		*mmp = oip->mmp ;
	int		rs ;
	int		hl = 0 ;
	cchar		*hn = HN_FROM ;
	cchar		*hp ;
	if ((rs = mailmsg_hdrval(mmp,hn,&hp)) > 0) {
	    hl = rs ;
	} else if ((rs == 0) || isNoMsg(rs)) {
	    hn = HN_RETURNPATH ;
	    if ((rs = mailmsg_hdrval(mmp,hn,&hp)) > 0) {
	        hl = rs ;
	    } else if ((rs == 0) || isNoMsg(rs)) {
	        hn = HN_REPLYTO ;
	        if ((rs = mailmsg_hdrval(mmp,hn,&hp)) > 0) {
	            hl = rs ;
	        } else if (isNoMsg(rs)) {
		    rs = SR_OK ;
		}
	    }
	}
	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? hp : NULL ;
	}
	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (outinfo_getfrom) */


static int outinfo_mkfrom(OUTINFO *oip)
{
	int		rs ;
	int		len = 0 ;
	cchar		*hp ;
	if ((rs = outinfo_getfrom(oip,&hp)) >= 0) {
	    const int	hl = rs ;
	    const int	rlen = rs ;
	    char	*rbuf ;
	    if ((rs = uc_malloc((rlen+1),&rbuf)) >= 0) {
		if ((rs = mkaddrname(rbuf,rlen,hp,hl)) > 0) {
		    rs = outinfo_cvtfrom(oip,rbuf,rs) ;
		    len = rs ;
		} /* end if (mkaddrname) */
		uc_free(rbuf) ;
	    } /* end if (m-a-f) */
	} /* end if (have) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (outinfo_mkfrom) */


static int outinfo_cvtfrom(OUTINFO *oip,cchar *sp,int sl)
{
	PROGINFO	*pip = oip->pip ;
	const int	size = ((sl+1) * sizeof(wchar_t)) ;
	const int	ilen = sl ;
	int		rs ;
	int		rcols = 0 ;
	wchar_t		*ibuf ;
#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_cvt: ent sl=%d\n",sl) ;
#endif
	if ((rs = uc_malloc(size,&ibuf)) >= 0) {
	    if ((rs = progcs_trans(pip,ibuf,ilen,sp,sl)) >= 0) {
	        oip->flen = rs ;
		oip->fbuf = ibuf ;
		rcols = rs ;
	    } /* end if (progcs_trans) */
	    if (rs < 0) {
		uc_free(ibuf) ;
	    }
	} /* end if (m-a) */
#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_cvt: ret rs=%d rcols=%d\n",rs,rcols) ;
#endif
	return (rs >= 0) ? rcols : rs ;
}
/* end subroutine (outinfo_cvtfrom) */


static int outinfo_getsubj(OUTINFO *oip,cchar **rpp)
{
	MAILMSG		*mmp = oip->mmp ;
	int		rs ;
	int		hl = 0 ;
	cchar		*hn = HN_SUBJECT ;
	cchar		*hp ;
	if ((rs = mailmsg_hdrval(mmp,hn,&hp)) > 0) {
	    hl = rs ;
	} else if ((rs == 0) || isNoMsg(rs)) {
	    hp = hn ; /* something and static */
	    rs = SR_OK ;
	}
	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? hp : NULL ;
	}
	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (outinfo_getsubj) */


static int outinfo_mksubj(OUTINFO *oip)
{
	int		rs ;
	int		len = 0 ;
	cchar		*hp ;
	if ((rs = outinfo_getsubj(oip,&hp)) >= 0) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfshrink(hp,rs,&cp)) >= 0) {
		    rs = outinfo_cvtsubj(oip,cp,cl) ;
		    len = rs ;
	    }
	} /* end if (have) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (outinfo_mksubj) */


static int outinfo_cvtsubj(OUTINFO *oip,cchar *sp,int sl)
{
	PROGINFO	*pip = oip->pip ;
	const int	size = ((sl+1) * sizeof(wchar_t)) ;
	const int	ilen = sl ;
	int		rs ;
	int		rcols = 0 ;
	wchar_t		*ibuf ;
#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_cvt: ent sl=%d\n",sl) ;
#endif
	if ((rs = uc_malloc(size,&ibuf)) >= 0) {
	    if ((rs = progcs_trans(pip,ibuf,ilen,sp,sl)) >= 0) {
	        oip->slen = rs ;
		oip->sbuf = ibuf ;
		rcols = rs ;
	    } /* end if (progcs_trans) */
	    if (rs < 0) {
		uc_free(ibuf) ;
	    }
	} /* end if (m-a) */
#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_cvt: ret rs=%d rcols=%d\n",rs,rcols) ;
#endif
	return (rs >= 0) ? rcols : rs ;
}
/* end subroutine (outinfo_cvtsubj) */


static int outinfo_adjust(OUTINFO *oip)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	int		tl ;

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: ent\n") ;
#endif

/* setup */

	oip->fl = oip->flen ;
	oip->sl = oip->slen ;
	oip->ul = strlen(oip->un) ;

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: fl=%u\n",oip->fl) ;
	debugprintf("progcsmsg/outinfo_adjust: sl=%u\n",oip->sl) ;
	debugprintf("progcsmsg/outinfo_adjust: ul=%u\n",oip->ul) ;
#endif

/* reductions */

	tl = outinfo_cols(oip) ;

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: mid1 tl=%u\n",tl) ;
#endif

	if (tl > oip->ll) {
	    if ((rl = wsfnormfrom(oip->fbuf,oip->fl)) >= 0) {
	        oip->fl = rl ;
		tl = outinfo_cols(oip) ;
	    }
	}


#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: mid2 tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    rl = (tl - oip->ll) ;
	    if ((oip->fl - rl) >= MAXFROMLEN) {
	        oip->fl -= rl ;
	        tl = outinfo_cols(oip) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: mid3 tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    if (oip->fl > MAXFROMLEN) {
	        oip->fl = MAXFROMLEN ;
	        tl = outinfo_cols(oip) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: mid4 tl=%d\n",tl) ;
#endif

	if (tl > oip->ll) {
	    rl = (tl - oip->ll) ;
	    oip->sl -= rl ;
	    tl = outinfo_cols(oip) ;
	}

	if (rs >= 0) {
	   oip->tl = tl ;
	}

#if	CF_DEBUGS
	debugprintf("progcsmsg/outinfo_adjust: ret rs=%d tl=%d\n",rs,tl) ;
#endif

	return (rs >= 0) ? tl : rs ;
}
/* end if (outinfo_adjust) */


static int outinfo_cols(OUTINFO *oip)
{
	int	cols = (3+5+3+3) ; /* non-field columns in output string */
	cols += (oip->dl+oip->ul+oip->fl+oip->sl) ;
	return cols ;
}
/* end if (outinfo_cols) */


static int outinfo_termbegin(OUTINFO *oip)
{
	int		rs = SR_OK ;
	if (rs >= 0) {
	    rs = outinfo_termbeginfrom(oip) ;
	} /* end if (ok) */
	if (rs >= 0) {
	    rs = outinfo_termbeginsubj(oip) ;
	} /* end if (ok) */
	return rs ;
}
/* end if (outinfo_termbegin) */


static int outinfo_termend(OUTINFO *oip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (oip->osbuf != NULL) {
	    rs1 = uc_free(oip->osbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    oip->osbuf = NULL ;
	    oip->oslen = 0 ;
	}
	if (oip->ofbuf != NULL) {
	    rs1 = uc_free(oip->ofbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    oip->ofbuf = NULL ;
	    oip->oflen = 0 ;
	}
	return rs ;
}
/* end if (outinfo_termend) */


static int outinfo_termbeginfrom(OUTINFO *oip)
{
	const int	fl = oip->flen ;
	const int	oflen = (2*oip->flen) ;
	int		rs ;
	const wchar_t	*fp = oip->fbuf ;
	char		*ofbuf ;
	    if ((rs = uc_malloc((oflen+1),&ofbuf)) >= 0) {
		const int	tlen = oflen ;
		char		*tbuf ;
		if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
		    if ((rs = snwcpywidehdr(tbuf,tlen,fp,fl)) >= 0) {
		        if ((rs = mkdisphdr(ofbuf,oflen,tbuf,rs)) >= 0) {
		            oip->ofbuf = ofbuf ;
		            oip->oflen = rs ;
		        }
		    }
		    uc_free(tbuf) ;
		} /* end if (m-a-f) */
		if (rs < 0) {
		    uc_free(ofbuf) ;
		}
	    } /* end if (m-a) */
	return rs ;
}
/* end subroutine (outinfo_termbeginfrom) */


static int outinfo_termbeginsubj(OUTINFO *oip)
{
	const int	sl = oip->slen ;
	const int	oslen = (2*oip->slen) ;
	int		rs ;
	const wchar_t	*sp = oip->sbuf ;
	char		*osbuf ;
	    if ((rs = uc_malloc((oslen+1),&osbuf)) >= 0) {
		const int	tlen = oslen ;
		char		*tbuf ;
		if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
		    if ((rs = snwcpywidehdr(tbuf,tlen,sp,sl)) >= 0) {
		        if ((rs = snwcpycompact(osbuf,oslen,tbuf,rs)) >= 0) {
		            oip->osbuf = osbuf ;
		            oip->oslen = rs ;
			}
		    }
		    uc_free(tbuf) ;
		} /* end if (m-a-f) */
		if (rs < 0) {
		    uc_free(osbuf) ;
		}
	    } /* end if (m-a) */
	return rs ;
}
/* end subroutine (outinfo_termbeginsubj) */


static int outinfo_print(OUTINFO *oip)
{
	PROGINFO	*pip = oip->pip ;
	const int	olen = oip->tl ;
	int		rs ;
	int		wlen = 0 ;
	char		*obuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progcsmsg/outinfo_print: ent olen=%d\n",olen) ;
#endif

	if ((rs = uc_malloc((olen+1),&obuf)) >= 0) {
	    time_t	dt = pip->daytime ;
	    const int	fl = oip->fl ;
	    const int	sl = oip->sl ;
	    cchar	*pn = pip->progname ;
	    cchar	*un = oip->un ;
	    cchar	*db = oip->dbuf ;
	    cchar	*fb = oip->ofbuf ;
	    cchar	*sb = oip->osbuf ;
	    cchar	*fmt ;
	    char	tbuf[TIMEBUFLEN+1] ;

	    fmt = "%s: %s u=%s time=%s\n" ;
	    timestr_logz(dt,tbuf) ;
	    progerr_printf(pip,fmt,pn,tbuf,un,db) ;
	    fmt = "%s: from=>%t<\n" ;
	    progerr_printf(pip,fmt,pn,fb,fl) ;
	    fmt = "%s: subj=>%t<\n" ;
	    progerr_printf(pip,fmt,pn,sb,sl) ;

	    progloglock_printf(pip,"%s u=%s time=%s",tbuf,un,db) ;
	    progloglock_printf(pip,"  from=»%s«",fb) ;
	    progloglock_printf(pip,"  subj=»%s«",sb) ;

	    fmt = "¶ %s %s « %t - %t" ;
	    if ((rs = bufprintf(obuf,olen,fmt,db,un,fb,fl,sb,sl)) >= 0) {
	        const int	max = 3 ;
	        const int	o = (TERMNOTE_OBIFF | TERMNOTE_OBELL) ;
		const int	ol = rs ;
	        int		i = 0 ;
	        cchar		*recips[2] ;
	        wlen = rs ;
	        recips[i++] = un ;
	        recips[i] = NULL ;
	        rs = prognote_write(pip,recips,max,o,obuf,ol) ;
	        if (rs >= 0) {
	            rs = progloglock_printf(pip,"  termnotes=%u",rs) ;
	        }
	    } /* end if (ok) */

	    uc_free(obuf) ;
	} /* end of (m-a-f) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progcsmsg/outinfo_print: ret rs=%d wlen=%d\n",
		rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outinfo_print) */


static int wsfnormfrom(const wchar_t *fp,int fl)
{
	int		si ;
	if ((si = wsichr(fp,fl,',')) >= 0) {
	    fl = si ;
	    while (fl && wchar_iswhite(fp[fl-1])) fl -= 1 ;
	}
	return fl ;
}
/* end subroutine (wsfnormfrom) */

#endif /* CF_OUTINFO */


static int isNoMsg(int rs)
{
	return isOneOf(rsnomsg,rs) ;
}
/* end subroutine (isNoMsg) */


static int isBadMsg(int rs)
{
	int		f = FALSE ;
	f = f || isOneOf(rsbadmsg,rs) ;
	f = f || isNoMsg(rs) ;
	return f ;
}
/* end subroutine (isBadMsg) */


static int isBadTime(int rs)
{
	return isOneOf(rsbadtime,rs) ;
}
/* end subroutine (isBadTime) */


