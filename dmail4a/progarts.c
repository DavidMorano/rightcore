/* progarts */

/* process the input messages and spool them up */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGBODY	1		/* debug the MAILMSG body */
#define	CF_SPAMSUBJECT	1		/* check SUBJECT for spam */
#define	CF_SPAMFLAG	1		/* check spam-flag for spam */
#define	CF_SPAMSTATUS	1		/* check spam-status for YES */
#define	CF_BOGOSITY	1		/* check bogosity for "Spam" */
#define	CF_XMAILER	1		/* log XMAILER */
#define	CF_RECEIVED	1		/* log RECEIVED */
#define	CF_LOGMLEN	0		/* log message length ('mlen') */
#define	CF_CLENSTART	0		/* search for MSG-START w/ CLEN */
#define	CF_CLENTRAIL	0		/* trailing CLEN processing */
#define	CF_CLINES	1		/* CLINES processing */
#define	CF_NEWSGROUPS	1		/* newsgroups */
#define	CF_MKARTFILE	1		/* use |mkartfile(3dam)| */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module processes one or more mail messages (in appropriate mailbox
        format if more than one) on STDIN. The output is a single file that is
        ready to be added to each individual mailbox in the spool area.

	Things to do:

	Change use of 'sfsubstance()' to 'mkdisphdr()'.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<char.h>
#include	<mailmsgmatenv.h>
#include	<mailmsg.h>
#include	<mailmsg_enver.h>
#include	<mailmsghdrs.h>
#include	<ema.h>
#include	<emainfo.h>
#include	<dater.h>
#include	<mhcom.h>
#include	<comparse.h>
#include	<logzones.h>
#include	<nulstr.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"bfliner.h"
#include	"article.h"
#include	"stackaddr.h"
#include	"received.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	STACKADDRLEN
#define	STACKADDRLEN	(2 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	HDRNAMELEN
#define	HDRNAMELEN	80
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	MAX(LINEBUFLEN,(2 * 1024))
#endif

#ifndef	LOGFMTLEN
#define	LOGFMTLEN	LOGFILE_FMTLEN
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	TABLEN
#define	TABLEN		8
#endif

#ifndef	DATEBUFLEN
#define	DATEBUFLEN	80
#endif

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#define	FMAT(cp)	((cp)[0] == 'F')

#ifndef	HN_XMAILER
#define	HN_XMAILER	"x-mailer"
#endif

#ifndef	HN_RECEIVED
#define	HN_RECEIVED	"received"
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))

#undef	NBLANKS
#define	NBLANKS		20

#define	PROCDATA	struct procdata
#define	PROCDATA_FL	struct procdata_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpyclean(char *,int,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sisub(const char *,int,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mkartfile(char *,mode_t,const char *,const char *,int) ;
extern int	mkbestaddr(char *,int,const char *,int) ;
extern int	mailmsgmathdr(const char *,int,int *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	hasEOH(cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

extern int	progmsgid(PROGINFO *,char *,int,int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

extern int	progprinthdrs(PROGINFO *,bfile *,MAILMSG *,cchar *) ;
extern int	progprinthdraddrs(PROGINFO *,bfile *,MAILMSG *,cchar *) ;
extern int	progprinthdremas(PROGINFO *,bfile *,const char *,EMA *) ;
extern int	progprinthdr(PROGINFO *,bfile *,const char *,
			cchar *,int) ;
extern int	prognamecache_lookup(PROGINFO *,const char *,cchar **) ;
extern int	progmsgfromema(PROGINFO *,EMA **) ;
extern int	progexpiration(PROGINFO *,const char **) ;

extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	mailmsg_loadline(MAILMSG *,const char *,int) ;
extern int	ema_haveaddr(EMA *,const char *,int) ;

extern int	sfsubstance(const char *,int,const char **) ;
extern int	mkaddrpart(char *,int,const char *,int) ;
extern int	hdrextnum(const char *,int) ;
extern int	hdrextid(char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpyclean(char *,int,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwset(char *,int,int) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct procdata_flags {
	uint		plaintext:1 ;
	uint		ct_plaintext:1 ;
	uint		ce_text:1 ;
	uint		hdr_clines:1 ;
	uint		hdr_lines:1 ;
	uint		hdr_xlines:1 ;
	uint		eom:1 ;
	uint		eof:1 ;
	uint		spam:1 ;
} ;

struct procdata {
	const char	*tdname ;
	bfile		*ifp ;
	bfile		*tfp ;
	vechand		*alp ;
	vecstr		*nlp ;
	MAILMSG		*msgp ;
	ARTICLE		*aip ;
	BFLINER		bline ;
	VECSTR		wh ;
	PROCDATA_FL	f ;
	MAILMSGMATENV	me ;
	DATER		edate ;
	offset_t	offset ;
	offset_t	off_start, off_clen, off_body, off_finish ;
	offset_t	off_clines ;
	int		moff ;
	int		mlen ;		/* message length (calculated) */
	int		tlen ;		/* message length (calculated) */
	int		clen ;		/* content-length (calculated) */
	int		clines ;	/* content-lines (calculated) */
	int		mi ;
	char		efrom[MAILADDRLEN + 1] ;
} ;


/* forward references */

static int	procmsg(PROGINFO *, PROCDATA *,int) ;
static int	procmsgct(PROGINFO *,PROCDATA *,MAILMSG *) ;
static int	procmsgce(PROGINFO *,PROCDATA *,MAILMSG *) ;
static int	procmsghdrval(PROGINFO *,PROCDATA *,MAILMSG *,cchar *,int *) ;
static int	procspam(PROGINFO *,PROCDATA *,MAILMSG *) ;

static int	procmailmsg_spamflag(PROGINFO *,MAILMSG *) ;
static int	procmailmsg_spamstatus(PROGINFO *,MAILMSG *) ;
static int	procmailmsg_bogosity(PROGINFO *,MAILMSG *) ;

static int	procmsger(PROGINFO *,PROCDATA *) ;

static int	procmsgenv(PROGINFO *,PROCDATA *) ;
static int	procmsghdrs(PROGINFO *,PROCDATA *) ;
static int	procmsgout(PROGINFO *,PROCDATA *) ;
static int	procmsglog(PROGINFO *,PROCDATA *) ;

static int	procmsgenver(PROGINFO *,PROCDATA *,char *,int) ;
static int	procmsgenvdate(PROGINFO *,char *,int,const char *,int) ;
static int	procmsgenveraddr(PROGINFO *,PROCDATA *,
			STACKADDR *,const char *,int,const char *,int) ;
static int	procmsgenvmk(PROGINFO *,PROCDATA *,char *,int) ;

static int	procmsghdr_messageid(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_articleid(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_newsgroups(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_clen(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_clines(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_xmailer(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_received(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_replyto(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_errorsto(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_sender(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_deliveredto(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_xoriginalto(PROGINFO *,PROCDATA *) ;
static int	procmsghdr_xpriority(PROGINFO *,PROCDATA *) ;

static int	procmsgoutenv(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdrs(PROGINFO *,PROCDATA *) ;
static int	procmsgouteol(PROGINFO *,PROCDATA *) ;
static int	procmsgoutbody(PROGINFO *,PROCDATA *) ;
static int	procmsgoutback(PROGINFO *,PROCDATA *) ;

static int	procmsgouthdr_xenv(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_returnpath(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_received(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_clen(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_clines(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_messageid(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_remaining(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_status(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_references(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_from(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_to(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_cc(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_bcc(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_date(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_subject(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_expires(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_articleid(PROGINFO *,PROCDATA *) ;
#if	CF_NEWSGROUPS
static int	procmsgouthdr_newsgroups(PROGINFO *,PROCDATA *) ;
#endif

#ifdef	COMMENT
static int	procmsgouthdr_deliveredto(PROGINFO *,PROCDATA *) ;
#endif

static int	proclogenv(PROGINFO *,int,MAILMSG_ENVER *) ;
static int	procmsglogaddr(PROGINFO *,const char *,
			const char *,int) ;
static int	procmsglogema(PROGINFO *,const char *,EMA *) ;
static int	procmsglog_date(PROGINFO *,const char *,int) ;

static int	cheapspamcheck(PROGINFO *,const char *,int) ;
static int	vcmpheadname(const char **,const char **) ;

static int	mkarticlefile(PROGINFO *,PROCDATA *,char *,
			const char *) ;
static int	mknewhdrname(char *,int,const char *) ;


/* local variables */

#ifdef	COMMENT
static const char	atypes[] = "LUIR" ;	/* address types */
#endif

static const char	blanks[] = "                    " ;

static int (*msghdrgets[])(PROGINFO *,PROCDATA *) = {
	procmsghdr_messageid,
	procmsghdr_articleid,
	procmsghdr_newsgroups,
	procmsghdr_clen,
	procmsghdr_clines,
	procmsghdr_xmailer,
	procmsghdr_received,
	procmsghdr_replyto,
	procmsghdr_errorsto,
	procmsghdr_sender,
	procmsghdr_deliveredto,
	procmsghdr_xoriginalto,
	procmsghdr_xpriority,
	NULL
} ;

static int (*msgouthdrs[])(PROGINFO *,PROCDATA *) = {
	procmsgouthdr_xenv,
	procmsgouthdr_returnpath,
	procmsgouthdr_received,
	procmsgouthdr_clen,
	procmsgouthdr_clines,
	procmsgouthdr_messageid,
	procmsgouthdr_remaining,
	procmsgouthdr_status,
	procmsgouthdr_references,
	procmsgouthdr_from,
	procmsgouthdr_to,
	procmsgouthdr_cc,
	procmsgouthdr_bcc,
	procmsgouthdr_date,
	procmsgouthdr_subject,
	procmsgouthdr_expires,
	procmsgouthdr_articleid,
#if	CF_NEWSGROUPS
	procmsgouthdr_newsgroups,
#endif
#ifdef	COMMENT
	procmsgouthdr_deliveredto,
#endif
	NULL
} ;

/* mark the following headers as done since we "do" them at the end */
static const char	*hdrspecials[] = {
	"status",
	"references",
	"from",
	"to",
	"cc",
	"date",
	"subject",
	NULL
} ;


/* exported subroutines */


int progarts(pip,tip,alp,ifp,nlp)
PROGINFO	*pip ;
struct tdinfo	*tip ;
vechand		*alp ;
bfile		*ifp ;
vecstr		*nlp ;
{
	PROCDATA	pd, *pdp = &pd ;
	BFLINER		*blp ;
	offset_t	boff ;
	const int	llen = MSGLINELEN ;
	int		rs ;
	int		vi ;
	int		mi = 0 ;
	int		f_bol, f_eol ;
	int		f_env, f_hdr, f_eoh ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts: ent\n") ;
#endif

	memset(pdp,0,sizeof(PROCDATA)) ;
	pdp->tdname = tip->tdname ;
	pdp->alp = alp ;
	pdp->ifp = ifp ;
	pdp->nlp = nlp ;
	pdp->clines = -1 ;

	    blp = &pd.bline ;
	if ((rs = bfliner_start(blp,ifp,0L,-1)) >= 0) {
	        int		ll = 0 ;
	        const char	*lp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts: 2\n") ;
#endif

/* find the start of a message */

	    while ((rs >= 0) && (! pdp->f.eof)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts: 4\n") ;
#endif
		    f_env = FALSE ;
		    f_hdr = FALSE ;
		    f_eoh = FALSE ;
	            f_bol = TRUE ;
	       while ((rs = bfliner_readline(blp,llen,&lp)) > 0) {
	                ll = rs ;
	                if (ll == 0) break ;

#if	CF_DEBUG
			if (DEBUGLEVEL(3))
	                debugprintf("progarts_gathermsg: line=>%t<\n",
	                    lp,strlinelen(lp,ll,40)) ;
#endif

	           f_eol = (lp[ll-1] == '\n') ;

	           pd.offset += ll ;
	           if (f_bol && (ll > 5) && FMAT(lp) && 
			((rs = mailmsgmatenv(&pd.me,lp,ll)) > 0)) {
	                    f_env = TRUE ;
	           } else if (f_bol && (ll > 2) && 
			((rs = mailmsgmathdr(lp,ll,&vi) > 0))) {
	                    f_hdr = TRUE ;
	           } else if (f_bol && (ll <= 2) && (mi == 0)) {
			if ((lp[0] == '\n') || hasEOH(lp,ll)) {
	                    f_eoh = TRUE ;
			}
	           }

		   if (rs < 0) break ;
	                bfliner_getpoff(blp,&boff) ;
	                pd.off_start = boff ;

	                if (f_env || f_hdr) break ;

	                ll = 0 ;
	                bfliner_readover(blp) ;

	                f_bol = f_eol ;
	                if (f_eoh) break ;
	            } /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	            debugprintf("progarts_gathermsg: while-out rs=%d\n",rs) ;
	            debugprintf("progarts_gathermsg: ll=%d\n",ll) ;
	            debugprintf("progarts_gathermsg: f_eoh=%u\n", f_eoh) ;
	            debugprintf("progarts_gathermsg: f_env=%u\n", f_env) ;
	            debugprintf("progarts_gathermsg: f_hdr=%u\n", f_hdr) ;
	}
#endif

/* EOF already? */

	            if (rs < 0) break ;

	            if ((! f_eoh) && (! f_env) && (! f_hdr)) break ;

#ifdef	COMMENT
	            if (ll == 0) break ;
#endif

	            if (pip->f.logmsg)
	                proglog_printf(pip,"message %4u",pip->nmsgs) ;

	            rs = procmsg(pip,&pd,f_eoh) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progarts: procmsg() rs=%d\n",rs) ;
#endif

	            if (rs <= 0) break ;

	            pip->nmsgs += 1 ;
	            mi += 1 ;

	            pd.efrom[0] = '\0' ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progarts: f_eof=%u\n",pdp->f.eof) ;
#endif

	            if (pd.f.eof) break ;
	        } /* end while (processing article messages) */

	        bfliner_finish(blp) ;
	    } /* end if (bfliner) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts: ret rs=%d mi=%u\n",rs,mi) ;
#endif

	return (rs >= 0) ? mi : rs ;
}
/* end subroutine (progarts) */


/* local subroutines */


/* process the current message */
static int procmsg(pip,pdp,f_eoh)
PROGINFO	*pip ;
PROCDATA	*pdp ;
int		f_eoh ;
{
	BFLINER		*blp = &pdp->bline ;
	MAILMSG		amsg, *msgp = &amsg ;
	const int	llen = MSGLINELEN ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: ent f_eoh=%u\n",f_eoh) ;
#endif

	pdp->tlen = 0 ;
	pdp->f.eom = FALSE ;
	pdp->f.spam = FALSE ;

	if ((rs = mailmsg_start(msgp)) >= 0) {
	    int		ll ;
	    const char	*lp ;
	    pdp->msgp = msgp ;

/* process the MAILMSG file */

	    if (! f_eoh) {
	        while ((rs = bfliner_readline(blp,llen,&lp)) > 0) {
	            ll = rs ;
	            if ((rs = mailmsg_loadline(msgp,lp,ll)) >= 0) {
	                pdp->offset += rs ;
		        bfliner_readover(blp) ;
		    }
		    if (rs <= 0) break ;
	        } /* end while */
	    } /* end if (did not yet see EOH) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procmsger(pip,pdp) ;
	        len = rs ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: mid2 rs=%d\n",rs) ;
#endif

	    rs1 = mailmsg_finish(msgp) ;
	    if (rs >= 0) rs = rs1 ;
	    pdp->msgp = NULL ;
	} /* end if (mailmsg) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmsg) */


static int procmsger(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	MAILMSG		*msgp = pdp->msgp ;
	ARTICLE		*aip ;
	const int	msize = sizeof(ARTICLE) ;
	int		rs ;
	int		tlen = 0 ;

	if ((rs = uc_malloc(msize,&aip)) >= 0) {
	    if ((rs = article_start(aip)) >= 0) {
	        pdp->aip = aip ;

	            if ((rs = vecstr_start(&pdp->wh,10,0)) >= 0) {

	                rs = procmsgenv(pip,pdp) ;

	                if (rs >= 0)
	                    rs = procmsghdrs(pip,pdp) ;

	                if (rs >= 0) {
	                    rs = procmsgout(pip,pdp) ;
	                    tlen += rs ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("progarts/procmsger: "
				"mid rs=%d mlen=%u\n",
				rs,pdp->mlen) ;
	                    debugprintf("progarts/procmsger: something\n") ;
#endif

	                if (rs >= 0)
	                    rs = procmsglog(pip,pdp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts/procmsger: "
		"procmsglog() rs=%d\n",rs) ;
#endif

	                vecstr_finish(&pdp->wh) ;
	            } /* end if (vecstr) */

	        if (rs >= 0) {
		    pdp->mi += 1 ;
		    rs = vechand_add(pdp->alp,aip) ;
		}

	        if (rs < 0)
		    article_finish(aip) ;
	        pdp->aip = NULL ;
	    } /* end if (article-start) */
	    if (rs < 0)
		uc_free(aip) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts/procmsger: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsger) */


/* process the envelope information */
static int procmsgenv(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	const int	salen = STACKADDRLEN ;
	int		rs ;
	int		sal = -1 ;
	int		c = 0 ;
	char		sabuf[STACKADDRLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgenv: procmsgenver()\n") ;
#endif

	rs = procmsgenver(pip,pdp,sabuf,salen) ;
	c = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgenv: procmsgenver() rs=%d\n",rs) ;
	    debugprintf("progarts/procmsgenv: efrom=%s\n",sabuf) ;
	    debugprintf("progarts/procmsgenv: f_trusted=%u\n",pip->f.trusted) ;
	}
#endif

/* form the envelope address that we will be using for this message */

	if ((rs >= 0) && (c == 0)) {
	    rs = procmsgenvmk(pip,pdp,sabuf,salen) ;
	    sal = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgenv: mid rs=%d sal=%d\n",rs,sal) ;
	    debugprintf("progarts/procmsgenv: s=%t\n",sabuf,sal) ;
	}
#endif

	if (rs >= 0) {
	    const int	st = articlestr_envfrom ;
	    rs = article_addstr(aip,st,sabuf,sal) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgenv: ret rs=%d\n",rs) ;
	    debugprintf("progarts/procmsgenv: sa=%t\n",sabuf,sal) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgenv) */


/* process MAILMSG envelope information */
static int procmsgenver(pip,pdp,abuf,alen)
PROGINFO	*pip ;
PROCDATA	*pdp ;
char		abuf[] ;
int		alen ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	STACKADDR	s ;
	int		rs ;
	int		rs1 ;
	int		i, sl ;
	int		cl ;
	int		ml ;
	int		al = 0 ;
	int		abl = alen , sabl ;
	int		sal = 0 ;
	int		ai = 0 ;
	int		c = 0 ;
	int		f_remote ;
	const char	*cp ;
	char		*ap = abuf ;
	char		*sap = abuf ;
	char		*addr = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgenver: ent\n") ;
#endif

	if ((rs = stackaddr_start(&s,abuf,alen)) >= 0) {
	    MAILMSG_ENVER	me ;

	    for (i = 0 ; mailmsg_enver(msgp,i,&me) >= 0 ; i += 1) {
	        MAILMSG_ENVER	*mep = &me ;
	        EMAINFO		ai ;
	        int		froml = -1 ;
	        const char	*fromp = NULL ;

	        if (pip->open.logenv)
		    rs = proclogenv(pip,i,mep) ;

	        fromp = mep->a.ep ;
	        froml = mep->a.el ;
	        if ((fromp == NULL) || (fromp[0] == '\0')) {
	            fromp = "mailer-daemon" ;
	            froml = -1 ;
	        }

	        if (rs >= 0) {
		    const int	dlen = DATEBUFLEN ;
		    char	dbuf[DATEBUFLEN + 1] ;
		    cp = mep->d.ep ;
		    cl = mep->d.el ;
	            if ((rs = procmsgenvdate(pip,dbuf,dlen,cp,cl)) > 0) {
		        c += 1 ;
		        rs = article_addenvdate(aip,&pip->td) ;
		    }
		    if ((rs >= 0) && pip->open.logprog)
			proglog_printf(pip,"  env %s",dbuf) ;
	        }

		if (rs >= 0) {
	            const char	*hp = mep->r.ep ;
	            const int	hl = mep->r.el ;
	            if (strnchr(fromp,froml,'@') != NULL) {
		        rs = procmsgenveraddr(pip,pdp,&s,hp,hl,fromp,froml) ;
	            } else {
	                rs = stackaddr_add(&s,hp,hl,fromp,froml) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progarts/procmsgenver: r=%t f=%t\n",
		hp,hl,fromp,froml) ;
	    debugprintf("progarts/procmsgenver: stackaddr_add() rs=%d\n",rs) ;
	}
#endif
		    }
		    if ((rs >= 0) && pip->open.logprog) {
			const char	*fmt ;
			if (hp != NULL) {
			    fmt = "  env %t!%t" ;
		    	    proglog_printf(pip,fmt,hp,hl,fromp,froml) ;
			} else {
			    fmt = "  env %t" ;
		    	    proglog_printf(pip,fmt,fromp,froml) ;
			}
		    }
		}

	        if (rs < 0) break ;
	    } /* end for (looping through envelopes) */

	    rs1 = stackaddr_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progarts/procmsgenver: ret rs=%d c=%u\n",rs,c) ;
	    debugprintf("progarts/procmsgenver: sa=%s\n",abuf) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgenver) */


static int procmsgenveraddr(pip,pdp,sap,hp,hl,up,ul)
PROGINFO	*pip ;
PROCDATA	*pdp ;
STACKADDR	*sap ;
const char	*hp ;
int		hl ;
const char	*up ;
int		ul ;
{
	int		rs ;

	if (strnchr(up,ul,'@') != NULL) {
	    const int	blen = strnlen(up,ul) + 4 ;
	    char	*bp ;
	    if ((rs = uc_malloc((blen+1),&bp)) >= 0) {
		EMAINFO	ai ;
	        if ((rs = emainfo(&ai,up,ul)) >= 0) {
		    const int	at = EMAINFO_TUUCP ;
		    int		bl ;
	            if ((bl = emainfo_mktype(&ai,at,bp,blen)) >= 0) {
	    		rs = stackaddr_add(sap,hp,hl,bp,bl) ;
		    }
		}
		uc_free(bp) ;
	    } /* end if (memory-allocation) */
	} else
	    rs = stackaddr_add(sap,hp,hl,up,ul) ;

	return rs ;
}
/* end subroutine (procmsgenveraddr) */


static int procmsgenvdate(pip,dbuf,dlen,ep,el)
PROGINFO	*pip ;
char		dbuf[] ;
int		dlen ;
const char	*ep ;
int		el ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	dbuf[0] = '\0' ;
	if ((rs1 = dater_setstd(&pip->td,ep,el)) >= 0) {
	    c += 1 ;
	    rs = dater_mkstrdig(&pip->td,dbuf,dlen) ;
	} else if ((rs1 == SR_INVALID) || (rs1 == SR_DOM)) {
	    rs = bufprintf(dbuf,dlen,"¿¿ %t ¿¿",ep,el) ;
	} else if (rs1 < 0)
	    rs = rs1 ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgenvdate) */


static int procmsgenvmk(PROGINFO *pip,PROCDATA *pdp,char *sabuf,int salen)
{
	int		rs = SR_OK ;
	int		sal = 0 ;

	if (sabuf[0] == '\0') {
	    cchar	*cp = pip->username ;
	    if (pip->f.trusted) {
	        cp = pip->envfromaddr ;
	    }
	    rs = sncpy1(sabuf,salen,cp) ;
	    sal = rs ;
	} /* end if (needed to create an address) */

#ifdef	COMMENT
	if (pip->open.logprog && pip->f.logmsg) {
	    proglog_printf(pip,"  F %t",sabuf,sal) ;
	}
#endif /* COMMENT */

	return (rs >= 0) ? sal : rs ;
}
/* end subroutine (procmsgenvmk) */


static int procmsghdrs(PROGINFO *pip,PROCDATA *pdp)
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; msghdrgets[i] != NULL ; i += 1) {
	    rs = (*msghdrgets[i])(pip,pdp) ;
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (procmsghdrs) */


/* mailmsg message-ID */
static int procmsghdr_messageid(PROGINFO *pip,PROCDATA *pdp)
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	const int	mlen = MAILADDRLEN ;
	int		rs ;
	int		vl ;
	int		ml = 0 ;
	int		f_messageid = FALSE ;
	const char	*hdr = HN_MESSAGEID ;
	const char	*vp ;
	char		mbuf[MAILADDRLEN+1] = { 0 } ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_MESSAGEID)) >= 0) {

	        if ((rs = hdrextid(mbuf,mlen,vp,vl)) == 0) {
	            ml = rs ;
	        } else if (isNotValid(rs)) {
	            rs = SR_OK ;
		}
	    } /* end if (do not copy header) */
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: 1 messageid=%s\n",mbuf) ;
#endif

	if (rs >= 0) {

	    f_messageid = TRUE ;
	    if (mbuf[0] == '\0') {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progarts/procmsg: making messageid\n") ;
#endif

	        f_messageid = FALSE ;
	        rs = progmsgid(pip,mbuf,mlen,pdp->mi) ;
	        ml = rs ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: 2 messageid=%s\n",mbuf) ;
#endif /* CF_DEBUG */

	if (rs >= 0) {
	    const int	st = articlestr_messageid ;
	    rs = article_addstr(aip,st,mbuf,ml) ;
	}

/* log the MID if logging enabled */

	if ((rs >= 0) && pip->f.logmsg) {
	    int	f_first = TRUE ;
	    const char	*fmt ;
	    const char	*mp = mbuf ;
	    const char	*cp ;
	    int		cl ;

	    while (ml > 0) {

	        cp = mp ;
	        cl = MIN(ml,(LOGFMTLEN - TABLEN)) ;

	        if (f_first) {
	            f_first = FALSE ;
	            fmt = "  mid=| %t" ;
	        } else
	            fmt = "      | %t" ;

	        proglog_printf(pip,fmt,cp,cl) ;

	        mp += cl ;
	        ml -= cl ;

	    } /* end while */

	} /* end if (logging MID) */

	return rs ;
}
/* end subroutine (procmsghdr_messageid) */


/* article-ID */
static int procmsghdr_articleid(PROGINFO *pip,PROCDATA *pdp)
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_ARTICLEID ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_ARTICLEID)) >= 0) {

		if (pip->open.logprog && pip->f.logmsg) {
		    const int	mlen = MAXNAMELEN ;
		    char	mbuf[MAXNAMELEN+1] ;
	            if ((rs = hdrextid(mbuf,mlen,vp,vl)) >= 0) {
			const char	*fmt = "  prev-articleid=%t" ;
	                proglog_printf(pip,fmt,mbuf,rs) ;
	            } else if ((rs == SR_NOTFOUND) || isNotValid(rs))
	                rs = SR_OK ;
		} /* end if (logging) */

	    } /* end if (do not copy header) */
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_articleid) */


/* news-groups */
static int procmsghdr_newsgroups(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_NEWSGROUPS ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_NEWSGROUPS)) >= 0) {
	    const int	at = articleaddr_newsgroups ;

	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }

	    } /* end if (do not copy header) */
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_newsgroups) */


/* mailmsg content-length */
static int procmsghdr_clen(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_CLEN ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_CLEN)) >= 0) {

	        if ((rs = hdrextnum(vp,vl)) >= 0) {
	            aip->clen = rs ;
	        } else if (isNotValid(rs))
	            rs = SR_OK ;

	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progarts/procmsg: clen=%d\n",aip->clen) ;
#endif

	return rs ;
}
/* end subroutine (procmsghdr_clen) */


/* mailmsg content-lines */
static int procmsghdr_clines(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		rs1 ;
	int		v = -1 ;

	rs = procmsgct(pip,pdp,msgp) ;

	if (rs >= 0)
	    rs = procmsgce(pip,pdp,msgp) ;

	pdp->f.plaintext = pdp->f.ct_plaintext && pdp->f.ce_text ;
	if ((rs >= 0) && pdp->f.plaintext) {

	    rs1 = procmsghdrval(pip,pdp,msgp,HN_CLINES,&v) ;
	    pdp->f.hdr_clines = ((rs1 >= 0) && (v >= 0)) ;

	    if ((rs1 == SR_NOTFOUND) || (v < 0)) {
	        rs1 = procmsghdrval(pip,pdp,msgp,HN_LINES,&v) ;
	        pdp->f.hdr_lines = ((rs1 >= 0) && (v >= 0)) ;
	    }

	    if ((rs1 == SR_NOTFOUND) || (v < 0)) {
	        rs1 = procmsghdrval(pip,pdp,msgp,HN_XLINES,&v) ;
	        pdp->f.hdr_xlines = ((rs1 >= 0) && (v >= 0)) ;
	    }

	} /* end if (plain text) */

	aip->clines = v ;

	return rs ;
}
/* end subroutine (procmsghdr_clines) */


static int procmsghdr_xmailer(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	int		vl ;
	const char	*hdr = HN_XMAILER ;
	const char	*vp ;

#if	CF_XMAILER
	if (pip->f.logmsg) {
	    int	n ;
	    int	i ;

	    n = mailmsg_hdrcount(msgp,hdr) ;
	    if (n < 0)
	        n = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progarts/procmsg: xmailer n=%d\n",n) ;
#endif

	    for (i = 0 ; (vl = mailmsg_hdrival(msgp,hdr,i,&vp)) >= 0 ; i += 1) {
	        const char	*fmt ;

	        proglog_printf(pip,"  xmailer %u",(n - i - 1)) ;

	        fmt = (strchr(vp,' ') != NULL) ? "    >%t<" : "    %t" ;
	        proglog_printf(pip,fmt,vp,vl) ;

	    } /* end for */

	} /* end if (mailer) */
#endif /* CF_XMAILER */

	return rs ;
}
/* end subroutine (procmsghdr_xmailer) */


static int procmsghdr_received(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	const int	dlen = DATEBUFLEN ;
	const int	salen = STACKADDRLEN ;
	int		rs = SR_OK ;
	int		vl ;
	const char	*hdr = HN_RECEIVED ;
	const char	*vp ;
	char		dbuf[DATEBUFLEN+1] ;
	char		sabuf[STACKADDRLEN+1] ;

#if	CF_RECEIVED
	if (pip->f.logmsg) {
	    int	n, ri ;
	    int	i ;

	    n = mailmsg_hdrcount(msgp,HN_RECEIVED) ;
	    if (n < 0)
	        n = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progarts/procmsg: received n=%d\n",n) ;
#endif

	    for (i = (n - 1) ; i >= 0 ; i -= 1) {
	        RECEIVED	rh ;

	        vl = mailmsg_hdrival(msgp,hdr,i,&vp) ;
	        if (vl < 0) break ;

	        if (vl == 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progarts/procmsg: i=%u r=>%t<\n",
	                i,vp,strlinelen(vp,vl,40)) ;
#endif

	        if ((rs = received_start(&rh,vp,vl)) >= 0) {
	            int		rl ;
	            int		j ;
	            const char	*fmt ;
	            const char	*sp ;
	            const char	*rp ;

	            ri = (n - 1) - i ;
	            proglog_printf(pip,"  received %u",ri) ;

	            for (j = 0 ; (rl = received_getkey(&rh,j,&rp)) >= 0 ; 
	                j += 1) {

	                if (rp == NULL) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("progarts/procmsg: j=%u r=>%t<\n",
	                        j,rp,strlinelen(rp,rl,40)) ;
#endif

	                sp = rp ;
	                if (j == received_keydate) {
			    int	rs1 = dater_setmsg(&pip->td,rp,rl) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("progarts/procmsg: "
					"dater_setmsg() rs=%d\n",
	                            rs1) ;
#endif

			    if (rs1 >= 0) {
			        DATER	*dp = &pip->td ;

				sp = dbuf ;
	                        rs = dater_mkstrdig(dp,dbuf,dlen) ;

			    } else {
				const char *fmt = "** bad date-spec (%d) **" ;
				sp = dbuf ;
				bufprintf(dbuf,dlen,fmt,rs1) ;
			    }

	                } else if (j == received_keyfor) {

			    sp = sabuf ;
	                    rs = mkbestaddr(sabuf,salen,rp,rl) ;

	                } /* end if (special handling cases) */

			if (sp != NULL) {

	                    fmt = "    %s=%s" ;
	                    if (strchr(sp,' ') != NULL)
	                        fmt = "    %s=>%s<" ;

	                    proglog_printf(pip,fmt,
	                        received_keys[j],sp) ;

			} /* end if (something to log) */

	                if (rs < 0) break ;
	            } /* end for */

	            received_finish(&rh) ;
	        } /* end if (initialized) */

	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progarts/procmsg: received-done\n") ;
#endif

	} /* end if */
#endif /* CF_RECEIVED */

	return rs ;
}
/* end subroutine (procmsghdr_received) */


static int procmsghdr_replyto(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_REPLYTO ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    const int	at = articleaddr_replyto ;
	    vl = rs ;
	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_replyto) */


static int procmsghdr_errorsto(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_ERRORSTO ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    const int	at = articleaddr_errorsto ;
	    vl = rs ;
	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_errorsto) */


static int procmsghdr_sender(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_SENDER ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    const int	at = articleaddr_sender ;
	    vl = rs ;
	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_sender) */


static int procmsghdr_deliveredto(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_DELIVEREDTO ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    const int	at = articleaddr_deliveredto ;
	    vl = rs ;
	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_deliveredto) */


static int procmsghdr_xoriginalto(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_XORIGINALTO ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    const int	at = articleaddr_xoriginalto ;
	    vl = rs ;
	    if ((rs = article_addaddr(aip,at,vp,vl)) >= 0) {
		EMA	*emap ;
		if ((rs = article_getaddrema(aip,at,&emap)) >= 0)
	            rs = procmsglogema(pip,hdr,emap) ;
	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsghdr_xoriginal) */


static int procmsghdr_xpriority(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	int		hl ;
	const char	*hdr = HN_XPRIORITY ;
	const char	*hp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: hdr=%s\n",hdr) ;
#endif
	if (pip->f.logmsg) {
	if ((rs = mailmsg_hdrval(msgp,hdr,&hp)) >= 0) {
	    COMPARSE	com ;
	    hl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: h=%t\n",hp,hl) ;
#endif
	    if ((rs = comparse_start(&com,hp,hl)) >= 0) {
		const char	*vp ;
	        if ((rs = comparse_getval(&com,&vp)) > 0) {
		    const char	*cp ;
		    int		vl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: v=%t\n",vp,vl) ;
#endif
	            if ((rs = comparse_getcom(&com,&cp)) >= 0) {
		        const int	plen = 20 ;
		        const int	hlen = HDRNAMELEN ;
		        int		cl = rs ;
			int		hnl ;
		        const char	*hnp = hdr ;
		        char		hbuf[HDRNAMELEN + 1] ;

		        if ((rs = mknewhdrname(hbuf,hlen,hdr)) > 0) {
	                        hnp = hbuf ;
	                        hnl = rs ;
		        } else
	                        hnl = strlen(hnp) ;

		        if (rs >= 0) {
			    const char	*fmt = "  %t=%t (%t)" ;
			    if (vl > plen) vl = plen ;
			    if (cl > plen) cl = plen ;
			    if (cl == 0) fmt = "  %t=%t" ;
			    proglog_printf(pip,fmt,hnp,hnl,vp,vl,cp,cl) ;
		        }

		    } /* end if (comparse-get) */
		} /* end if (comparse-get) */
		comparse_finish(&com) ;
	    } /* end if (comparse) */
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;
	} /* end if (msg-logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsghdr_xpriority) */


/* start writing the output file */
static int procmsgout(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		tlen = 0 ;
	char		afname[MAXPATHLEN+1] ;

	if ((rs = mkarticlefile(pip,pdp,afname,pdp->tdname)) >= 0) {
	    bfile	tfile, *tfp = &tfile ;
	    const int	afl = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: afname=%s\n",afname) ;
#endif

	    if ((rs = bopen(tfp,afname,"rwct",0666)) >= 0) {
		pdp->tfp = tfp ;

	        if (rs >= 0) {
	            rs = procmsgoutenv(pip,pdp) ;
	            tlen += rs ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: in1 rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            rs = procmsgouthdrs(pip,pdp) ;
	            tlen += rs ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: in2 rs=%d\n",rs) ;
#endif
	        if (rs >= 0) {
	            rs = procmsgouteol(pip,pdp) ;
	            tlen += rs ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: in3 rs=%d\n",rs) ;
#endif
	        if (rs >= 0) {
	            rs = procmsgoutbody(pip,pdp) ;
	            tlen += rs ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: in4 rs=%d\n",rs) ;
#endif
	        if (rs >= 0) {
	            rs = procmsgoutback(pip,pdp) ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: end rs=%d\n",rs) ;
#endif

		pdp->tfp = NULL ;
	        bclose(tfp) ;
	    } /* end if (open-article-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: open-out rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
		uc_unlink(afname) ;
	    }
	} /* end if (making article-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgout: rs=%d tlen=%u\n",rs,tlen) ;
#endif

	pdp->mlen = tlen ;
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgout) */


/* write out our own (new) envelope */
static int procmsgoutenv(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	DATER		*tdp = &pip->td ;
	time_t		dt = pip->daytime ;
	const int	isdst = pip->now.dstflag ;
	const int	zoff = pip->now.timezone ;
	int		rs ;
	int		tlen = 0 ;
	const char	*znp = pip->zname ;

	if ((rs = dater_settimezon(tdp,dt,zoff,znp,isdst)) >= 0) {

	    if ((rs = article_countenvdate(aip)) > 0) {
	        DATER		*edp ;
	        const int	n = (rs-1) ;
	        if ((rs = article_getenvdate(aip,n,&edp)) >= 0) {
		    DATER_ZINFO	zi ;
	            if ((rs = dater_getzinfo(edp,&zi)) >= 0) {
		        rs = dater_setzinfo(tdp,&zi) ;
		    } else if (isNotPresent(rs))
		        rs = SR_OK ;
	        }
	    } /* end if (had some existing envelope dates) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgoutenv: mid rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        const int	st = articlestr_envfrom ;
		const char	*sp ;

	        if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	            const int	dlen = DATEBUFLEN ;
	            char	dbuf[DATEBUFLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progarts/procmsgoutenv: article_getstr() rs=%d\n",rs) ;
	    debugprintf("progarts/procmsgoutenv: s=%s\n",sp) ;
	}
#endif
	            if ((rs = dater_mkstd(tdp,dbuf,dlen)) >= 0) {
		        const char	*fmt = "From %s %s\n" ;
	                rs = bprintf(pdp->tfp,fmt,sp,dbuf) ;
	                pdp->tlen += rs ;
	                tlen += rs ;
		    } /* end if (dater-mkstd) */
	        } else if (rs == SR_NOENT)
		    rs = SR_OK ;

	    } /* end if */

	} /* end if (set current time) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgoutenv: ret rs=%d tlen=%u\n",
		rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgoutenv) */


static int procmsgouthdrs(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	int		i ;
	int		slen = pdp->tlen ;
	int		tlen = 0 ;

	for (i = 0 ; msgouthdrs[i] != NULL ; i += 1) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progarts/procmsgouthdrs: i=%u\n",i) ;
#endif
	    rs = (*msgouthdrs[i])(pip,pdp) ;
	    if (rs < 0) break ;
	} /* end for */

	tlen = (pdp->tlen - slen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdrs: ret rs=%d tlen=%u\n",
	        rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgouthdrs) */


static int procmsgouthdr_xenv(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	BUFFER		b ;
	int		rs ;
	int		vl  = 0 ;
	const char	*hdr = "x-env" ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr: hdr=%s\n",hdr) ;
#endif

	if ((rs = buffer_start(&b,76)) >= 0) {
	    MAILMSG_ENVER	me ;
	    const int	dlen = DATEBUFLEN ;
	    int		i ;
	    char	dbuf[DATEBUFLEN + 1] ;
	    for (i = 0 ; mailmsg_enver(msgp,i,&me) >= 0 ; i += 1) {
	        MAILMSG_ENVER	*mep = &me ;
	   	const int	hl = mep->r.el ;
	        int		froml = mep->a.el ;
	        const char	*hp = mep->r.ep ;
	        const char	*fromp = mep->a.ep ;

		buffer_reset(&b) ;

	        if ((fromp == NULL) || (fromp[0] == '\0')) {
	            fromp = "mailer-daemon" ;
	            froml = -1 ;
	        }

	        if (rs >= 0) {
		    const char	*cp = mep->d.ep ;
		    const int	cl = mep->d.el ;
	            rs = procmsgenvdate(pip,dbuf,dlen,cp,cl) ;
		}

		if (rs >= 0) {
		    const char	*bp ;
		    buffer_strw(&b,dbuf,-1) ;
		    buffer_char(&b,' ') ;
		    if (hp != NULL) {
			buffer_strw(&b,hp,hl) ;
		        buffer_char(&b,'!') ;
		    }
		    buffer_strw(&b,fromp,froml) ;
		    if ((rs = buffer_get(&b,&bp)) >= 0) {
	        	rs = progprinthdr(pip,pdp->tfp,hdr,bp,rs) ;
	        	pdp->tlen += rs ;
		    }
		} /* end if */

	    } /* end for (looping through envelopes) */
	    buffer_finish(&b) ;
	} /* end if (buffer) */

	return rs ;
}
/* end subroutine (procmsgouthdr_xenv) */


static int procmsgouthdr_returnpath(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl  = 0 ;
	const char	*hdr = HN_RETURNPATH ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    rs = vecstr_add(hlp,hdr,-1) ;
	}

	if ((rs >= 0) && (vl > 0)) {

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,hdr,vp,vl) ;

	} else if (((rs >= 0) && (vl == 0)) || (rs == SR_NOTFOUND)) {
	    const int	st = articlestr_envfrom ;
	    const char	*sp ;

	    if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	        rs = progprinthdr(pip,pdp->tfp,hdr,sp,-1) ;
	        pdp->tlen += rs ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (procmsgouthdr_returnpath) */


static int procmsgouthdr_received(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	const char	*hdr = HN_RECEIVED ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_received: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrcount(msgp,hdr)) > 0) {

	    if ((rs = vecstr_add(hlp,hdr,-1)) >= 0) {

	        rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	        pdp->tlen += rs ;

	    } /* end if */

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_received: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_received) */


static int procmsgouthdr_clen(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	const char	*hdr = HN_CLEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_clen: hdr=%s\n",hdr) ;
#endif

	if (rs >= 0) {
	    rs = bprintf(pdp->tfp,"%s: ",hdr) ;
	    pdp->tlen += rs ;
	}

	if (rs >= 0) {
	    offset_t	coff ;
	    btell(pdp->tfp,&coff) ;
	    pdp->off_clen = coff ;
	    rs = bprintf(pdp->tfp,"%t\n",blanks,NBLANKS) ;
	    pdp->tlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_clen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_clen) */


static int procmsgouthdr_clines(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs = SR_OK ;
	const char	*hdr = HN_CLINES ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_clines: hdr=%s\n",hdr) ;
#endif

	if (pdp->f.plaintext && (aip->clines < 0)) {

	    if (rs >= 0) {
	        rs = bprintf(pdp->tfp,"%s: ",HN_CLINES) ;
	        pdp->tlen += rs ;
	    }

	    if (rs >= 0) {
	        offset_t	coff ;
	        btell(pdp->tfp,&coff) ;
	        pdp->off_clines = coff ;
	        rs = bprintf(pdp->tfp,"%t\n",blanks,NBLANKS) ;
	        pdp->tlen += rs ;
	    }

	    if (rs >= 0) rs = vecstr_add(hlp,HN_CLINES,HL_CLINES) ;
	    if (rs >= 0) rs = vecstr_add(hlp,HN_LINES,HL_LINES) ;
	    if (rs >= 0) rs = vecstr_add(hlp,HN_XLINES,HL_XLINES) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progarts/procmsgouthdr_clines: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_clines) */


static int procmsgouthdr_messageid(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	const int	st = articlestr_messageid ;
	int		rs ;
	const char	*hdr = HN_MESSAGEID ;
	const char	*sp ;

	if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	    bfile	*tfp = pdp->tfp ;
	    const int	sl = rs ;
	    int		msize ;
	    char	*buf ;
	    msize = (sl+3) ;
	    if ((rs = uc_malloc(msize,&buf)) >= 0) {
		char	*bp = buf ;
		*bp++ = CH_LANGLE ;
		bp = strwcpy(bp,sp,sl) ;
		*bp++ = CH_RANGLE ;
		*bp = '\0' ;
	        rs = progprinthdr(pip,tfp,hdr,buf,(bp-buf)) ;
	        pdp->tlen += rs ;
		uc_free(buf) ;
	    } /* end if (memory-allocation) */
	} /* end if (article-getstr) */

	return rs ;
}
/* end subroutine (procmsgouthdr_messageid) */


/* put out all remaining headers */
static int procmsgouthdr_remaining(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	const int	hdrlen = HDRNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		kl ;
	const char	*kp ;
	char		hdrname[HDRNAMELEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_remaining: looping\n") ;
#endif

	vecstr_sort(hlp,vcmpheadname) ;

	for (i = 0 ; (kl = mailmsg_hdrikey(msgp,i,&kp)) >= 0 ; i += 1) {
	    if (kp == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progarts/procmsgouthdr_remaining: hdr=%t\n",
	            kp,kl) ;
#endif

	    strwcpy(hdrname,kp,MIN(kl,hdrlen)) ;
	    rs1 = vecstr_search(hlp,hdrname,vcmpheadname,NULL) ;

	    if (rs1 == SR_NOTFOUND) {
	        if (matcasestr(hdrspecials,hdrname,-1) >= 0) rs1 = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progarts/procmsgouthdr_remaining: have rs1=%d\n",
	            rs1) ;
#endif

	    if (rs1 == SR_NOTFOUND) {
	        rs = progprinthdrs(pip,pdp->tfp,msgp,hdrname) ;
	        pdp->tlen += rs ;
	    } /* end if */

	    if (rs < 0) break ;
	} /* end for (putting all other headers out) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_remaining: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_remaining) */


static int procmsgouthdr_status(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	const int	slen = 10 ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_STATUS ;
	const char	*vp ;
	char		sbuf[10+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_status: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if (vl < slen) {
	        char	*bp = strdcpy1w(sbuf,slen,vp,vl) ;
	        if ((bp-sbuf) < slen) {
	            strwset(bp,' ',(slen-(bp-sbuf))) ;
	            vp = sbuf ;
	            vl = slen ;
	        }
	    }
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	    vp = blanks ;
	    vl = slen ;
	}

	if (rs >= 0) {
	    rs = bprintf(pdp->tfp,"%s: %t\n",hdr,vp,vl) ;
	    pdp->tlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_status: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_status) */


static int procmsgouthdr_references(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	const char	*hdr = HN_REFERENCES ;

	if ((rs = mailmsg_hdrcount(msgp,hdr)) > 0) {
	    rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_references) */


static int procmsgouthdr_from(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_FROM ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND) {
		EMA	*emap ;
		bfile	*tfp = pdp->tfp ;
		if ((rs = progmsgfromema(pip,&emap)) > 0) {
		    rs = progprinthdremas(pip,tfp,hdr,emap) ;
	    	    pdp->tlen += rs ;
		}
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("procmsgouthdr_from: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_from) */


static int procmsgouthdr_to(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_TO ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_to) */


static int procmsgouthdr_cc(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_CC ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_cc) */


static int procmsgouthdr_bcc(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	const char	*hdr = HN_BCC ;
	const char	*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_bcc) */


static int procmsgouthdr_date(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		tlen = 0 ;
	const char	*hdr = HN_DATE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_date: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrcount(msgp,hdr)) > 0) {
	    int		vl ;
	    const char	*vp ;

	    rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progarts/procmsgouthdr_date: "
	            "progprinthdrs() rs=%d\n",
	            rs) ;
#endif

	    if ((rs >= 0) && ((rs = mailmsg_hdrival(msgp,hdr,0,&vp)) >= 0)) {
	        vl = rs ;

		rs = procmsglog_date(pip,vp,vl) ;

	    } /* end if (first date header retrieved) */

	} else if ((rs == 0) || (rs == SR_NOTFOUND)) { /* add a DATE header */
	    DATER	*edp = NULL ;
	    if ((rs = article_countenvdate(aip)) > 0) {
	        DATER	*dp = NULL ;
		time_t	ti_oldest = pip->daytime ;
		time_t	ti_env ;
	        int	i ;
	        for (i = 0 ; article_getenvdate(aip,i,&dp) >= 0 ; i += 1) {
		    if ((rs = dater_gettime(dp,&ti_env)) >= 0) {
			if (ti_env < ti_oldest) {
			    ti_oldest = ti_env ;
			    edp = dp ;
			}
		    }
		    if (rs < 0) break ;
		} /* end for */
	    } else if ((rs == 0) || (rs == SR_NOTFOUND)) {
		time_t		dt = pip->daytime ;
		const int	isdst = pip->now.dstflag ;
		const int	zoff = pip->now.timezone ;
		const char	*znp = pip->zname ;
		edp = &pip->td ;
		rs = dater_settimezon(edp,dt,zoff,znp,isdst) ;
	    } /* end if */
	    if ((rs >= 0) && (edp != NULL)) {
		const int	dlen = DATEBUFLEN ;
		char		dbuf[DATEBUFLEN+1] ;
		if ((rs = dater_mkmsg(edp,dbuf,dlen)) >= 0) {
		    int		dl = rs ;
		    const char	*fmt = "%s: %t\n" ;
	            rs = bprintf(pdp->tfp,fmt,hdr,dbuf,dl) ;
	            pdp->tlen += rs ;
	            tlen += rs ;
		} /* end if (dater-mkstd) */
	     }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_date: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgouthdr_date) */


static int procmsgouthdr_subject(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		rs1 ;
	int		vl ;
	const char	*hdr = HN_SUBJECT ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_subject: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    const char	*cp ;
	    int		cl ;
	    vl = rs ;

	    rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progarts/procmsgouthdr_subject: "
	            "progprinthdrs() rs=%d\n",
	            rs) ;
#endif

	    if (rs >= 0) {

/* store the subject away as message information (MI) */

	        if ((cl = sfshrink(vp,vl,&cp)) >= 0) {
		    const int	msize = (cl+1) ;
		    char	*bp ;

		    if ((rs = uc_malloc(msize,&bp)) >= 0) {
		        const int	st = articlestr_subject ;

			strdcpyclean(bp,cl,'¿',cp,cl) ;
	                rs = article_addstr(aip,st,bp,cl) ;

	        	if (pip->f.logmsg)
	            	    proglog_printf(pip,"  subject=>%t<",cp,cl) ;

			uc_free(bp) ;
		    } /* end if (memory-allocation) */

/* check for SPAM */

#if	CF_SPAMSUBJECT
	        if ((rs >= 0) && (! pdp->f.spam) && pip->f.spam) {
	            rs1 = cheapspamcheck(pip,cp,cl) ;
	            pdp->f.spam = (rs1 > 0) ;
	        } /* end if (spam check) */
#endif /* CF_SPAMSUBJECT */

		} /* end if */

	    } /* end if */

	} else if ((rs == 0) || (rs == SR_NOTFOUND)) {
	    const char	*sp = pip->msgsubject ;
	    rs = SR_OK ;
	    if ((sp != NULL) && (sp[0] != '\0')) {

	        rs = progprinthdr(pip,pdp->tfp,hdr,sp,-1) ;
	        pdp->tlen += rs ;

	    } /* end if (SUBJECT) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_subject: mid rs=%d\n",rs) ;
#endif

/* check spam */

	if ((rs >= 0) && pip->f.spam && (! pdp->f.spam)) {
	    rs = procspam(pip,pdp,msgp) ;
	    aip->f.spam = pdp->f.spam ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_subject: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_subject) */


static int procmsgouthdr_expires(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	const char	*hdr = HN_EXPIRES ;

	if (pip->ti_expires > 0) {
	    const char	*vp ;
	    if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) == SR_NOTFOUND) {
		const char	*sp ;
	        if ((rs = progexpiration(pip,&sp)) > 0) {
	            rs = progprinthdr(pip,pdp->tfp,hdr,sp,rs) ;
	            pdp->tlen += rs ;
	        }
	    } /* end if (needed header) */
	} /* end if (have expiration time) */

	return rs ;
}
/* end subroutine (procmsgouthdr_expires) */


static int procmsgouthdr_articleid(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	const int	st = articlestr_articleid ;
	int		rs ;
	const char	*sp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_articleid: ent\n") ;
#endif

	if ((rs = article_getstr(aip,st,&sp)) >= 0) {
	    bfile	*tfp = pdp->tfp ;
	    const char	*hdr = HN_ARTICLEID ;
	    rs = bprintf(tfp,"%s: %s\n",hdr,sp) ;
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_articleid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_articleid) */


#if	CF_NEWSGROUPS
static int procmsgouthdr_newsgroups(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	bfile		*tfp = pdp->tfp ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*hdr = HN_NEWSGROUPS ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: ent\n") ;
#endif

	if ((rs = progprinthdrs(pip,tfp,msgp,hdr)) >= 0) {
	    EMA		*emap ;
	    const int	at = articleaddr_newsgroups ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: p0 rs=%d\n",rs) ;
#endif

	    if ((rs = article_getaddrema(aip,at,&emap)) >= 0) {
		BUFFER		b ;

		if ((rs = buffer_start(&b,76)) >= 0) {
	            vecstr	*nlp = pdp->nlp ;
		    int		c = 0 ;
	            int		i ;
		    int		nl ;
		    int		bl ;
		    const char	*np ;
		    const char	*bp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: p1 rs=%d\n",rs) ;
#endif
	            for (i = 0 ; vecstr_get(nlp,i,&np) >= 0 ; i += 1) {
		        if (np == NULL) continue ;
		        nl = strlen(np) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: "
			"p2 n=%t\n",np,nl) ;
#endif
	                if ((rs = ema_haveaddr(emap,np,nl)) == 0) {
			    if (c++ > 0) rs = buffer_strw(&b,", ",2) ;
			    if (rs >= 0) rs = buffer_strw(&b,np,nl) ;
			    if (pip->f.logmsg) {
			        if (nl > 50) nl = 50 ;
			        proglog_printf(pip,"  %s»%t",hdr,np,nl) ;
			    }
		        }
		        if (rs < 0) break ;
		    } /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: "
			"for-out rs=%d\n",rs) ;
#endif

		    if ((rs >= 0) && ((rs = buffer_get(&b,&bp)) >= 0)) {
			bl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: "
			"b=%t\n",bp,strlinelen(bp,bl,50)) ;
#endif
	                if ((rs = progprinthdr(pip,tfp,hdr,bp,bl)) >= 0) {
	                    pdp->tlen += rs ;
			} /* end if (print-hdr) */
		    }

		    buffer_finish(&b) ;
		} /* end if (buffer) */

	    } /* end if (article-getaddr) */
	} /* end if (prog-print-hdrs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_newsgroups: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_newsgroups) */
#endif /* CF_NEWSGROUPS */


#ifdef	COMMENT
static int procmsgouthdr_deliveredto(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	bfile		*tfp = pdp->tfp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	const char	*hdr = HN_DELIVEREDTO ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_deliveredto: ent\n") ;
#endif

	if ((vl = mailmsg_hdrval(msgp,hdr,&vp)) == SR_NOTFOUND) {
	    RECIP	*rp ;
	    const int	hlen = strlen(hdr) ;
	    const int	mlen = MAXMSGLINELEN ;
	    const int	elen = MAXMSGLINELEN ;
	    int		i ;
	    char	ebuf[MAXMSGLINELEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgouthdr_deliveredto: not-found\n") ;
	    debugprintf("progarts/procmsgouthdr_deliveredto: rlp={%p}\n",rlp) ;
	}
#endif

	    for (i = 0 ; vecobj_get(rlp,i,&rp) >= 0 ; i += 1) {
	        const char	*cp ;
		if (rp == NULL) continue ;
		if ((rs = recip_get(rp,&cp)) > 0) {
		    int		cl = rs ;
		    const char	*np ;

#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progarts/procmsgouthdr_deliveredto: "
				"recip=%t\n",
				cp,cl) ;
#endif

		    if ((rs = prognamecache_lookup(pip,cp,&np)) > 0) {
		        int	nl = rs ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progarts/procmsgouthdr_deliveredto: "
			    "nl=%u n=%t\n",nl,np,nl) ;
#endif
		        if ((hlen+cl+nl+5) > mlen) {
			    nl = (mlen - (hlen+cl+5)) ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progarts/procmsgouthdr_deliveredto: "
			"shortening new nl=%u\n",nl) ;
#endif
			}
			rs1 = bufprintf(ebuf,elen,"%t (%t)",cp,cl,np,nl) ;
			    if (rs1 >= 0) {
				cl = rs1 ;
			        cp = ebuf ;
			    }
		    } else if (rs == SR_NOTFOUND)
			rs = SR_OK ;

#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progarts/procmsgouthdr_deliveredto: "
			"out rs=%d\n",rs) ;
#endif

		    if (rs >= 0) {
	                if ((rs = progprinthdr(pip,tfp,hdr,cp,cl)) >= 0) {
	                    pdp->tlen += rs ;
	                    if (pip->f.logmsg) {
		                const int	ml = MIN(cl,50) ;
	                        proglog_printf(pip,"  %s»%t",hdr,cp,ml) ;
		            }
			} /* end if (print-hdr) */
	            } /* end if */

		} /* end if (recip-get) */
#if	CF_DEBUG
		if (DEBUGLEVEL(4))
		    debugprintf("progarts/procmsgouthdr_deliveredto: "
			"out recip_get() rs=%d\n",rs) ;
#endif
		if (rs < 0) break ;
	    } /* end for (recipients) */

	} /* end if (not already have) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgouthdr_deliveredto: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_deliveredto) */
#endif /* COMMENT */


static int procmsgouteol(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsg: writing> EOH\n") ;
#endif

	rs = bputc(pdp->tfp,'\n') ;
	pdp->tlen += rs ;
	tlen += rs ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgouteol) */


static int procmsgoutbody(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	BFLINER		*blp = &pdp->bline ;
	MAILMSG		*msgp = pdp->msgp ;
	bfile		*ifp = pdp->ifp ;
	const int	llen = MSGLINELEN ;
	int		rs = SR_OK ;
	int		ll ;
	int		lenr ;
	int		clines = 0 ;
	int		tlen = 0 ;
	int		f_bol = TRUE ;
	int		f_eol = FALSE ;
	const char	*lp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgoutbody: copying body clen=%d\n",
	        aip->clen) ;
	    debugprintf("progarts/procmsgoutbody: in mailmsg_offset=%u\n",
	        pdp->offset) ;
	}
#endif /* CF_DEBUG */

/* copy the body of the input to the output */

	pdp->off_body = pdp->offset ;
	pdp->off_finish = pdp->offset ;
	if (aip->clen >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progarts/procmsgoutbody: writing w/ CLEN\n") ;
#endif

	    f_bol = TRUE ;
	    lenr = aip->clen ;
	    while ((rs >= 0) && (lenr > 0)) {
	        int	rl = MIN(lenr,llen) ;
	        rs = bfliner_readline(blp,rl,&lp) ;
	        ll = rs ;
	        pdp->f.eof = (ll == 0) ;
	        if (rs <= 0) break ;

	        pdp->offset += ll ;
	        f_eol = (lp[ll - 1] == '\n') ;

#if	CF_DEBUG && CF_DEBUGBODY
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progarts/procmsgoutbody: ll=%u body=>%t<\n",
	                ll,lp,strlinelen(lp,ll,45)) ;
	            debugprintf("progarts/procmsgoutbody: f_eol=%u\n",f_eol) ;
		}
#endif

#if	CF_CLENSTART
	        if (f_bol && FMAT(lp)) {
		    if ((rs = mailmsgmatenv(&pdp->me,lp,lp)) != 0) break ;
#endif

	        pdp->off_finish = pdp->offset ;
	        if (f_bol)
	            clines += 1 ;

	        rs = bwrite(pdp->tfp,lp,ll) ;
	        tlen += rs ;

	        lenr -= ll ;

	        ll = 0 ;
	        f_bol = f_eol ;
	        bfliner_readover(blp) ;

	        if (rs < 0) break ;
	    } /* end while (writing w/ CLEN) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("progarts/procmsgoutbody: f_eol=%u\n",f_eol) ;
#endif

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progarts/procmsgoutbody: writing w/o CLEN\n") ;
#endif

	    f_bol = TRUE ;
	    while (rs >= 0) {
	        rs = bfliner_readline(blp,llen,&lp) ;
	        ll = rs ;
	        pdp->f.eof = (ll == 0) ;

#if	CF_DEBUG && CF_DEBUGBODY
	        if (DEBUGLEVEL(4))
	            debugprintf("progarts/procmsgoutbody: _readline() rs=%d\n",
			rs) ;
#endif

	        if (rs <= 0) break ;

	        pdp->offset += ll ;
	        f_eol = (lp[ll - 1] == '\n') ;

#if	CF_DEBUG && CF_DEBUGBODY
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progarts/procmsgoutbody: ll=%u body=>%t<\n",
	                ll,lp,strlinelen(lp,ll,45)) ;
	            debugprintf("progarts/procmsgoutbody: f_eol=%u\n",f_eol) ;
		}
#endif

	        if (f_bol && FMAT(lp)) {
		    if ((rs = mailmsgmatenv(&pdp->me,lp,ll)) != 0) break ;
		}

	        pdp->off_finish = pdp->offset ;
	        if (f_bol)
	            clines += 1 ;

	        rs = bwrite(pdp->tfp,lp,ll) ;
	        tlen += rs ;

#if	CF_DEBUG && CF_DEBUGBODY
	        if (DEBUGLEVEL(4))
	            debugprintf("progarts/procmsgoutbody: bwrite() rs=%d\n",
			rs) ;
#endif

	        ll = 0 ;
	        f_bol = f_eol ;
	        bfliner_readover(blp) ;

	        if (rs < 0) break ;
	    } /* end while (writing w/o CLEN) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progarts/procmsgoutbody: while-out loop rs=%d\n",
			rs) ;
	  	debugprintf("progarts/procmsgoutbody: f_eol=%u\n",f_eol) ;
	    }
#endif

	} /* end if (content length or not) */

/* if no NL at the EOM, write one, but why?? -- for fun! */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	  	debugprintf("progarts/procmsgoutbody: rs=%d f_eol=%u\n",
			rs,f_eol) ;
#endif

	if ((rs >= 0) && (! f_eol)) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progarts/procmsgoutbody: extra EOL\n") ;
#endif
	    pdp->f.eom = TRUE ;
	    rs = bputc(pdp->tfp,'\n') ;
	    tlen += rs ;
	} /* end if */

/* update some state for this message */

	pdp->tlen += tlen ;
	pdp->clen = tlen ;
	pdp->clines = clines ;

/* done */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progarts/procmsgoutbody: ret rs=%d tlen=%u\n",
		rs,tlen) ;
	    debugprintf("progarts/procmsgoutbody: f_eof=%u\n",pdp->f.eof) ;
	}
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgoutbody) */


static int procmsgoutback(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;
	offset_t	coff ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgoutback: out mailmsg_offset=%u\n",
	        pdp->offset) ;
#endif /* CF_DEBUG */

/* write-back out the content-length */

	if (rs >= 0) {
	    int		clen = (pdp->off_finish - pdp->off_body) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progarts/procmsgoutback: writing post CLEN\n") ;
	        debugprintf("progarts/procmsgoutback: clen=%d:%d\n",
	            pdp->clen,clen) ;
	    }
#endif

	    if ((pdp->clen >= 0) && (pdp->clen < clen))
	        clen = pdp->clen ;

	    if (pip->f.logmsg)
	        proglog_printf(pip,"  clen=%u",clen) ;

	    if ((rs = btell(pdp->tfp,&coff)) >= 0) {
	        bseek(pdp->tfp,pdp->off_clen,SEEK_SET) ;
	        rs = bprintf(pdp->tfp,"%u",clen) ;
	        if (rs >= 0)
	            bseek(pdp->tfp,coff,SEEK_SET) ;
	    }

	    aip->clen = clen ;
	} /* end if (writing back a content length) */

/* write-back out the content-lines (if specified) */

	if ((rs >= 0) && pdp->f.plaintext && (aip->clines < 0)) {
	    int	clines = pdp->clines ;

	    if (pip->f.logmsg)
	        proglog_printf(pip,"  clines=%u",clines) ;

	    if ((rs = btell(pdp->tfp,&coff)) >= 0) {
	        bseek(pdp->tfp,pdp->off_clines,SEEK_SET) ;
	        rs = bprintf(pdp->tfp,"%u",clines) ;
	        if (rs >= 0)
	            bseek(pdp->tfp,coff,SEEK_SET) ;
	    }

	    aip->clines = clines ;
	} /* end if (content-lines) */

	return rs ;
}
/* end subroutine (procmsgoutback) */


static int procmsglog(pip,pdp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("procmsglog: ent\n") ;
#endif

	if (pip->open.logprog) {
	ARTICLE		*aip = pdp->aip ;
	MAILMSG		*msgp = pdp->msgp ;

#if	CF_LOGMLEN
	if (pip->f.logmsg)
	    proglog_printf(pip,"  mlen=%u",pdp->tlen) ;
#endif

	if (pip->f.logmsg && pdp->f.spam)
	    proglog_printf(pip,"  spam") ;

	} /* end if (logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("procmsglog: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsglog) */


static int procmsgct(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	const char	*hp ;

	pdp->f.ct_plaintext = TRUE ;
	if ((hl = mailmsg_hdrval(msgp,HN_CTYPE,&hp)) > 0) {
	MHCOM		c ;
	int		vl ;
	const char	*tp ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgct: hdr=>%t<\n",hp,hl) ;
#endif

	if ((rs = mhcom_start(&c,hp,hl)) >= 0) {
	    if ((vl = mhcom_getval(&c,&vp)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progarts/procmsgct: v=>%t<\n",vp,vl) ;
#endif

	        if ((tp = strnchr(vp,vl,';')) != NULL)
	            vl = (tp - vp) ;

	        rs1 = sisub(vp,vl,"text") ;

	        if ((rs1 >= 0) && (strnchr(vp,vl,'/') != NULL))
	            rs1 = sisub(vp,vl,"plain") ;

	        pdp->f.ct_plaintext = (rs1 >= 0) ;

	    } /* end if (non-zero) */
	    mhcom_finish(&c) ;
	} /* end if (mhcom) */

	} /* end if (mailmsg_hdrval) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgct: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgct) */


static int procmsgce(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	const char	*hp ;

	pdp->f.ce_text = TRUE ;
	if ((hl = mailmsg_hdrval(msgp,HN_CENCODING,&hp)) > 0) {
	COMPARSE	com ;
	int		hl, vl ;
	const char	*tp ;
	const char	*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgce: hdr=>%t<\n",hp,hl) ;
#endif

	if ((rs = comparse_start(&com,hp,hl)) >= 0) {
	    if ((vl = comparse_getval(&com,&vp)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progarts/procmsgce: v=>%t<\n",vp,vl) ;
#endif

	        if ((tp = strnchr(vp,vl,';')) != NULL)
	            vl = (tp - vp) ;

	        rs1 = sisub(vp,vl,"7bit") ;
	        if (rs1 < 0)
	            rs1 = sisub(vp,vl,"8bit") ;

	        pdp->f.ce_text = (rs1 >= 0) ;

	    } /* end if (getval) */
	    comparse_finish(&com) ;
	} /* end if (comparse) */

	} /* end if (mailmsg_hdrval) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progarts/procmsgce: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgce) */


static int procmsghdrval(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp,
		cchar *hname,int *valp)
{
	int		rs ;
	int		vl ;
	int		v = -1 ;
	const char	*vp ;

	if (hname == NULL) return SR_FAULT ;

	if (hname[0] == '\0') return SR_INVALID ;

	if ((rs = mailmsg_hdrval(msgp,hname,&vp)) >= 0) {
	    vl = rs ;

	    rs = hdrextnum(vp,vl) ;
	    v = rs ;

	} /* end if */

	if (valp != NULL)
	    *valp = (rs >= 0) ? v : -1 ;

	return rs ;
}
/* end subroutine (procmsghdrval) */


#ifdef	COMMENT
static int procmsgaddr(pip,at,hdrname,ap,al)
PROGINFO	*pip ;
int		at ;
const char	hdrname[] ;
const char	*ap ;
int		al ;
{
	EMA		a ;
	EMA_ENT		*ep ;
	const int	hlen = HDRNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		j ;
	int		hnl = -1 ;
	int		cl ;
	int		c = 0 ;
	int		f_route = FALSE ;
	const char	*sp ;
	const char	*cp ;
	const char	*hnp = hdrname ;
	char		hbuf[HDRNAMELEN + 1] ;

	if (ap == NULL) return SR_FAULT ;

	if (al < 0) al = strlen(ap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsgaddr: ent a=>%t<\n",
	        ap,strlinelen(ap,al,40)) ;
#endif

	if ((rs = ema_start(&a)) >= 0) {
	    if ((rs1 = ema_parse(&a,ap,al)) >= 0) {

	        for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	            f_route = TRUE ;
	            cp = ep->rp ;
	            if (cp == NULL) {
	                f_route = FALSE ;
	                cp = ep->ap ;
	            }

	            if (cp != NULL) {

	                if ((c == 0) && (addrbuf != NULL)) {
	                    strwcpy(addrbuf,cp,MAILADDRLEN) ;
			}

	                if (pip->f.logmsg) {

	                    if ((rs = mknewhdrname(hbuf,hlen,hdrname)) > 0) {
	                        hnp = hbuf ;
	                        hnl = rs ;
	                    } else
	                        hnl = strlen(hnp) ;

			    if (rs >= 0) {

	                    cl = strnlen(cp,(LOGFMTLEN -2 -hnl -1)) ;
	                    proglog_printf(pip,"  %s=%t",hnp,cp,cl) ;

	                    sp = ep->cp ; /* EMA-comment */
	                    if (sp != NULL) {

	                        if ((cl = sfsubstance(sp,ep->cl,&cp)) > 0) {
	                            cl = MIN(cl,(LOGFMTLEN -4 -2)) ;
	                            proglog_printf(pip,"    (%t)",cp,cl) ;
	                        }

	                    } /* end if */

	                    if (f_route && (ep->cp == NULL) &&
	                        (ep->ap != NULL)) {

	                        sp = ep->ap ; /* EMA-address */
	                        if ((cl = sfsubstance(sp,ep->al,&cp)) > 0) {
	                            cl = MIN(cl,(LOGFMTLEN -4 -2)) ;
	                            proglog_printf(pip,"    (%t)",cp,cl) ;
	                        }

	                    } /* end if */

			    } /* end if */

	                } /* end if (logging enabled) */

	                c += 1 ;
	                if ((! pip->f.logmsg) && (c > 0)) break ;
	            } /* end if (got an address) */

		    if (rs < 0) break ;
	        } /* end for */

	    } /* end if (parse) */
	    ema_finish(&a) ;
	} /* end if (ema) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsgaddr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgaddr) */
#endif /* COMMENT */


static int procspam(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		f_spam = FALSE ;

	if (pip->f.spam && (! pdp->f.spam)) {

/* check spam-flag */

#if	CF_SPAMFLAG

	if ((rs >= 0) && (! pdp->f.spam)) {

	    rs = procmailmsg_spamflag(pip,msgp) ;

	    if (rs > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }

	} /* end if (spam-flag check) */

#endif /* CF_SPAMFLAG */

/* check spam-status */

#if	CF_SPAMSTATUS

	if ((rs >= 0) && (! pdp->f.spam)) {

	    rs = procmailmsg_spamstatus(pip,msgp) ;

	    if (rs > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }

	} /* end if (spam-status check) */

#endif /* CF_SPAMSTATUS */

/* check bogosity */

#if	CF_BOGOSITY

	if ((rs >= 0) && (! pdp->f.spam)) {

	    rs = procmailmsg_bogosity(pip,msgp) ;

	    if (rs > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }

	} /* end if (bogosity check) */

#endif /* CF_BOGOSITY */

	} /* end if (wanted) */

	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procspam) */


static int procmsglogaddr(pip,hdrname,ap,al)
PROGINFO	*pip ;
const char	hdrname[] ;
const char	*ap ;
int		al ;
{
	EMA		a ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (ap == NULL) return SR_FAULT ;

	if (al < 0)
	    al = strlen(ap) ;

	if (pip->f.logmsg && (al > 0)) {
	    if ((rs = ema_start(&a)) >= 0) {
	        if (ema_parse(&a,ap,al) >= 0) {
		    rs = procmsglogema(pip,hdrname,&a) ;
		    c = rs ;
	        }
		ema_finish(&a) ;
	    } /* end if (ema) */
	} /* end if (logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsglogaddr: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsglogaddr) */


static int procmsglogema(pip,hdrname,emap)
PROGINFO	*pip ;
const char	hdrname[] ;
EMA		*emap ;
{
	EMA_ENT		*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		j ;
	int		cl ;
	int		hnl = -1 ;
	int		c = 0 ;
	int		f_route = FALSE ;
	const char	*sp ;
	const char	*cp ;
	const char	*hnp = hdrname ;

	if (pip->f.logmsg) {
	    const int	hlen = HDRNAMELEN ;
	    char	hbuf[HDRNAMELEN + 1] ;
	    for (j = 0 ; ema_get(emap,j,&ep) >= 0 ; j += 1) {

	            f_route = TRUE ;
	            cp = ep->rp ;
	            if (cp == NULL) {
	                f_route = FALSE ;
	                cp = ep->ap ;
	            }

		if (cp != NULL) {

	                if ((rs = mknewhdrname(hbuf,hlen,hdrname)) > 0) {
	                    hnp = hbuf ;
	                    hnl = rs ;
			} else
			    hnl = strlen(hdrname) ;

			if (rs >= 0) {
	                    cl = strnlen(cp,(LOGFMTLEN -2 -hnl -1)) ;
	                    proglog_printf(pip,"  %s=%t",hnp,cp,cl) ;

	                    sp = ep->cp ; /* EMA-comment */
	                    if (sp != NULL) {

	                        if ((cl = sfsubstance(sp,ep->cl,&cp)) > 0) {
	                            cl = MIN(cl,(LOGFMTLEN -4 -2)) ;
	                            proglog_printf(pip,"    (%t)",cp,cl) ;
	                        }

	                    } /* end if */

	                    if (f_route && (ep->cp == NULL) &&
	                        (ep->ap != NULL)) {

	                        sp = ep->ap ; /* EMA-address */
	                        if ((cl = sfsubstance(sp,ep->al,&cp)) > 0) {
	                            cl = MIN(cl,(LOGFMTLEN -4 -2)) ;
	                            proglog_printf(pip,"    (%t)",cp,cl) ;
	                        }

	                    } /* end if */

	                } /* end if */

	                c += 1 ;

	            } /* end if (got an address) */

		if (rs < 0) break ;
	    } /* end for */
	} /* end if (logging enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsglogema: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsglogema) */


static int procmsglog_date(pip,vp,vl)
PROGINFO	*pip ;
const char	*vp ;
int		vl ;
{
	DATER		*tdp = &pip->td ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rs2 ;

	if (pip->open.logprog || pip->open.logzone) {
	    if ((rs = dater_setmsg(tdp,vp,vl)) >= 0) {
		const int	dlen = DATEBUFLEN ;
		char		dbuf[DATEBUFLEN+1] ;
	            time_t	t ;

	            dater_gettime(tdp,&t) ;

	            dater_mkstrdig(tdp,dbuf,dlen) ;

	            if (pip->f.logmsg)
	                proglog_printf(pip,"  date=%s",dbuf) ;

	            if (pip->open.logzone) {
	                const int	zlen = DATER_ZNAMESIZE ;
	                int		zoff ;
	                char		zbuf[DATER_ZNAMESIZE + 1] ;

	                if ((rs1 = dater_getzonename(tdp,zbuf,zlen)) >= 0) {

	                	rs2 = dater_getzoneoff(tdp,&zoff) ;
	                    if (rs2 < 0)
	                        zoff = LOGZONES_NOZONEOFFSET ;

	                    rs = logzones_update(&pip->lz,zbuf,zlen,
	                        zoff,pip->stamp) ;

	                } /* end if */

	            } /* end if (logging time-zone information) */

	        } else if (isNotValid(rs)) {
	            rs = SR_OK ;
	            if (pip->f.logmsg)
	                proglog_printf(pip,"  bad_date=>%t<",vp,vl) ;
	        }
	} /* end if (logging enabled) */

	return rs ;
}
/* end subroutine (procmsglog_date) */


static int procmailmsg_spamflag(pip,msgp)
PROGINFO	*pip ;
MAILMSG		*msgp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	const char	*tp, *sp ;
	const char	*cp ;

#if	defined(HN_SPAMFLAG)
	rs1 = mailmsg_hdrval(msgp,HN_SPAMFLAG,&sp) ;
#else
	rs1 = mailmsg_hdrval(msgp,"x-spam-flag",&sp) ;
#endif
	sl = rs1 ;
	if (rs1 >= 0) {

	    if (sl < 0)
	        sl = strlen(sp) ;

	    f_spam = (sfsub(sp,sl,"YES",&cp) >= 0) ;

	    if (! f_spam) {

	        if ((tp = strnpbrk(sp,sl,",;")) != NULL)
	            sl = (tp - sp) ;

	        cl = sfshrink(sp,sl,&cp) ;

	        f_spam = (cl == 3) && (strncasecmp("yes",cp,cl) == 0) ;
	    }

	} /* end if */

	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procmailmsg_spamflag) */


static int procmailmsg_spamstatus(pip,msgp)
PROGINFO	*pip ;
MAILMSG		*msgp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	const char	*tp, *sp ;
	const char	*cp ;

#if	defined(HN_SPAMSTATUS)
	rs1 = mailmsg_hdrval(msgp,HN_SPAMSTAUS,&sp) ;
#else
	rs1 = mailmsg_hdrval(msgp,"x-spam-status",&sp) ;
#endif
	sl = rs1 ;
	if (rs1 >= 0) {

	    if (sl < 0)
	        sl = strlen(sp) ;

	    f_spam = (sfsub(sp,sl,"Yes",&cp) >= 0) ;

	    if (! f_spam) {

	        if ((tp = strnpbrk(sp,sl,",;")) != NULL)
	            sl = (tp - sp) ;

	        cl = sfshrink(sp,sl,&cp) ;

	        f_spam = (cl == 3) && (strncasecmp("yes",cp,cl) == 0) ;
	    }

	} /* end if */

	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procmailmsg_spamstatus) */


static int procmailmsg_bogosity(pip,msgp)
PROGINFO	*pip ;
MAILMSG		*msgp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	const char	*tp, *sp ;
	const char	*cp ;

#if	defined(HN_BOGOSITY)
	rs1 = mailmsg_hdrval(msgp,HN_BOGOSITY,&sp) ;
#else
	rs1 = mailmsg_hdrval(msgp,"x-bogosity",&sp) ;
#endif
	sl = rs1 ;
	if (rs1 >= 0) {

	    if (sl < 0)
	        sl = strlen(sp) ;

	    if ((tp = strnchr(sp,sl,',')) != NULL)
	        sl = (tp - sp) ;

	    f_spam = (sfsub(sp,sl,"Spam",&cp) >= 0) ;

	    if (! f_spam) {

	        cl = sfshrink(sp,sl,&cp) ;

	        f_spam = (cl == 4) && (strncasecmp("spam",cp,cl) == 0) ;
	    }

	} /* end if */

	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procmailmsg_bogosity) */


static int proclogenv(PROGINFO *pip,int ei,MAILMSG_ENVER *mep)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;
	const char	*fmt ;

	if (pip->open.logenv) {

	        cp = mep->a.ep ;
	        cl = mep->a.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progarts/procmsgenv: env a=>%t<\n",cp,cl) ;
#endif

		if (cp == NULL) {
		    cp = "*na*" ;
		    cl = -1 ;
		}

	        fmt = "%4u:%2u F %t" ;
	        logfile_printf(&pip->envsum,fmt,pip->nmsgs,ei,cp,cl) ;

	        cp = mep->d.ep ;
	        cl = mep->d.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progarts/procmsgenv: env d=>%t<\n",cp,cl) ;
#endif

	        fmt = "%4u:%2u D %t" ;
	        logfile_printf(&pip->envsum,fmt,pip->nmsgs,ei,cp,cl) ;

	        cp = mep->r.ep ;
	        cl = mep->r.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progarts/procmsgenv: env r=>%t<\n",cp,cl) ;
#endif

	        if ((cp != NULL) && (cp[0] != '\0')) {
		    fmt = "%4u:%2u R %t" ;
	            logfile_printf(&pip->envsum,fmt,pip->nmsgs,ei,cp,cl) ;
		}

	    } /* end if (special envelope logging) */

	return rs ;
}
/* end subroutine (proclogenv) */


#if	CF_SPAMSUBJECT

static int cheapspamcheck(pip,buf,buflen)
PROGINFO	*pip ;
const char	buf[] ;
int		buflen ;
{
	int		sl, cl ;
	const char	*sp, *cp ;
	const char	*tp ;

	sp = buf ;
	sl = (buflen < 0) ? strlen(buf) : buflen ;

	tp = strnchr(sp,sl,'[') ;

	if (tp == NULL)
	    return FALSE ;

	sl = (sp + sl) - (tp + 1) ;
	sp = (tp + 1) ;

	cl = sisub(sp,sl,"SPAM") ;
	cp = (sp+cl) ;

	if (cl < 0) {
	    cl = sisub(sp,sl,"Possible UCE") ;
	    cp = (sp+cl) ;
	}

	if (cl < 0)
	    return FALSE ;

	sl = (sp + sl) - (cp + 1) ;
	sp = cp + 1 ;

	tp = strnchr(sp,sl,']') ;

	return (tp == NULL) ? FALSE : TRUE ;
}
/* end subroutine (cheapspamcheck) */

#endif /* CF_SPAMSUBJECT */


#if	CF_MKARTFILE
static int mkarticlefile(pip,pdp,afname,tdname)
PROGINFO	*pip ;
PROCDATA	*pdp ;
char		afname[] ;
const char	tdname[] ;
{
	ARTICLE		*aip = pdp->aip ;
	const mode_t	om = 0664 ;
	const int	mi = pdp->mi ;
	int		rs ;
	const char	*nn = pip->nodename ;

	    if ((rs = mkartfile(afname,om,tdname,nn,mi)) >= 0) {
		int		al ;
		const char	*ap ;
		if ((al = sfbasename(afname,rs,&ap)) > 0) {
		    const int	st = articlestr_articleid ;
		    rs = article_addstr(aip,st,ap,al) ;
		}
		if (rs < 0) {
		    uc_unlink(afname) ;
		}
	    } /* end if (mkartfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progarts/mkarticlefile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkarticlefile) */
#else /* CF_MKARTFILE */
static int mkarticlefile(pip,pdp,afname,tdname)
PROGINFO	*pip ;
PROCDATA	*pdp ;
char		afname[] ;
const char	tdname[] ;
{
	ARTICLE		*aip = pdp->aip ;
	const mode_t	om = 0664 ;
	int		rs ;
	char		inname[MAXNAMELEN+1] ;
	char		tmpcname[MAXNAMELEN+1] ;
	char		*bp ;

	bp = tmpcname ;
	bp = strwcpy(bp,pip->nodename,7) ;
	bp = strwset(bp,'X',7) ;
	if ((rs = mkpath2(inname,tdname,tmpcname)) >= 0) {
	    if ((rs = mktmpfile(afname,om,inname)) >= 0) {
		int		al ;
		const char	*ap ;
		if ((al = sfbasename(afname,rs,&ap)) > 0) {
		    const int	st = articlestr_articleid ;
		    rs = article_addstr(aip,st,ap,al) ;
		}
		if (rs < 0) {
		    uc_unlink(afname) ;
		}
	    } /* end if (mktmpfile) */
	} /* end if (mkpath) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progarts/mkarticlefile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkarticlefile) */
#endif /* CF_MKARTFILE */


/* make (really clean-up) a header key-name */
static int mknewhdrname(hbuf,hlen,hdrname)
char		hbuf[] ;
int		hlen ;
const char	hdrname[] ;
{
	int		rs = SR_OK ;
	int		rlen = hlen ;
	int		j = 0 ;
	int		f = FALSE ;

	f = (strchr(hdrname,'-') != NULL) ;
	f = f || hasuc(hdrname,-1) ;

	if (f) {
	    int	i ;
	    for (i = 0 ; rlen && hdrname[i] ; i += 1) {
	        if (hdrname[i] != '-') {
	            hbuf[j++] = CHAR_TOLC(hdrname[i]) ;
	            rlen -= 1 ;
	        }
	    } /* end for */
	    hbuf[j] = '\0' ;
	    if ((rlen == 0) && (hdrname[i] != '\0'))
	        rs = SR_OVERFLOW ;
	} /* end if */

	return (rs >= 0) ? j : rs ;
}
/* end subroutine (mknewhdrname) */


static int vcmpheadname(e1pp,e2pp)
const char	**e1pp, **e2pp ;
{

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	return strcasecmp(*e1pp,*e2pp) ;
}
/* end subroutine (vcmpheadname) */


