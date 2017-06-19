/* progmsgs */

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


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module processes one or more mail messages (in appropriate mailbox
	format if more than one) on STDIN.  The output is a single file that is
	ready to be added to each individual mailbox in the spool area.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
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
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"received.h"
#include	"bfliner.h"
#include	"recip.h"
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

#define	LOGLINELEN	(80 - 16)

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

extern int	sncpy1(char *,int,cchar *) ;
extern int	snwcpyclean(char *,int,int,cchar *,int) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfsub(cchar *,int,cchar *,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sisub(cchar *,int,cchar *) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	mkbestaddr(char *,int,cchar *,int) ;
extern int	mailmsgmathdr(cchar *,int,int *) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	hasuc(cchar *,int) ;
extern int	isprintlatin(int) ;
extern int	hasEOH(cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

extern int	progmsgid(PROGINFO *,char *,int,int) ;
extern int	progprinthdrs(PROGINFO *,bfile *,MAILMSG *,cchar *) ;
extern int	progprinthdraddrs(PROGINFO *,bfile *,MAILMSG *,cchar *) ;
extern int	progprinthdremas(PROGINFO *,bfile *,cchar *,EMA *) ;
extern int	progprinthdr(PROGINFO *,bfile *,cchar *,cchar *,int) ;
extern int	prognamecache_lookup(PROGINFO *,cchar *,cchar **) ;

extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	mailmsg_loadline(MAILMSG *,cchar *,int) ;

extern int	sfsubstance(cchar *,int,cchar **) ;
extern int	mkdispaddr(char *,int,cchar *,int) ;
extern int	hdrextnum(cchar *,int) ;
extern int	hdrextid(char *,int,cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*strdcpyclean(char *,int,int,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
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
	MSGINFO		*mip ;
	bfile		*ifp ;
	bfile		*tfp ;
	vecobj		*tip ;
	vecobj		*rlp ;
	MAILMSG		*msgp ;
	BFLINER		bline ;
	VECSTR		wh ;
	PROCDATA_FL	f ;
	MAILMSGMATENV	me ;
	DATER		edate ;
	offset_t	offset ;
	offset_t	off_start, off_clen, off_body, off_finish ;
	offset_t	off_clines ;
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
static int	procspam(PROGINFO *,PROCDATA *) ;
static int	procmsglogaddr(PROGINFO *,char *,cchar *,cchar *,int) ;

static int	procmailmsg_spamsubj(PROGINFO *,cchar *,int) ;
static int	procmailmsg_spamflag(PROGINFO *,MAILMSG *) ;
static int	procmailmsg_spamstatus(PROGINFO *,MAILMSG *) ;
static int	procmailmsg_bogosity(PROGINFO *,MAILMSG *) ;

static int	procmsger(PROGINFO *,PROCDATA *) ;

static int	procmsgenv(PROGINFO *,PROCDATA *) ;
static int	procmsghdrs(PROGINFO *,PROCDATA *) ;
static int	procmsgout(PROGINFO *,PROCDATA *) ;
static int	procmsglog(PROGINFO *,PROCDATA *) ;

static int	procmsgenver(PROGINFO *,PROCDATA *,char *,int) ;

static int	procmsghdr_messageid(PROGINFO *,PROCDATA *) ;
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
static int	procmsgouthdr_articleid(PROGINFO *,PROCDATA *) ;
static int	procmsgouthdr_deliveredto(PROGINFO *,PROCDATA *) ;

static int	cmpheadname(cchar **,cchar **) ;

static int	mknewhdrname(char *,int,cchar *) ;


/* local variables */

static cchar	atypes[] = "LUIR" ;	/* address types */

/* should be at least "status-bytes" long (currently 10 bytes) */
static cchar	blanks[] = "                    " ;

static int (*msghdrgets[])(PROGINFO *,PROCDATA *) = {
	procmsghdr_messageid,
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
	procmsgouthdr_articleid,
	procmsgouthdr_deliveredto,
	NULL
} ;

/* mark the following headers as done since we "do" them at the end */
static cchar	*hdrspecials[] = {
	"status",
	"references",
	"from",
	"to",
	"cc",
	"bcc",
	"date",
	"subject",
	NULL
} ;


/* exported subroutines */


int progmsgs(PROGINFO *pip,bfile *ifp,bfile *tfp,vecobj *fip,vecobj *rlp)
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
	if (DEBUGLEVEL(3)) {
	    debugprintf("progmsgs: ent\n") ;
	    debugprintf("progmsgs: f_optnospam=%u f_spam=%u\n",
		pip->f.optnospam,pip->f.spam) ;
	}
#endif

	memset(pdp,0,sizeof(PROCDATA)) ;
	pdp->ifp = ifp ;
	pdp->tfp = tfp ;
	pdp->tip = fip ;
	pdp->rlp = rlp ;
	pdp->clines = -1 ;

	if ((rs = dater_startcopy(&pd.edate,&pip->tmpdate)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmsgs: 1\n") ;
#endif

	    blp = &pd.bline ;
	    if ((rs = bfliner_start(blp,ifp,0L,-1)) >= 0) {
	        int	ll = 0 ;
	        cchar	*lp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmsgs: 2\n") ;
#endif

/* find the start of a message */

	        while ((rs >= 0) && (! pdp->f.eof)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmsgs: 4\n") ;
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
	                debugprintf("mailmsgstage_gathermsg: line=>%t<\n",
	                    lp,strlinelen(lp,ll,40)) ;
#endif

	                f_eol = (lp[ll-1] == '\n') ;

	                pd.offset += ll ;
	                if (f_bol && FMAT(lp) && (ll > 5) &&
			    ((rs = mailmsgmatenv(&pd.me,lp,ll)) > 0)) {
	                    f_env = TRUE ;
	                } else if (f_bol && (ll > 2) &&
			    ((rs = mailmsgmathdr(lp,ll,&vi)) > 0)) {
	                    f_hdr = TRUE ;
	                } else if (f_bol && (ll <= 2) && (mi == 0)) {
			    if ((lp[0] == '\n') || hasEOH(lp,ll)) {
	                        f_eoh = TRUE ;
			    }
	                }

#if	CF_DEBUG
			if (DEBUGLEVEL(3)) {
	                debugprintf("mailmsgstage_gathermsg: det "
				"rs=%d f_env=%u f=hdr=%u f_eoh=%u\n",
				rs,f_env,f_hdr,f_eoh) ;
			}
#endif

			if (rs < 0) break ;
	                bfliner_getpoff(blp,&boff) ;
	                pd.off_start = boff ;

	                if (f_env || f_hdr) break ;

	                ll = 0 ;
	                bfliner_readover(blp) ;

	                f_bol = f_eol ;
	                if (f_eoh) break ;
	            } /* end while */

/* EOF already? */

	            if (rs < 0) break ;

	            if ((! f_eoh) && (! f_env) && (! f_hdr)) break ;

#ifdef	COMMENT
	            if (ll == 0) break ;
#endif

	            if (pip->f.logmsg)
	                proglog_printf(pip,"message %4u",
	                    pip->nmsgs) ;

	            rs = procmsg(pip,&pd,f_eoh) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progmsgs: progmsg() rs=%d\n",rs) ;
#endif

	            if (rs <= 0) break ;

	            pip->nmsgs += 1 ;
	            mi += 1 ;

	            pd.efrom[0] = '\0' ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progmsgs: f_eof=%u\n",pdp->f.eof) ;
#endif

	            if (pd.f.eof) break ;
	        } /* end while (processing article messages) */

	        bfliner_finish(blp) ;
	    } /* end if (bfliner) */

	    dater_finish(&pd.edate) ;
	} /* end if (dater) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmsgs: ret rs=%d mi=%u\n",rs,mi) ;
#endif

	return (rs >= 0) ? mi : rs ;
}
/* end subroutine (progmsgs) */


/* local subroutines */


/* process the current message */
static int procmsg(PROGINFO *pip,PROCDATA *pdp,int f_eoh)
{
	BFLINER		*blp = &pdp->bline ;
	MAILMSG		amsg, *msgp = &amsg ;
	const int	llen = MSGLINELEN ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsg: ent\n") ;
#endif

	pdp->tlen = 0 ;
	pdp->f.eom = FALSE ;
	pdp->f.spam = FALSE ;

	if ((rs = mailmsg_start(msgp)) >= 0) {
	    int		ll ;
	    cchar	*lp ;
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

	    if (rs >= 0) {
	        rs = procmsger(pip,pdp) ;
	        len = rs ;
	    }

	    rs1 = mailmsg_finish(msgp) ;
	    if (rs >= 0) rs = rs1 ;
	    pdp->msgp = NULL ;
	} /* end if (mailmsg) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsg: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmsg) */


static int procmsger(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		mi, *mip = &mi ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;

	pdp->mip = mip ;
	memset(mip,0,sizeof(MSGINFO)) ;
	mip->clen = -1 ;
	mip->clines = -1 ;

	if ((rs = dater_startcopy(&mip->edate,&pip->tmpdate)) >= 0) {

	    if ((rs = vecstr_start(&pdp->wh,10,0)) >= 0) {

	        rs = procmsgenv(pip,pdp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmsgs/procmsger: procmsgenv() rs=%d\n",
			rs) ;
#endif

	        if (rs >= 0) {
	            rs = procmsghdrs(pip,pdp) ;
		}

	        if (rs >= 0) {
	            rs = procmsgout(pip,pdp) ;
	            tlen += rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progmsgs/procmsger: "
			"procmsgout() rs=%d mlen=%u\n",
			rs,mi.mlen) ;
#endif

	        if (rs >= 0) {
	            rs = procmsglog(pip,pdp) ;
		}

	        if (rs >= 0) {
		    pdp->mi += 1 ;
	            rs = vecobj_add(pdp->tip,mip) ;
		}

	        rs1 = vecstr_finish(&pdp->wh) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */

	    rs1 = dater_finish(&mip->edate) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dater) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("progmsgs/procmsger: f_spam=%u\n",mip->f.spam) ;
	debugprintf("progmsgs/procmsger: ret rs=%d tlen=%u\n",rs,tlen) ;
	}
#endif

	pdp->mip = NULL ;
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsger) */


/* process the envelope information */
static int procmsgenv(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	const int	salen = STACKADDRLEN ;
	int		rs ;
	char		sabuf[STACKADDRLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgenv: ent\n") ;
#endif

	if ((rs = procmsgenver(pip,pdp,sabuf,salen)) >= 0) {
	    struct timeb	*nowp = &pip->now ;
	    const int		malen = MAILADDRLEN ;

	    if ((mip->e_from[0] == '\0') || (! pip->f.trusted)) {
	        const time_t	t = nowp->time ;
		cchar		*envfrom = pip->envfrom ;

	        if ((rs = sncpy1(pdp->efrom,malen,envfrom)) >= 0) {
	            const int	isdst = nowp->dstflag ;
	            const int	zoff = nowp->timezone ;
	            cchar	*zname = pip->zname ;
	            rs = dater_settimezon(&pdp->edate,t,zoff,zname,isdst) ;
		}

	    } else {

	        strwcpy(pdp->efrom,mip->e_from,malen) ;

	        rs = dater_setcopy(&pdp->edate,&mip->edate) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsgenv: dater_setcopy() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (pip->open.logprog && pip->f.logmsg) {
		int	cl ;
	        cchar	*cp = pdp->efrom ;
		cl = strnlen(cp,(LOGLINELEN - 4)) ;
	        proglog_printf(pip,"  F %t",cp,cl) ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsgenv: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgenv) */


/* process MAILMSG envelope information */
static int procmsgenver(PROGINFO *pip,PROCDATA *pdp,char *addrbuf,int addrlen)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	MAILMSG_ENVER	me, *mep = &me ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, sl ;
	int		cl ;
	int		ml ;
	int		al = 0 ;
	int		abl = addrlen, sabl ;
	int		sal = 0 ;
	int		f_remote ;
	cchar		*cp ;
	char		*ap = addrbuf ;
	char		*sap = addrbuf ;
	char		*addr = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgenver: ent\n") ;
#endif

	mip->e_from[0] = '\0' ;

	sal = 0 ;
	for (i = 0 ; mailmsg_enver(msgp,i,mep) >= 0 ; i += 1) {
	    EMAINFO	ai ;
	    const int	dlen = DATEBUFLEN ;
	    int		atype ;
	    int		froml = -1 ;
	    cchar	*fromp = NULL ;
	    char	dbuf[DATEBUFLEN + 1] ;

	    if (pip->open.logenv) {
	        cchar	*fmt ;

	        cp = mep->a.ep ;
	        cl = mep->a.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progmsgs/procmsgenver: env a=>%t<\n",cp,cl) ;
#endif

	        logfile_printf(&pip->envsum,"%4d:%2d F %t",
	            pip->nmsgs,i,
	            ((cp != NULL) ? cp : "*NA*"),cl) ;

	        cp = mep->d.ep ;
	        cl = mep->d.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progmsgs/procmsgenver: env d=>%t<\n",cp,cl) ;
#endif

	        fmt = "%4d:%2d D %t" ;
	        logfile_printf(&pip->envsum,fmt,pip->nmsgs,i,cp,cl) ;

	        cp = mep->r.ep ;
	        cl = mep->r.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progmsgs/procmsgenver: env r=>%t<\n",cp,cl) ;
#endif

	        if ((cp != NULL) && (cp[0] != '\0')) {
	            logfile_printf(&pip->envsum,"%4d:%2d R %t",
	                pip->nmsgs,i,
	                cp,cl) ;
		}

	    } /* end if (special envelope logging) */

	    fromp = mep->a.ep ;
	    froml = mep->a.el ;
	    if ((fromp == NULL) || (fromp[0] == '\0')) {
	        fromp = "mailer-daemon" ;
	        froml = -1 ;
	    }

	    rs1 = dater_setstd(&mip->edate,mep->d.ep,mep->d.el) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progmsgs/procmsgenver: dater_setstd() rs=%d\n",
	            rs) ;
#endif

	    dbuf[0] = '\0' ;
	    if (rs1 >= 0) {
	        rs = dater_mkstrdig(&mip->edate,dbuf,dlen) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progmsgs/procmsgenver: "
			"dater_mkstrdig() rs=%d\n",
	                rs) ;
#endif
	    }

	    if ((i > 0) && (abl < addrlen)) {
	        *ap++ = '!' ;
	        sal += 1 ;
	        abl -= 1 ;
	    }

	    addr = ap ;
	    al = 0 ;

	    sap = ap ;
	    sabl = abl ;

	    if (rs >= 0) {

	        f_remote = ((mep->r.ep != NULL) && (mep->r.ep[0] != '\0')) ;
	        if (f_remote) {

	            ml = MIN(mep->r.el,(abl-1)) ;
	            sl = strwcpy(ap,mep->r.ep,ml) - ap ;

	            ap += sl ;
	            al += sl ;
	            sal += sl ;
	            abl -= sl ;

	            sap = ap ;
	            sabl = abl ;

	            *ap++ = '!' ;
	            al += 1 ;
	            abl -= 1 ;

	        } /* end if (remote node name) */

	        atype = emainfo(&ai,fromp,froml) ;

	        sl = emainfo_mktype(&ai, EMAINFO_TUUCP, ap,abl) ;

	        if (sl > 0) {
	            ap += sl ;
	            al += sl ;
	            abl -= sl ;
	        }

	        if (pip->open.logprog && pip->f.logmsg) {
	            proglog_printf(pip,"  | %-25s", dbuf) ;
	            proglog_printf(pip,"  |   %c %t",
	                atypes[atype],addr,MIN(al,(LOGLINELEN - TABLEN))) ;
	        }

	        ap = sap ;
	        abl = sabl ;

	    } /* end if */

	    if (rs < 0) break ;
	} /* end for (looping through envelopes) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgenver: mid rs=%d i=%u\n",rs,i) ;
#endif

	if ((rs >= 0) && (i > 0)) {
	    time_t	t = 0 ;

	    sal += (sl + ((f_remote) ? 1 : 0)) ;

	    addrbuf[sal] = '\0' ;
	    strwcpy(mip->e_from,addrbuf,MAILADDRLEN) ;

	    if (pip->open.logprog && pip->f.logmsg) {
	        proglog_printf(pip,"  > %t",
	            addrbuf,MIN(sal,(LOGLINELEN - 4))) ;
	    }

	    if (dater_gettime(&mip->edate,&t) >= 0) mip->etime = t ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progmsgs/procmsgenver: ret rs=%d\n",rs) ;
	    debugprintf("progmsgs/procmsgenver: ret i=%u sal=%u\n",i,sal) ;
	}
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (procmsgenver) */


static int procmsghdrs(PROGINFO *pip,PROCDATA *pdp)
{
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
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl ;
	int		sl = 0 ;
	int		f_messageid = FALSE ;
	cchar		*hdr = HN_MESSAGEID ;
	cchar		*vp ;
	cchar		*sp = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_mid: ent\n") ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_MESSAGEID)) >= 0) {
		const int	mlen = MAILADDRLEN ;
		char		*mbuf = mip->h_messageid ;
	        if ((rs = hdrextid(mbuf,mlen,vp,vl)) == 0) {
	            sp = mip->h_messageid ;
	            sl = rs ;
	        } else if (isNotValid(rs)) {
	            rs = SR_OK ;
		}
	    } /* end if (do not copy header) */
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_mid: mid=%s\n",
	        mip->h_messageid) ;
#endif

	if (rs >= 0) {
	    f_messageid = TRUE ;
	    if (mip->h_messageid[0] == '\0') {
		const int	mlen = MAILADDRLEN ;
		char		*mbuf = mip->h_messageid ;
	        f_messageid = FALSE ;
	        sp = mip->h_messageid ;
	        rs = progmsgid(pip,mbuf,mlen,pdp->mi) ;
	        sl = rs ;
	    } /* end if (needed) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_mid: mid=%s\n",
	        mip->h_messageid) ;
#endif /* CF_DEBUG */

/* log the MID if logging enabled */

	if ((rs >= 0) && pip->f.logmsg) {
	    int	f_first = TRUE ;
	    cchar	*fmt ;
	    cchar	*cp ;
	    int		cl ;

#ifdef	OPTIONAL
	    sp = mip->h_messageid ;
	    sl = strlen(sp) ;
#endif

	    while (sl > 0) {
	        cp = sp ;
	        cl = MIN(sl,(LOGLINELEN - TABLEN)) ;
	        if (f_first) {
	            f_first = FALSE ;
	            fmt = "  mid=| %t" ;
	        } else {
	            fmt = "      | %t" ;
		}
	        proglog_printf(pip,fmt,cp,cl) ;
	        sp += cl ;
	        sl -= cl ;
	    } /* end while */

	} /* end if (logging MID) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_mid: ret rs=%d f_mid=%u\n",
		rs,f_messageid) ;
#endif /* CF_DEBUG */

	mip->f.messageid = f_messageid ;
	return rs ;
}
/* end subroutine (procmsghdr_messageid) */


/* mailmsg content-length */
static int procmsghdr_clen(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_CLEN ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    if ((rs = vecstr_add(hlp,hdr,HL_CLEN)) >= 0) {

	        if ((rs = hdrextnum(vp,vl)) >= 0) {
	            mip->clen = rs ;
	        } else if (isNotValid(rs))
	            rs = SR_OK ;

	    }
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progmsgs/procmsg: clen=%d\n",mip->clen) ;
#endif

	return rs ;
}
/* end subroutine (procmsghdr_clen) */


/* mailmsg content-lines */
static int procmsghdr_clines(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
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

	mip->clines = v ;

	return rs ;
}
/* end subroutine (procmsghdr_clines) */


static int procmsghdr_xmailer(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs = SR_OK ;
	int		vl ;
	cchar		*hdr = HN_XMAILER ;
	cchar		*vp ;

#if	CF_XMAILER
	if (pip->f.logmsg) {
	    int	n ;
	    int	i ;

	    n = mailmsg_hdrcount(msgp,hdr) ;
	    if (n < 0)
	        n = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progmsgs/procmsg: xmailer n=%d\n",n) ;
#endif

	    for (i = 0 ; (vl = mailmsg_hdrival(msgp,hdr,i,&vp)) >= 0 ; i += 1) {
	        cchar	*fmt ;

	        proglog_printf(pip,"  xmailer %u",(n - i - 1)) ;

	        fmt = (strchr(vp,' ') != NULL) ? "    >%t<" : "    %t" ;
	        proglog_printf(pip,fmt,vp,vl) ;

	    } /* end for */

	} /* end if (mailer) */
#endif /* CF_XMAILER */

	return rs ;
}
/* end subroutine (procmsghdr_xmailer) */


static int procmsghdr_received(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	const int	salen = STACKADDRLEN ;
	const int	dlen = DATEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl ;
	cchar		*hdr = HN_RECEIVED ;
	cchar		*vp ;
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
	        debugprintf("progmsgs/procmsg: received n=%d\n",n) ;
#endif

	    for (i = (n - 1) ; i >= 0 ; i -= 1) {
	        RECEIVED	rh ;

	        vl = mailmsg_hdrival(msgp,hdr,i,&vp) ;
	        if (vl < 0) break ;

	        if (vl == 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progmsgs/procmsg: i=%u r=>%t<\n",
	                i,vp,strlinelen(vp,vl,40)) ;
#endif

	        if ((rs = received_start(&rh,vp,vl)) >= 0) {
	            int		rl ;
	            int		j ;
	            cchar	*fmt ;
	            cchar	*sp ;
	            cchar	*rp ;

	            ri = (n - 1) - i ;
	            proglog_printf(pip,"  received %u",ri) ;

	            for (j = 0 ; (rl = received_getkey(&rh,j,&rp)) >= 0 ; 
	                j += 1) {

	                if (rp == NULL) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("progmsgs/procmsg: j=%u r=>%t<\n",
	                        j,rp,strlinelen(rp,rl,40)) ;
#endif

	                sp = rp ;
	                if (j == received_keydate) {
			    rs1 = dater_setmsg(&pip->tmpdate,rp,rl) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("progmsgs/procmsg: "
					"dater_setmsg() rs=%d\n", rs1) ;
#endif

			    if (rs1 >= 0) {
			        DATER	*dp = &pip->tmpdate ;
				sp = dbuf ;
	                        rs = dater_mkstrdig(dp,dbuf,dlen) ;
			    } else {
				cchar *fmt = "** bad date-spec (%d) **" ;
				sp = dbuf ;
				bufprintf(dbuf,dlen,fmt,rs1) ;
			    }

	                } else if (j == received_keyfor) {

			    sp = sabuf ;
	                    rs = mkbestaddr(sabuf,salen,rp,rl) ;

	                } /* end if (special handling cases) */

			if (sp != NULL) {
	                    fmt = "    %s=%s" ;
	                    if (strchr(sp,' ') != NULL) {
	                        fmt = "    %s=>%s<" ;
			    }
	                    proglog_printf(pip,fmt,received_keys[j],sp) ;
			} /* end if (something to log) */

	                if (rs < 0) break ;
	            } /* end for */

	            rs1 = received_finish(&rh) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (initialized) */

	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progmsgs/procmsg: received-done\n") ;
#endif

	} /* end if */
#endif /* CF_RECEIVED */

	return rs ;
}
/* end subroutine (procmsghdr_received) */


static int procmsghdr_replyto(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_REPLYTO ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsg: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("progmsgs/procmsg: mailmsg_hdrval() rs=%d\n",rs) ;
	        debugprintf("progmsgs/procmsg: REPLYTO v=>%t<\n",
	            vp,strlinelen(vp,vl,40)) ;
	    }
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progmsgs/procmsg: procmsglogaddr() \n") ;
#endif

	    rs = procmsglogaddr(pip,mip->h_replyto,hdr,vp,vl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progmsgs/procmsg: procmsglogaddr() rs=%d\n",rs) ;
#endif

	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (procmsghdr_replyto) */


static int procmsghdr_errorsto(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_ERRORSTO ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_errorsto: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_errorsto: v=%t\n",vp,vl) ;
#endif
	    rs = procmsglogaddr(pip,mip->h_errorsto,hdr,vp,vl) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsghdr_errorsto: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsghdr_errorsto) */


static int procmsghdr_sender(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_SENDER ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsg: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    rs = procmsglogaddr(pip,mip->h_sender,hdr,vp,vl) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (procmsghdr_sender) */


static int procmsghdr_deliveredto(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_DELIVEREDTO ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    rs = procmsglogaddr(pip,mip->h_deliveredto,hdr,vp,vl) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (procmsghdr_deliveredto) */


static int procmsghdr_xoriginalto(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_XORIGINALTO ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    rs = procmsglogaddr(pip,mip->h_xoriginalto,hdr,vp,vl) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (procmsghdr_xoriginal) */


static int procmsghdr_xpriority(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: hdr=%s\n",hdr) ;
#endif
	if (pip->f.logmsg) {
	    MAILMSG	*msgp = pdp->msgp ;
	    int		hl ;
	    cchar	*hdr = HN_XPRIORITY ;
	    cchar	*hp ;
	    if ((rs = mailmsg_hdrval(msgp,hdr,&hp)) >= 0) {
	        COMPARSE	com ;
	        hl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: h=%t\n",hp,hl) ;
#endif
	        if ((rs = comparse_start(&com,hp,hl)) >= 0) {
		    cchar	*vp ;
	            if ((rs = comparse_getval(&com,&vp)) > 0) {
		        cchar	*cp ;
		        int	vl = rs ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: v=%t\n",vp,vl) ;
#endif
	                if ((rs = comparse_getcom(&com,&cp)) >= 0) {
		            const int	plen = 20 ;
		            const int	hlen = HDRNAMELEN ;
		            int		cl = rs ;
			    int		hnl ;
		            cchar	*hnp = hdr ;
		            char	hbuf[HDRNAMELEN + 1] ;

		            if ((rs = mknewhdrname(hbuf,hlen,hdr)) > 0) {
	                            hnp = hbuf ;
	                            hnl = rs ;
		            } else
	                            hnl = strlen(hnp) ;

		            if (rs >= 0) {
			        cchar	*fmt = "  %t=%t (%t)" ;
			        if (vl > plen) vl = plen ;
			        if (cl > plen) cl = plen ;
			        if (cl == 0) fmt = "  %t=%t" ;
			        proglog_printf(pip,fmt,hnp,hnl,vp,vl,cp,cl) ;
		            }

		        } /* end if (comparse-get) */
		    } /* end if (comparse-get) */
		    comparse_finish(&com) ;
	        } /* end if (comparse) */
	    } else if (rs == SR_NOTFOUND) {
	        rs = SR_OK ;
	    }
	} /* end if (msg-logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmsghdr_xpriority: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsghdr_xpriority) */


/* start writing the output file */
static int procmsgout(PROGINFO *pip,PROCDATA *pdp)
{
	offset_t	moff ;
	int		rs ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgout: ent\n") ;
#endif

	if ((rs = btell(pdp->tfp,&moff)) >= 0) {
	    MSGINFO	*mip = pdp->mip ;
	    mip->moff = (moff & INT_MAX) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgout: moff=%llu\n",moff) ;
#endif

	    if (rs >= 0) {
	        rs = procmsgoutenv(pip,pdp) ;
	        tlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = procmsgouthdrs(pip,pdp) ;
	        tlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = procmsgouteol(pip,pdp) ;
	        tlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = procmsgoutbody(pip,pdp) ;
	        tlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = procmsgoutback(pip,pdp) ;
	    }

	    mip->mlen = tlen ;
	} /* end if (btell) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progmsgs/procmsgout: f_spam=%u\n",pdp->f.spam) ;
	    debugprintf("progmsgs/procmsgout: ret rs=%d mlen=%u\n",rs,tlen) ;
	}
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgout) */


/* write out our own (new) envelope */
static int procmsgoutenv(PROGINFO *pip,PROCDATA *pdp)
{
	const int	dlen = DATEBUFLEN ;
	int		rs ;
	int		tlen = 0 ;
	char		dbuf[DATEBUFLEN+1] ;

	if ((rs = dater_mkstd(&pdp->edate,dbuf,dlen)) >= 0) {
	    cchar	*fmt = "From %s %s\n" ;
	    rs = bprintf(pdp->tfp,fmt,pdp->efrom,dbuf) ;
	    pdp->tlen += rs ;
	    tlen += rs ;
	}

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgoutenv) */


static int procmsgouthdrs(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs = SR_OK ;
	int		i ;
	int		slen = pdp->tlen ;
	int		tlen = 0 ;

	for (i = 0 ; msgouthdrs[i] != NULL ; i += 1) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progmsgs/procmsgouthdrs: i=%u\n",i) ;
#endif
	    rs = (*msgouthdrs[i])(pip,pdp) ;
	    if (rs < 0) break ;
	} /* end for */

	tlen = (pdp->tlen - slen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progmsgs/procmsgouthdrs: mid rs=%d tlen=%u\n",
	        rs,tlen) ;
	    debugprintf("progmsgs/procmsgouthdrs: f_spam=%u\n",pdp->f.spam) ;
	}
#endif

/* check spam */

	if ((rs >= 0) && pip->f.spam && (! pdp->f.spam)) {
	    rs = procspam(pip,pdp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progmsgs/procmsgouthdrs: f_spam=%u\n",pdp->f.spam) ;
	    debugprintf("progmsgs/procmsgouthdrs: ret rs=%d tlen=%u\n",
	        rs,tlen) ;
	}
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgouthdrs) */


static int procmsgouthdr_returnpath(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	int		vl  = 0 ;
	cchar		*hdr = HN_RETURNPATH ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsg: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) >= 0) {
	    vl = rs ;
	    rs = vecstr_add(hlp,hdr,-1) ;
	}

	if ((rs >= 0) && (vl > 0)) {

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,mip->h_returnpath,hdr,vp,vl) ;

	} else if (((rs >= 0) && (vl == 0)) || (rs == SR_NOTFOUND)) {

	    rs = progprinthdr(pip,pdp->tfp,hdr,pdp->efrom,-1) ;
	    pdp->tlen += rs ;

	} /* end if */

	return rs ;
}
/* end subroutine (procmsgouthdr_returnpath) */


static int procmsgouthdr_received(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs ;
	cchar		*hdr = HN_RECEIVED ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgouthdr_received: hdr=%s\n",hdr) ;
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
	    debugprintf("progmsgs/procmsgouthdr_received: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_received) */


static int procmsgouthdr_clen(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs = SR_OK ;
	cchar		*hdr = HN_CLEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgouthdr_clen: hdr=%s\n",hdr) ;
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
	    debugprintf("progmsgs/procmsgouthdr_clen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_clen) */


static int procmsgouthdr_clines(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	VECSTR		*hlp = &pdp->wh ;
	int		rs = SR_OK ;
	cchar		*hdr = HN_CLINES ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progmsgs/procmsgouthdr_clines: hdr=%s\n",hdr) ;
#endif

	if (pdp->f.plaintext && (mip->clines < 0)) {

	    if (rs >= 0) {
	        rs = bprintf(pdp->tfp,"%s: ",hdr) ;
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
	    debugprintf("progmsgs/procmsgouthdr_clines: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_clines) */


static int procmsgouthdr_messageid(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	int		rs = SR_OK ;

	if (mip->h_messageid != NULL) {
	    cchar	*sp = mip->h_messageid ;
	    bfile	*tfp = pdp->tfp ;
	    const int	sl = strlen(mip->h_messageid) ;
	    int		mlen ;
	    cchar	*hdr = HN_MESSAGEID ;
	    char	*mbuf ;
	    mlen = (sl+2) ;
	    if ((rs = uc_malloc((mlen+1),&mbuf)) >= 0) {
		char	*bp = mbuf ;
		*bp++ = CH_LANGLE ;
		bp = strwcpy(bp,sp,sl) ;
		*bp++ = CH_RANGLE ;
		*bp = '\0' ;
	        rs = progprinthdr(pip,tfp,hdr,mbuf,(bp-mbuf)) ;
	        pdp->tlen += rs ;
		uc_free(mbuf) ;
	    } /* end if (m-a-f) */
	} /* end if (present) */

	return rs ;
}
/* end subroutine (procmsgouthdr_messageid) */


/* put out all remaining headers */
static int procmsgouthdr_remaining(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	VECSTR		*hlp = &pdp->wh ;
	const int	hdrlen = HDRNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		kl ;
	cchar		*kp ;
	char		hdrname[HDRNAMELEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_remaining: ent\n") ;
#endif

	vecstr_sort(hlp,cmpheadname) ;

	for (i = 0 ; (kl = mailmsg_hdrikey(msgp,i,&kp)) >= 0 ; i += 1) {
	    if (kp == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progmsgs/procmsgouthdr_remaining: hdr=%t\n",
	            kp,kl) ;
#endif

	    strwcpy(hdrname,kp,MIN(kl,hdrlen)) ;
	    rs1 = vecstr_search(hlp,hdrname,cmpheadname,NULL) ;

	    if (rs1 == SR_NOTFOUND) {
	        if (matcasestr(hdrspecials,hdrname,-1) >= 0) rs1 = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progmsgs/procmsgouthdr_remaining: have rs1=%d\n",
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
	    debugprintf("progmsgs/procmsgouthdr_remaining: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_remaining) */


static int procmsgouthdr_status(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	const int	slen = 10 ; /* status bytes available */
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_STATUS ;
	cchar		*vp ;
	char		sbuf[10+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_status: hdr=%s\n",hdr) ;
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
	    debugprintf("progmsgs/procmsgouthdr_status: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_status) */


static int procmsgouthdr_references(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	cchar		*hdr = HN_REFERENCES ;

	if ((rs = mailmsg_hdrcount(msgp,hdr)) > 0) {
	    rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_references) */


static int procmsgouthdr_from(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_FROM ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0) {
	        rs = procmsglogaddr(pip,mip->h_from,hdr,vp,vl) ;
	    }

	} else if ((rs == 0) || (rs == SR_NOTFOUND)) {

	    rs = SR_OK ;
	    vp = pip->envfrom ;
	    strwcpy(mip->h_from,vp,MAILADDRLEN) ;

	} /* end if (from) */

	return rs ;
}
/* end subroutine (procmsgouthdr_from) */


static int procmsgouthdr_to(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_TO ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,mip->h_to,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_to) */


static int procmsgouthdr_cc(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_CC ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,NULL,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_cc) */


static int procmsgouthdr_bcc(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_BCC ;
	cchar		*vp ;

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    vl = rs ;

	    rs = progprinthdraddrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;
	    if (rs >= 0)
	        rs = procmsglogaddr(pip,NULL,hdr,vp,vl) ;

	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (procmsgouthdr_bcc) */


static int procmsgouthdr_date(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		rs1 ;
	int		rs2 ;
	cchar		*hdr = HN_DATE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_date: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrcount(msgp,hdr)) > 0) {
	    int		vl ;
	    cchar	*vp ;
	    if ((rs = progprinthdrs(pip,pdp->tfp,msgp,hdr)) >= 0) {
	        pdp->tlen += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progmsgs/procmsgouthdr_date: "
	                "progprinthdrs() rs=%d\n",rs) ;
#endif

	        if ((rs = mailmsg_hdrival(msgp,hdr,0,&vp)) >= 0) {
		    DATER	*tdp = &pip->tmpdate ;
	            vl = rs ;

	            if ((rs = dater_setmsg(&pip->tmpdate,vp,vl)) >= 0) {
		        const int	dlen = DATEBUFLEN ;
	                int		zoff ;
		        char		dbuf[DATEBUFLEN+1] ;
	                time_t		t ;

	                rs2 = dater_getzoneoff(tdp,&zoff) ;

	                dater_gettime(tdp,&t) ;
	                mip->mtime = t ;

	                dater_mkstrdig(tdp,dbuf,dlen) ;

	                if (pip->f.logmsg) {
			    const int	czoff = pip->now.timezone ;
			    cchar	*fmt = "  date=%s" ;
			    char	tbuf[TIMEBUFLEN+1] = { 0 } ;
#if	CF_DEBUG
			    if (DEBUGLEVEL(4))
	    			debugprintf("progmsgs/procmsgouthdr_date: "
					"czoff=%d zoff=%d\n",czoff,zoff) ;
#endif
			    if (czoff != zoff) {
			        fmt = "  date=%s (%s)" ;
			        timestr_logz(t,tbuf) ;
			    }
	                    proglog_printf(pip,fmt,dbuf,tbuf) ;
		        }

	                if (pip->open.logzone) {
	                    const int	zlen = DATER_ZNAMESIZE ;
	                    char	zbuf[DATER_ZNAMESIZE + 1] ;
    
	                    if ((rs1 = dater_getzonename(tdp,zbuf,zlen)) >= 0) {
			        LOGZONES	*lzp = &pip->lz ;
	                        if (rs2 < 0) zoff = LOGZONES_NOZONEOFFSET ;
	                        logzones_update(lzp,zbuf,zlen,zoff,pip->stamp) ;
	                    } /* end if */

	                } /* end if (logging time-zone information) */

	            } else if (isNotValid(rs)) {
	                rs = SR_OK ;
	                if (pip->f.logmsg) {
	                    proglog_printf(pip,"  bad_date=>%t<",vp,vl) ;
		        }
	            }

	        } /* end if (first date header retrieved) */
	    } /* end if (progprinthdrs) */
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_status: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_date) */


static int procmsgouthdr_subject(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_SUBJECT ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_subject: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    cchar	*cp ;
	    int		cl ;
	    vl = rs ;

	    rs = progprinthdrs(pip,pdp->tfp,msgp,hdr) ;
	    pdp->tlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progmsgs/procmsgouthdr_subject: "
	            "progprinthdrs() rs=%d\n",
	            rs) ;
#endif

	    if (rs >= 0) {

/* store the subject away as message information (MI) */

	        if ((cl = sfshrink(vp,vl,&cp)) >= 0) {

	        strdcpyclean(mip->h_subject,MAILADDRLEN,'¿',cp,cl) ;

	        if (pip->f.logmsg) {
	            proglog_printf(pip,"  subject=>%t<",
	                mip->h_subject,MIN(cl,50)) ;
		}

/* check for SPAM */

#if	CF_SPAMSUBJECT
	        if ((rs >= 0) && (! pdp->f.spam) && pip->f.spam) {
	            rs = procmailmsg_spamsubj(pip,cp,cl) ;
	            mip->f.spam = (rs > 0) ;
	            pdp->f.spam = (rs > 0) ;
	        } /* end if (spam check) */
#endif /* CF_SPAMSUBJECT */

		} /* end if */

	    } /* end if */

	} else if ((rs == 0) || (rs == SR_NOTFOUND)) {
	    cchar	*sp = pip->msgsubject ;
	    rs = SR_OK ;
	    if ((sp != NULL) && (sp[0] != '\0')) {

	        rs = progprinthdr(pip,pdp->tfp,hdr,pip->msgsubject,-1) ;
	        pdp->tlen += rs ;

	    } /* end if (SUBJECT) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_subject: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_subject) */


static int procmsgouthdr_articleid(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	MAILMSG		*msgp = pdp->msgp ;
	int		rs ;
	int		vl ;
	cchar		*hdr = HN_ARTICLEID ;
	cchar		*vp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_articleid: hdr=%s\n",hdr) ;
#endif

	if ((rs = mailmsg_hdrval(msgp,hdr,&vp)) > 0) {
	    cchar	*cp ;
	    int		cl ;
	    vl = rs ;

	    if ((cl = sfshrink(vp,vl,&cp)) >= 0) {
		int	ml = MIN(cl,MAXNAMELEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progmsgs/procmsgouthdr_articleid: a=%t\n",
			cp,ml) ;
#endif

	        strwcpy(mip->h_articleid,cp,ml) ;

	        if (pip->f.logmsg) {
		    int	ll = MIN(ml,50) ;
	            proglog_printf(pip,"  articleid=%t",cp,ll) ;
		}

	    } /* end if */

	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_articleid: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_articleid) */


static int procmsgouthdr_deliveredto(PROGINFO *pip,PROCDATA *pdp)
{
	MAILMSG		*msgp = pdp->msgp ;
	bfile		*tfp = pdp->tfp ;
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*hdr = HN_DELIVEREDTO ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_deliveredto: ent\n") ;
#endif

	if (mailmsg_hdrval(msgp,hdr,NULL) == SR_NOTFOUND) {
	    VECOBJ	*rlp = pdp->rlp ;
	    RECIP	*rp ;
	    const int	hlen = strlen(hdr) ;
	    const int	mlen = MAXMSGLINELEN ;
	    const int	elen = MAXMSGLINELEN ;
	    int		i ;
	    char	ebuf[MAXMSGLINELEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progmsgs/procmsgouthdr_deliveredto: not-found\n") ;
	    debugprintf("progmsgs/procmsgouthdr_deliveredto: rlp={%p}\n",rlp) ;
	}
#endif

	    for (i = 0 ; vecobj_get(rlp,i,&rp) >= 0 ; i += 1) {
	        cchar	*cp ;
		if (rp == NULL) continue ;
		if ((rs = recip_get(rp,&cp)) > 0) {
		    int		cl = rs ;
		    cchar	*np ;

#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progmsgs/procmsgouthdr_deliveredto: "
				"recip=%t\n",
				cp,cl) ;
#endif

		    if ((rs = prognamecache_lookup(pip,cp,&np)) > 0) {
		        int	nl = rs ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progmsgs/procmsgouthdr_deliveredto: "
			    "nl=%u n=%t\n",nl,np,nl) ;
#endif
		        if ((hlen+cl+nl+5) > mlen) {
			    nl = (mlen - (hlen+cl+5)) ;
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	 		debugprintf("progmsgs/procmsgouthdr_deliveredto: "
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
	 		debugprintf("progmsgs/procmsgouthdr_deliveredto: "
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
		    debugprintf("progmsgs/procmsgouthdr_deliveredto: "
			"out recip_get() rs=%d\n",rs) ;
#endif
		if (rs < 0) break ;
	    } /* end for (recipients) */

	} /* end if (not already have) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgouthdr_deliveredto: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgouthdr_deliveredto) */


static int procmsgouteol(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs ;
	int		tlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsg: writing> EOH\n") ;
#endif

	rs = bputc(pdp->tfp,'\n') ;
	pdp->tlen += rs ;
	tlen += rs ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgouteol) */


static int procmsgoutbody(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	BFLINER		*blp = &pdp->bline ;
	const int	llen = MSGLINELEN ;
	int		rs = SR_OK ;
	int		ll ;
	int		lenr ;
	int		clines = 0 ;
	int		tlen = 0 ;
	int		f_bol = TRUE ;
	int		f_eol = FALSE ;
	cchar		*lp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progmsgs/procmsgoutbody: ent clen=%d\n",
	        mip->clen) ;
	    debugprintf("progmsgs/procmsgoutbody: in mailmsg_offset=%u\n",
	        pdp->offset) ;
	}
#endif /* CF_DEBUG */

/* copy the body of the input to the output */

	pdp->off_body = pdp->offset ;
	pdp->off_finish = pdp->offset ;
	if (mip->clen >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progmsgs/procmsgoutbody: writing w/ CLEN\n") ;
#endif

	    f_bol = TRUE ;
	    lenr = mip->clen ;
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
	            debugprintf("progmsgs/procmsgoutbody: ll=%u body=>%t<\n",
	                ll,lp,strlinelen(lp,ll,45)) ;
	            debugprintf("progmsgs/procmsgoutbody: f_eol=%u\n",f_eol) ;
		}
#endif

#if	CF_CLENSTART
	        if (f_bol && FMAT(lp) && (ll > 5)) {
		    if ((rs = mailmsgmatenv(&pdp->me,lp,lp)) != 0) break ;
	        }
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
		debugprintf("progmsgs/procmsgoutbody: f_eol=%u\n",f_eol) ;
#endif

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progmsgs/procmsgoutbody: writing w/o CLEN\n") ;
#endif

	    f_bol = TRUE ;
	    while (rs >= 0) {
	        rs = bfliner_readline(blp,llen,&lp) ;
	        ll = rs ;
	        pdp->f.eof = (ll == 0) ;
	        if (rs <= 0) break ;

	        pdp->offset += ll ;
	        f_eol = (lp[ll - 1] == '\n') ;

#if	CF_DEBUG && CF_DEBUGBODY
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progmsgs/procmsgoutbody: ll=%u body=>%t<\n",
	                ll,lp,strlinelen(lp,ll,45)) ;
	            debugprintf("progmsgs/procmsgoutbody: f_eol=%u\n",f_eol) ;
		}
#endif

	        if (f_bol && FMAT(lp) && (ll > 5)) {
		    if ((rs = mailmsgmatenv(&pdp->me,lp,ll)) != 0) break ;
		}

	        pdp->off_finish = pdp->offset ;
	        if (f_bol)
	            clines += 1 ;

	        rs = bwrite(pdp->tfp,lp,ll) ;
	        tlen += rs ;

	        ll = 0 ;
	        f_bol = f_eol ;
	        bfliner_readover(blp) ;

	        if (rs < 0) break ;
	    } /* end while (writing w/o CLEN) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progmsgs/procmsgoutbody: out loop rs=%d\n",rs) ;
	  	debugprintf("progmsgs/procmsgoutbody: f_eol=%u\n",f_eol) ;
	    }
#endif

	} /* end if (content length or not) */

/* if no NL at the EOM, write one, but why?? -- for fun! */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	  	debugprintf("progmsgs/procmsgoutbody: rs=%d f_eol=%u\n",
			rs,f_eol) ;
	    }
#endif

	if ((rs >= 0) && (! f_eol)) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progmsgs/procmsgoutbody: extra EOL\n") ;
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
	    debugprintf("progmsgs/procmsgoutbody: ret rs=%d tlen=%u\n",
		rs,tlen) ;
	    debugprintf("progmsgs/procmsgoutbody: f_eof=%u\n",pdp->f.eof) ;
	}
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procmsgoutbody) */


static int procmsgoutback(PROGINFO *pip,PROCDATA *pdp)
{
	MSGINFO		*mip = pdp->mip ;
	offset_t	coff ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgoutback: ent mailmsg_offset=%u\n",
	        pdp->offset) ;
#endif /* CF_DEBUG */

/* write-back out the content-length */

	if (rs >= 0) {
	    int		clen = (pdp->off_finish - pdp->off_body) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progmsgs/procmsgoutback: writing post CLEN\n") ;
	        debugprintf("progmsgs/procmsgoutback: clen=%d:%d\n",
	            pdp->clen,clen) ;
	    }
#endif

	    if ((pdp->clen >= 0) && (pdp->clen < clen)) {
	        clen = pdp->clen ;
	    }

	    if (pip->f.logmsg)
	        proglog_printf(pip,"  clen=%u",clen) ;

	    if ((rs = btell(pdp->tfp,&coff)) >= 0) {
	        bseek(pdp->tfp,pdp->off_clen,SEEK_SET) ;
	        rs = bprintf(pdp->tfp,"%d",clen) ;
	        if (rs >= 0)
	            bseek(pdp->tfp,coff,SEEK_SET) ;
	    }

	    mip->clen = clen ;
	} /* end if (writing back a content length) */

/* write-back out the content-lines (if specified) */

	if ((rs >= 0) && pdp->f.plaintext && (mip->clines < 0)) {
	    int	clines = pdp->clines ;

	    if (pip->f.logmsg)
	        proglog_printf(pip,"  clines=%u",clines) ;

	    if ((rs = btell(pdp->tfp,&coff)) >= 0) {
	        bseek(pdp->tfp,pdp->off_clines,SEEK_SET) ;
	        rs = bprintf(pdp->tfp,"%u",clines) ;
	        if (rs >= 0)
	            bseek(pdp->tfp,coff,SEEK_SET) ;
	    }

	    mip->clines = clines ;
	} /* end if (content-lines) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgoutbackret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgoutback) */


static int procmsglog(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs = SR_OK ;

#if	CF_LOGMLEN
	if (pip->f.logmsg)
	    proglog_printf(pip,"  mlen=%u", pdp->tlen) ;
#endif

	if (pip->f.logmsg && pdp->f.spam) {
	    proglog_printf(pip,"  spam") ;
	}

	return rs ;
}
/* end subroutine (procmsglog) */


static int procmsgct(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp)
{
	MHCOM		c ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	cchar		*hp ;

	pdp->f.ct_plaintext = TRUE ;
	if ((hl = mailmsg_hdrval(msgp,HN_CTYPE,&hp)) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgct: hdr=>%t<\n",hp,hl) ;
#endif

	if ((rs = mhcom_start(&c,hp,hl)) >= 0) {
	    int		vl ;
	    cchar	*tp, *vp ;
	    if ((vl = mhcom_getval(&c,&vp)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progmsgs/procmsgct: v=>%t<\n",vp,vl) ;
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

	} /* end if (hdr-value) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgct: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgct) */


static int procmsgce(PROGINFO *pip,PROCDATA *pdp,MAILMSG *msgp)
{
	COMPARSE	com ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	cchar		*hp ;

	pdp->f.ce_text = TRUE ;
	if ((hl = mailmsg_hdrval(msgp,HN_CENCODING,&hp)) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgce: hdr=>%t<\n",hp,hl) ;
#endif

	if ((rs = comparse_start(&com,hp,hl)) >= 0) {
	    int		vl ;
	    cchar	*tp, *vp ;
	    if ((vl = comparse_getval(&com,&vp)) > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progmsgs/procmsgce: v=>%t<\n",vp,vl) ;
#endif

	        if ((tp = strnchr(vp,vl,';')) != NULL)
	            vl = (tp - vp) ;

	        rs1 = sisub(vp,vl,"7bit") ;
	        if (rs1 < 0)
	            rs1 = sisub(vp,vl,"8bit") ;

	        pdp->f.ce_text = (rs1 >= 0) ;

	    } /* end if */
	    comparse_finish(&com) ;
	} /* end if (comparse) */

	} /* end if (hdr-value) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progmsgs/procmsgce: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procmsgce) */


static int procmsghdrval(pip,pdp,msgp,hname,valp)
PROGINFO	*pip ;
PROCDATA	*pdp ;
MAILMSG		*msgp ;
cchar		hname[] ;
int		*valp ;
{
	int		rs ;
	int		v = -1 ;
	int		vl = 0 ;
	cchar		*vp ;

	if (hname == NULL) return SR_FAULT ;
	if (pdp == NULL) return SR_FAULT ;

	if (hname[0] == '\0') return SR_INVALID ;

	if ((rs = mailmsg_hdrval(msgp,hname,&vp)) >= 0) {
	    vl = rs ;

	    rs = hdrextnum(vp,vl) ;
	    v = rs ;

	} /* end if */

	if (valp != NULL)
	    *valp = (rs >= 0) ? v : -1 ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (procmsghdrval) */


static int procspam(PROGINFO *pip,PROCDATA *pdp)
{
	int		rs = SR_OK ;
	int		f_spam = FALSE ;

	if (pip->f.spam && (! pdp->f.spam)) {
	    MSGINFO	*mip = pdp->mip ;
	    MAILMSG	*msgp = pdp->msgp ;

/* check spam-flag */

#if	CF_SPAMFLAG
	if ((rs >= 0) && (! pdp->f.spam)) {
	    if ((rs = procmailmsg_spamflag(pip,msgp)) > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }
	} /* end if (spam-flag check) */
#endif /* CF_SPAMFLAG */

/* check spam-status */

#if	CF_SPAMSTATUS
	if ((rs >= 0) && (! pdp->f.spam)) {
	    if ((rs = procmailmsg_spamstatus(pip,msgp)) > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }
	} /* end if (spam-status check) */
#endif /* CF_SPAMSTATUS */

/* check bogosity */

#if	CF_BOGOSITY
	if ((rs >= 0) && (! pdp->f.spam)) {
	    if ((rs = procmailmsg_bogosity(pip,msgp)) > 0) {
	        f_spam = TRUE ;
	        pdp->f.spam = TRUE ;
	    }
	} /* end if (bogosity check) */
#endif /* CF_BOGOSITY */

	    mip->f.spam = pdp->f.spam ;
	} /* end if (additional spam processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procspam: ret rs=%d f_spam=%u\n",rs,f_spam) ;
#endif

	return (rs >= 0) ? f_spam : rs ;
}
/* end subroutine (procspam) */


/* does two things: 1) it extracts and saves the 1st EMA and 2) logs it */
static int procmsglogaddr(pip,addrbuf,hdr,ap,al)
PROGINFO	*pip ;
char		addrbuf[] ;
cchar		hdr[] ;
cchar		*ap ;
int		al ;
{
	EMA		a ;
	EMA_ENT		*ep ;
	const int	hlen = HDRNAMELEN ;
	int		rs = SR_OK ;
	int		j ;
	int		hnl = -1 ;
	int		cl ;
	int		c = 0 ;
	int		f_route = FALSE ;
	cchar		*sp ;
	cchar		*hnp = hdr ;
	cchar		*cp ;
	char		hbuf[HDRNAMELEN + 1] ;

	if (ap == NULL) return SR_FAULT ;

	if (al < 0) al = strlen(ap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsglogaddr: ent a=>%t<\n",
	        ap,strlinelen(ap,al,40)) ;
#endif

	if ((rs = ema_start(&a)) >= 0) {
	    if (ema_parse(&a,ap,al) >= 0) {
	        for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	            f_route = TRUE ;
	            cp = ep->rp ;
	            if (cp == NULL) {
	                f_route = FALSE ;
	                cp = ep->ap ;
	            }

	            if (cp != NULL) {

	                if ((c == 0) && (addrbuf != NULL))
	                    strwcpy(addrbuf,cp,MAILADDRLEN) ;

	                if (pip->f.logmsg) {

	                    if ((rs = mknewhdrname(hbuf,hlen,hdr)) > 0) {
	                        hnp = hbuf ;
	                        hnl = rs ;
	                    } else
	                        hnl = strlen(hnp) ;

			    if (rs >= 0) {

	                    cl = strnlen(cp,(LOGLINELEN -2 -hnl -1)) ;
	                    proglog_printf(pip,"  %s=%t",hnp,cp,cl) ;

	                    sp = ep->cp ; /* EMA-comment */
	                    if (sp != NULL) {

	                        if ((cl = sfsubstance(sp,ep->cl,&cp)) > 0) {
	                            cchar	*fmt = "    (%t)" ;
	                            cl = MIN(cl,(LOGLINELEN -4 -2)) ;
	                            proglog_printf(pip,fmt,cp,cl) ;
	                        }

	                    } /* end if */

	                    if (f_route && (ep->cp == NULL) &&
	                        (ep->ap != NULL)) {

	                        sp = ep->ap ; /* EMA-address */
	                        if ((cl = sfsubstance(sp,ep->al,&cp)) > 0) {
				    cchar	*fmt = "    (%t)" ;
	                            cl = MIN(cl,(LOGLINELEN -4 -2)) ;
	                            proglog_printf(pip,fmt,cp,cl) ;
	                        }

	                    } /* end if */

			    } /* end if */

	                } /* end if (logging enabled) */

	                c += 1 ;
	                if ((! pip->f.logmsg) && (c > 0))
	                    break ;

	            } /* end if (got an address) */

		    if (rs < 0) break ;
	        } /* end for (ema-parse) */
	    } /* end if (parse) */
	    ema_finish(&a) ;
	} /* end if (ema) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procmsglogaddr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsglogaddr) */


static int procmailmsg_spamflag(PROGINFO *pip,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	cchar		*tp, *sp ;
	cchar		*cp ;

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


static int procmailmsg_spamstatus(PROGINFO *pip,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	cchar		*tp, *sp ;
	cchar		*cp ;

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


static int procmailmsg_bogosity(PROGINFO *pip,MAILMSG *msgp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		f_spam = FALSE ;
	cchar		*tp, *sp ;
	cchar		*cp ;

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


#if	CF_SPAMSUBJECT

static int procmailmsg_spamsubj(PROGINFO *pip,cchar sp[],int sl)
{
	int		f = FALSE ;
	cchar		*tp ;

	if (pip == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if ((tp = strnchr(sp,sl,'[')) != NULL) {
	    int		si ;
	    sl = (sp + sl) - (tp + 1) ;
	    sp = (tp + 1) ;
	    if ((si = sisub(sp,sl,"SPAM")) < 0) {
	        si = sisub(sp,sl,"Possible UCE") ;
	    }
	    if (si >= 0) {
	        sl -= si ;
	        sp += si ;
	        if ((tp = strnchr(sp,sl,']')) != NULL) {
	            f = TRUE ;
	        }
	    }
	} /* end if (possible) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgs/procmailmsg_spamsubj: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (procmailmsg_spamsubj) */

#endif /* CF_SPAMSUBJECT */


/* make (really clean-up) a header key-name */
static int mknewhdrname(char hbuf[],int hlen,cchar hdrname[])
{
	int		rs = SR_OK ;
	int		rlen = hlen ;
	int		j = 0 ;
	int		f = FALSE ;

	f = f || (strchr(hdrname,'-') != NULL) ;
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


static int cmpheadname(cchar **e1pp,cchar **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
		if (*e2pp != NULL) {
		    rc = strcasecmp(*e1pp,*e2pp) ;
		} else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpheadname) */


