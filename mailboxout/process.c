/* process */

/* process the input messages and spool them up */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_FRESHDAYTIME	1		/* use fresh daytime */
#define	CF_SPAMSUBJECT	1		/* check SUBJECT for spam */
#define	CF_SPAMFLAG	1		/* check spam flag for spam */


/* revision history:

	= 1997-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
        I added a little code to "post" articles that do not have a valid
        newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module processes one or more mail messages (in appropriate mailbox
        format if more than one) on STDIN. The output is a single file that is
        ready to be added to each individual mailbox in the spool area.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<pcsconf.h>
#include	<mailmsgmatenv.h>
#include	<msg.h>
#include	<msgheaders.h>
#include	<ema.h>
#include	<dater.h>
#include	<tmz.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"logzones.h"
#include	"emainfo.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	ADDRLEN
#define	ADDRLEN		(2 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	LOGLINEBUFLEN	(80 - 16)

#define	DATEBUFLEN	80
#define	STACKADDRBUFLEN	(2 * 1024)

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#define	FMAT(cp)	((cp)[0] == 'F')


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sisub(const char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mailmsgmathdr(const char *,int,char *,int *) ;
extern int	unlinkd(const char *,int) ;
extern int	mkmsgid(struct proginfo *,char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*trailaddr(const char *,int) ;


/* external variables */


/* local structures */

struct lineinfo {
	char	lbuf[LINEBUFLEN + 1] ;
	int	linelen ;
} ;

struct procdata {
	bfile	*ifp ;
	bfile	*tfp ;
	vecobj	*tip ;
	MAILMSGMATENV	ei ;
	DATER	edate ;
	offset_t	offset, off_start, off_clen, off_body, off_finish ;
	int	mlen ;
	int	clen ;
	char	efrom[ADDRLEN + 1] ;
} ;

struct msginfo {
	char	*efrom ;
	char	*hsender ;
	char	*hfrom ;
	char	*mid ;
	DATER	edate ;
	time_t	etime ;
	time_t	mtime ;			/* message time */
	int	clen ;
	char	from[ADDRLEN + 1] ;
	char	messageid[ADDRLEN + 1] ;
} ;


/* forward references */

static int	procmsg(struct proginfo *, struct procdata *, char *,int) ;
static int	procmsgenv(struct proginfo *,struct msginfo *, MSG *,
			char *,int) ;
static int	ext_id(struct proginfo *,char *,char *) ;
static int	ph_write() ;
static int	didheader(vecstr *,struct msg_header *) ;
static int	sfcomment(const char *,int,char **) ;
static int	cheapspamcheck(struct proginfo *,const char *,int) ;


/* local variables */

static const char	atypes[] = "LUIR" ;	/* address types */


/* exported subroutines */


int process(pip,ifp,tfp,fip)
struct proginfo	*pip ;
bfile		*ifp ;
bfile		*tfp ;
vecobj		*fip ;
{
	struct pcsconf		*pp = pip->pp ;

	struct procdata		pd ;

	LOGZONES		lz ;

	int	rs = SR_OK ;
	int	len = 0 ;
	int	narticles = 0 ;
	int	f_bol, f_eol ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	hname[BUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered\n") ;
#endif

	memset(&pd,0,sizeof(struct procdata)) ;

	pd.ifp = ifp ;
	pd.tfp = tfp ;
	pd.tip = fip ;
	dater_startcopy(&pd.edate,&pip->tmpdate) ;

/* find the start of a message */

	pd.offset = 0L ;
	pd.off_start = 0L ;
	f_bol = TRUE ;
	len = breadline(ifp,lbuf,LINEBUFLEN) ;

	if (len > 0) {

	    f_eol = (lbuf[len - 1] == '\n') ;
	    pd.offset += len ;

	}

/* top of loop */

	while (len > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: top of loop\n") ;
#endif

/* match on either an envelope or a header! */

	    while ((len > 0) && FMAT(lbuf)) {
		rs = mailmsgmatenv(&pd.ei,lbuf,len) ;
		if (rs <= 0) break ;
	        if ((rs = mailmsgmathdr(lbuf,len,hname,NULL)) != 0) break ;
	        if ((len <= 1) && (lbuf[0] == '\n')) break ;

		pd.off_start = pd.offset ;
	        len = breadline(ifp,lbuf,LINEBUFLEN) ;
	        pd.offset += len ;

	    } /* end while */

/* EOF already? */

	    if (rs < 0) break ;
	    if (len <= 0) break ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: start message, blank=%d\n",
	            (lbuf[0] == '\n')) ;
#endif

	    if (pip->open.logfile)
	        logfile_printf(&pip->lh,"message %4d",
			pip->msgn) ;

	    rs = procmsg(pip,&pd,lbuf,len) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: procmsg() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

	    len = rs ;

	    pip->msgn += 1 ;
	    narticles += 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("process: bottom loop\n") ;
		if (len > 0)
		debugprintf("process: line=>%t<\n",
			lbuf,
			((lbuf[len - 1] == '\n') ? (len - 1) : len)) ;
	    }
#endif /* CF_DEBUG */

/* get the next line if any (prime 'lbuf' for MSG-start check) */

/* if the article had a content length, next article is beyond it */

	    if ((pd.clen >= 0) && (len > 0)) {

		pd.off_start = pd.offset ;
	        f_bol = TRUE ;
	        while ((len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	            f_eol = (lbuf[len - 1] == '\n') ;

	            pd.offset += len ;
	            if (f_bol) {
		        if FMAT(lbuf)) {
	                   if ((rs = mailmsgmatenv(&pd.ei,lbuf,len)) != 0) {
				break ;
			    }
			}
	                if ((rs = mailmsgmathdr(lbuf,len,hname,NULL)) != 0) 
			    break ;
			}
		    }

			pd.off_start = pd.offset ;
	            f_bol = f_eol ;

	        } /* end while (throwing away lines for a new start) */

	    } /* end if (post pocessing for a message w/ CLEN) */

	    pd.efrom[0] = '\0' ;

	} /* end while (processing article messages) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: ret len=%d narticles=%d\n",
	        len,narticles) ;
#endif

	dater_finish(&pd.edate) ;

	return narticles ;
}
/* end subroutine (process) */


/* local subroutines */


/* process the current message */
static int procmsg(pip,mdp,lbuf,len)
struct proginfo	*pip ;
struct procdata	*mdp ;
char		lbuf[] ;
int		len ;
{
	struct msginfo		mi ;

	MSG			amsg, *msgp = &amsg ;

	MSGHEADERS		hv ;

	MSG_HEADER		*mhp ;

	bfile			*ifp = mdp->ifp ;

	vecstr			wh ;

	offset_t			fileoff ;

	int	rs, rs1, sl, cl, i, j ;
	int	f_bol, f_eol, f_eom = FALSE ;
	int	f_spam = FALSE ;
	int	f_messageid ;

	char	datebuf[DATEBUFLEN + 1] ;
	char	stackaddrbuf[STACKADDRBUFLEN + 1] ;
	const char	*sp, *cp ;


	mdp->mlen = 0 ;
	memset(&mi,0,sizeof(struct msginfo)) ;

	dater_startcopy(&mi.edate,&pip->tmpdate) ;

/* process the headers of this message */

	rs = msg_init(msgp,ifp,mdp->offset,lbuf,len) ;

	if (rs < 0)
	    goto ret0 ;

	sl = rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("procmsg: msg_init() rs=%d\n",sl) ;

	    for (i = 0 ; msg_getheader(msgp,i,&mhp) >= 0 ; i += 1) {

	        debugprintf("procmsg: %s> %t\n",mhp->name,
	            mhp->value,MIN(mhp->vlen,50)) ;

	    }
	}
#endif /* CF_DEBUG */

	mdp->offset += sl ;
	rs = msgheaders_init(&hv,msgp) ;		/* index headers */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: msgheaders_init() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret1 ;

/* OK, process what we can from this article */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: got stuff from it, let's process\n") ;
#endif

/* stack the envelope addresses */

	rs = procmsgenv(pip,&mi,msgp,stackaddrbuf,STACKADDRBUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: procmsgenv() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: efrom=%s\n",mi.efrom) ;
#endif


/* form the envelope address that we will be using for this message */

	if ((mi.efrom == NULL) || (! pip->f.trusted)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procmsg: supplying new envelope from\n") ;
#endif

	    strwcpy(mdp->efrom,pip->fromaddr,ADDRLEN) ;

#if	CF_FRESHDAYTIME
	    dater_settimezn(&mdp->edate,pip->daytime,pip->zname,-1) ;
#else
	    dater_setcopy(&mdp->edate,&pip->tmpdate) ;
#endif

	} else {

	    strwcpy(mdp->efrom,mi.efrom,ADDRLEN) ;

	    dater_setcopy(&mdp->edate,&mi.edate) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: mdp->efrom=%s\n",mdp->efrom) ;
#endif

	if (pip->open.logfile)
	logfile_printf(&pip->lh,"  F %t",
	    mdp->efrom,strnlen(mdp->efrom,(LOGLINEBUFLEN - 4))) ;


/* initialize an object to track which headers we have processed */

	rs = vecstr_start(&wh,HI_NULL,VECSTR_PHOLES) ;

	if (rs < 0)
	    goto ret2 ;


/* message ID */

	mi.messageid[0] = '\0' ;
	if (hv.v[HI_MESSAGEID] != NULL) {

	    ext_id(pip,hv.v[HI_MESSAGEID],mi.messageid) ;

	    vecstr_add(&wh,mailmsghdrs_names[HI_MESSAGEID],HL_MESSAGEID) ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: 1 messageid=%s\n",mdp->messageid) ;
#endif

	f_messageid = TRUE ;
	if (mi.messageid[0] == '\0') {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: making our own messageid\n") ;
#endif

	f_messageid = FALSE ;
	    mkmsgid(pip,mi.messageid,ADDRLEN,pip->nmsgs) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("procmsg: 2 messageid=%s\n",mi.messageid) ;
	    debugprintf("procmsg: end messageid\n") ;
	}
#endif /* CF_DEBUG */

	{
	    int	f_first = TRUE ;
	    const char	*fmt ;

	    sp = mi.messageid ;
	    sl = strlen(sp) ;

	    while (sl > 0) {

	        cp = sp ;
	        cl = MIN(sl,(LOGLINEBUFLEN - 8)) ;

	        if (f_first) {

	            f_first = FALSE ;
	            fmt = "  mid=| %t" ;

	        } else
	            fmt = "      | %t" ;

		if (pip->open.logfile)
	            logfile_printf(&pip->lh,fmt, cp,cl) ;

	        sp += cl ;
	        sl -= cl ;

	    } /* end while */

	} /* end block */


/* get the content length if this article has one */

	mdp->clen = -1 ;
	if (hv.v[HI_CLEN] != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: we have a 'content-length' header\n") ;
#endif

	    sp = hv.v[HI_CLEN] ;
	    sl = -1 ;
	    if ((cp = strchr(sp,',')) != NULL)
		sl = (cp - sp) ;

	    if (cfdeci(sp,sl,&mdp->clen) < 0)
	        mdp->clen = -1 ;

	    vecstr_add(&wh,mailmsghdrs_names[HI_CLEN],HL_CLEN) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: clen=%d\n",mdp->clen) ;
#endif


/* log some information that is useful for tracking */


/* sender */

	rs1 = -1 ;
	if (hv.v[HI_SENDER] != NULL)
	    rs1 = msg_headersearch(msgp,HN_SENDER,HL_SENDER,
	        &mhp) ;

	if (rs1 >= 0) {
	    EMA		a ;
	    EMA_ENT	*ep ;

	    ema_start(&a) ;

	    ema_parse(&a,mhp->value,mhp->vlen) ;

	    for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	        if ((cp = ep->route) == NULL)
	            cp = ep->address ;

	        if ((cp != NULL) && pip->open.logfile)
	            logfile_printf(&pip->lh,"  sender=%t",
	                cp,strnlen(cp,(LOGLINEBUFLEN - 9))) ;

	    } /* end for */

	    ema_finish(&a) ;

	} /* end if (sender) */


/* errors-to */

	rs1 = -1 ;
	if (hv.v[HI_ERRORSTO] != NULL)
	    rs1 = msg_headersearch(msgp,HN_ERRORSTO,HL_ERRORSTO,
	        &mhp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procmsg: errorsto i=%d \n", rs1) ;
		if (rs1 >= 0)
	    debugprintf("procmsg: errorsto i=%d value=>%t<\n",
	    	rs1,mhp->value,mhp->vlen) ;
	}
#endif

	if (rs1 >= 0) {

	    EMA		a ;

	    EMA_ENT	*ep ;


	    ema_start(&a) ;

	    ema_parse(&a,mhp->value,mhp->vlen) ;

	    for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	        if ((cp = ep->route) == NULL)
	            cp = ep->address ;

	        if ((cp != NULL) && pip->open.logfile)
	            logfile_printf(&pip->lh,"  errorsto=%t",
	                cp,strnlen(cp,(LOGLINEBUFLEN - 11))) ;

	    } /* end for */

	    ema_finish(&a) ;

	} /* end if (errors-to) */

/* start writing the output file */

	btell(mdp->tfp,&fileoff) ;	/* get and save output offset */

/* write out our own (new) envelope */

	dater_mkstd(&mdp->edate,datebuf,DATEBUFLEN) ;

	rs1 = bprintf(mdp->tfp,"From %s %s\n",
	    mdp->efrom, datebuf) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

/* OK, we now write out the headers as we have them */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: checking> %s\n",HN_RETURNPATH) ;
#endif

	rs1 = -1 ;
	if (hv.v[HI_RETURNPATH] != NULL)
	    rs1 = msg_headersearch(msgp,HN_RETURNPATH,HL_RETURNPATH,
	        &mhp) ;

	if (rs1 >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: writing> %s\n",HN_RETURNPATH) ;
#endif

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_RETURNPATH,HL_RETURNPATH,-1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	    vecstr_add(&wh,mailmsghdrs_names[HI_RETURNPATH],HL_RETURNPATH) ;

	} else {

	    rs1 = bprintf(mdp->tfp,"%t: %s\n",
	        mailmsghdrs_names[HI_RETURNPATH],HL_RETURNPATH,
	        mdp->efrom) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	} /* end if (handling return path) */


/* OK, now we put the "Received" lines back */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: checking> %s\n",HN_RECEIVED) ;
#endif

	rs1 = -1 ;
	if (hv.v[HI_RECEIVED] != NULL)
	    rs1 = msg_headersearch(msgp,HN_RECEIVED,HL_RECEIVED,&mhp) ;

	if (rs1 >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: writing> %s\n",HN_RECEIVED) ;
#endif

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_RECEIVED,HL_RECEIVED,0) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	    vecstr_add(&wh,mailmsghdrs_names[HI_RECEIVED],HL_RECEIVED) ;

	} /* end if */


/* add a place-holder header for content-Length */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: writing> %s %d\n",
	        HN_CLEN,mdp->clen) ;
#endif

	rs1 = bprintf(mdp->tfp,"%s: ",HN_CLEN) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	btell(mdp->tfp,&mdp->off_clen) ;

	rs1 = bprintf(mdp->tfp,"                \n") ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;


/* Message-Id (we either have the original or the one we made) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: writing> %s\n",HN_MESSAGEID) ;
#endif

	rs1 = bprintf(mdp->tfp,"%s: <%s>\n",HN_MESSAGEID,mi.messageid) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;


/* put all remaining headers out */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: put all remaining\n") ;
#endif

/* mark the following headers as done since we "do" them at the end */

	vecstr_add(&wh,mailmsghdrs_names[HI_FROM],HL_FROM) ;

	vecstr_add(&wh,mailmsghdrs_names[HI_TO],HL_TO) ;

	vecstr_add(&wh,mailmsghdrs_names[HI_REFERENCES],HL_REFERENCES) ;

	vecstr_add(&wh,mailmsghdrs_names[HI_DATE],HL_DATE) ;

	vecstr_add(&wh,mailmsghdrs_names[HI_SUBJECT],HL_SUBJECT) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: put remaining looping\n") ;
#endif

	for (i = 0 ; msg_getheader(msgp,i,&mhp) >= 0 ; i += 1) {

	    if (mhp == NULL) continue ;

	    if (! didheader(&wh,mhp)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("procmsg: remaining writing> %\n",
	                mhp->name) ;
#endif

	        rs1 = ph_write(pip,mdp->tfp,mhp,
	            mhp->name,mhp->nlen,0) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	    } /* end if */

	} /* end for (putting all other headers out) */


/* put out the headers that we reserved for the end of the header area */

/* from */

	rs1 = -1 ;
	if (hv.v[HI_FROM] != NULL)
	    rs1 = msg_headersearch(msgp,HN_FROM,HL_FROM,
	        &mhp) ;

	if (rs1 >= 0) {

	    EMA		a ;

	    EMA_ENT	*ep ;

		int	c ;


	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_FROM,HL_FROM,1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	    ema_start(&a) ;

	    ema_parse(&a,mhp->value,mhp->vlen) ;

		c = 0 ;
	    for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	        if ((cp = ep->route) == NULL)
	            cp = ep->address ;

	        if (cp != NULL) {

			if (c == 0)
				strwcpy(mi.from,cp,ADDRLEN) ;

		    if (pip->open.logfile) {

	            logfile_printf(&pip->lh,"  from=%t",
	                cp,strnlen(cp,(LOGLINEBUFLEN - 7))) ;

		    sp = ep->comment ;
		    if (sp != NULL) {

			cl = sfcomment(sp,ep->clen,&cp) ;

			if (cl > 0)
	            	    logfile_printf(&pip->lh,"       (%t)",
	                	cp,MIN(cl,(LOGLINEBUFLEN - 9))) ;

		    }

		    } /* end if (logging enabled) */

			c += 1 ;

		} /* end if (got an address) */

	    } /* end for */

	    ema_finish(&a) ;

	} /* end if (from) */

/* to */

	rs1 = -1 ;
	if (hv.v[HI_TO] != NULL)
	    rs1 = msg_headersearch(msgp,HN_TO,HL_TO,
	        &mhp) ;

	if (rs1 >= 0) {

	    EMA		a ;

	    EMA_ENT	*ep ;


	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_TO,HL_TO,1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	if (pip->open.logfile) {

	    ema_start(&a) ;

	    ema_parse(&a,mhp->value,mhp->vlen) ;

	    for (j = 0 ; ema_get(&a,j,&ep) >= 0 ; j += 1) {

	        if ((cp = ep->route) == NULL)
	            cp = ep->address ;

	        if (cp != NULL) {

	            logfile_printf(&pip->lh,"  to=%t",
	                cp,strnlen(cp,(LOGLINEBUFLEN - 7))) ;

		    sp = ep->comment ;
		    if (sp != NULL) {

			cl = sfcomment(sp,ep->clen,&cp) ;

			if (cl > 0)
	            	    logfile_printf(&pip->lh,"       (%t)",
	                	cp,MIN(cl,(LOGLINEBUFLEN - 9))) ;

		    }
		}

	    } /* end for */

	    ema_finish(&a) ;

	} /* end if (logging enabled) */

	} /* end if (to) */


/* references */

	rs1 = -1 ;
	if (hv.v[HI_REFERENCES] != NULL)
	    rs1 = msg_headersearch(msgp,HN_REFERENCES,HL_REFERENCES,
	        &mhp) ;

	if (rs1 >= 0) {

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_REFERENCES,HL_REFERENCES,1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	}


/* date of posting */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: about to do DATE\n") ;
#endif

	rs1 = -1 ;
	if (hv.v[HI_DATE] != NULL)
	    rs1 = msg_headersearch(msgp,HN_DATE,HL_DATE,&mhp) ;

	if (rs1 >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: got a DATE\n") ;
#endif

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_DATE,HL_DATE,0) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

/* get some information from the (first) date in the message */

	    if (pip->open.logfilezones) {

		char	*vp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: checking for zones\n") ;
#endif

/* get the first instance value for this header */

	    rs1 = msg_headerivalue(msgp,HN_DATE,0,&vp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsg: msg_headerivalue() rs=%d\n",rs1) ;
#endif

/* log the time-zone name and time-zone offset */

	    if ((rs1 >= 0) && (vp != NULL)) {

		TMZ	info ;

		int	zoff, vlen = rs1 ;


		rs = tmz_msg(&info,vp,vlen) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: tmz zname=>%t< offset=%d\n",
		info.zname,strnlen(info.zname,TMZ_ZNAMESIZE),info.zoff) ;
#endif

		if ((rs >= 0) && (info.zname[0] != '\0')) {

		    zoff = (info.f.zoff) ? info.zoff : LOGZONES_NOZONEOFFSET ;
		    logzones_update(&pip->lz,info.zname,TMZ_ZNAMESIZE,
			zoff,pip->stamp) ;

		}

		} /* end if (had a date) */

	    } /* end if (logging time-zone information) */

/* log the date also */

	    rs = SR_NOENT ;
	    if (mhp->value != NULL) {

	        rs = dater_setmsg(&pip->tmpdate,mhp->value,mhp->vlen) ;

		if (rs < 0)
	            logfile_printf(&pip->lh,"  bad_date=>%t<",
	                mhp->value,mhp->vlen) ;

	    }

	    if (rs >= 0) {

		dater_gettime(&pip->tmpdate,&mi.mtime) ;

	        dater_mkstrdig(&pip->tmpdate,datebuf,DATEBUFLEN) ;

		if (pip->open.logfile)
	        logfile_printf(&pip->lh,"  date=%s",
	            datebuf) ;

	    }

	} /* end if (date) */


/* subject */

	rs1 = -1 ;
	if (hv.v[HI_SUBJECT] != NULL)
	    rs1 = msg_headersearch(msgp,HN_SUBJECT,HL_SUBJECT,
	        &mhp) ;

	if (rs1 >= 0) {

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        HN_SUBJECT,HL_SUBJECT,1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

/* check for SPAM */

#if	CF_SPAMSUBJECT
		if ((! f_spam) && pip->f.spam) {

			rs1 = cheapspamcheck(pip,mhp->value,mhp->vlen) ;

			f_spam = (rs1 > 0) ;

		} /* end if (spam check) */
#endif /* CF_SPAMSUBJECT */

	} /* end if (had a SUBJECT header) */


/* spam flag check */

#if	CF_SPAMFLAG

		if ((! f_spam) && pip->f.spam) {

#ifdef	HI_SPAMFLAG
	rs1 = -1 ;
	if (hv.v[HI_SUBJECT] != NULL)
	    rs1 = msg_headersearch(msgp,HN_SUBJECT,HL_SUBJECT, &mhp) ;
#else
	    rs1 = msg_headersearch(msgp,"s-spam-flag",11, &mhp) ;
#endif /* defined(HI_SPAMFLAG) */

	if (rs1 >= 0) {

		f_spam = (sfsub(mhp->value,mhp->vlen,"YES",&cp) >= 0) ;

	    rs1 = ph_write(pip,mdp->tfp,mhp,
	        "x-spam-flag",11,1) ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;

	}

	} /* end if (spam flag test) */

#endif /* CF_SPAMFLAG */


/* write out the end-of-header marker (a blank line) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: writing> EOH\n") ;
#endif

	rs1 = bprintf(mdp->tfp,"\n") ;

	if (rs1 >= 0)
		mdp->mlen += rs1 ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procmsg: copying body, clen=%d\n",mdp->clen) ;
#endif

/* copy the body of the input to the output */

	mdp->off_body = mdp->offset ;
	mdp->off_finish = mdp->offset ;
	if (mdp->clen >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: we are writing w/ CLEN\n") ;
#endif

	    f_eom = FALSE ;
	    sl = mdp->clen ;
	    while ((sl > 0) &&
	        ((len = breadline(ifp,lbuf,MIN(sl,LINEBUFLEN))) > 0)) {

	        mdp->offset += len ;
	        f_eol = (lbuf[len - 1] == '\n') ;

	        if (f_bol && FMAT(lbuf)) {
		    ((rs = mailmsgmatenv(&mdp->ei,lbuf,len)) != 0) break ;
	        }

	        mdp->off_finish = mdp->offset ;
		if (rs >= 0) {
	        rs = bwrite(mdp->tfp,lbuf,len) ;
		mdp->mlen += rs ;
		}

	        sl -= len ;
	        f_bol = f_eol ;
		f_eom = f_eol ;

	        if (rs < 0) break ;
	    } /* end while (writing w/ CLEN) */

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procmsg: we are writing w/o CLEN\n") ;
#endif

	    f_bol = TRUE ;
	    while ((len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	        mdp->offset += len ;
	        f_eol = (lbuf[len - 1] == '\n') ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("procmsg: body line> %t",lbuf,len) ;
#endif

	        if (f_bol && FMAT(lbuf)) {
		    if ((rs = mailmsgmatenv(&mdp->ei,lbuf,len)) != 0) break ;
		}

	        mdp->off_finish = mdp->offset ;
	        rs = bwrite(mdp->tfp,lbuf,len) ;
		mdp->mlen += rs ;

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (writing w/o CLEN) */

		f_eom = f_eol ;

	} /* end if (content length or not) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	debugprintf("process/procmsg: EOM determination rs=%d f_eom=%d\n",
		rs,f_eom) ;
#endif

	if ((rs >= 0) && (! f_eom)) {

	    rs = bwrite(mdp->tfp,"\n",1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	debugprintf("process/procmsg: bwrite() rs=%d\n",rs) ;
#endif

		if (rs >= 0)
	        mdp->mlen += rs ;

	}

/* write out the content length */

	if (rs >= 0) {

	    offset_t	off_current ;

	    int		blen = mdp->off_finish - mdp->off_body ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsg: writing post CLEN\n") ;
	        debugprintf("procmsg: clen=%d blen=%d\n",
	            mdp->clen,blen) ;
	    }
#endif

	    if ((mdp->clen >= 0) && (mdp->clen < blen))
	        blen = mdp->clen ;

	if (pip->open.logfile)
	    logfile_printf(&pip->lh,"  clen=%d",
	        blen) ;

	    btell(mdp->tfp,&off_current) ;

	    bseek(mdp->tfp,mdp->off_clen,SEEK_SET) ;

	    rs = bprintf(mdp->tfp,"%d",blen) ;

	    if (rs >= 0)
	        bseek(mdp->tfp,off_current,SEEK_SET) ;

	} /* end if (writing back a content length) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procmsg: closing article file\n") ;
		debugprintf("procmsg: rs=%d len=%d\n",rs,len) ;
	}
#endif /* CF_DEBUG */


	{
		struct msgoff	me ;


	memset(&me,0,sizeof(struct msgoff)) ;

	me.offset = fileoff ;
	me.mlen = mdp->mlen ;
	me.mtime = mi.mtime ;
	me.f_spam = f_spam ;

	strwcpy(me.from,mi.from,ADDRLEN) ;

	if (f_messageid)
	strwcpy(me.messageid,mi.messageid,ADDRLEN) ;

	vecobj_add(mdp->tip,&me) ;

	if (pip->open.logfile && f_spam)
	    logfile_printf(&pip->lh,"  spam") ;

	} /* end block */


/* clean up the junk lying ALL around this place ! */
ret3:
	vecstr_finish(&wh) ;

/* we are done with this article, let's clean up a bit */
ret2:
	msgheaders_free(&hv) ;		/* order -- first */

ret1:
	msg_free(msgp) ;		/* order -- second */

ret0:
	dater_finish(&mi.edate) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("procmsg: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procmsg) */


static int procmsgenv(pip,mip,msgp,addrbuf,addrlen)
struct proginfo	*pip ;
struct msginfo	*mip ;
MSG		*msgp ;
char		addrbuf[] ;
int		addrlen ;
{
	MSG_ENV	*mep ;

	int	rs, i, sl ;
	int	al = 0 ;
	int	abl = addrlen, sabl ;
	int	sal = 0 ;
	int	f_remote ;

	char	*ap = addrbuf ;
	char	*sap = addrbuf ;
	char	*addr = NULL ;


	mip->efrom = NULL ;

	sal = 0 ;
	for (i = 0 ; msg_getenv(msgp,i,&mep) >= 0 ; i += 1) {
	    EMAINFO	ai ;
	    int		atype ;
	    char	datebuf[DATEBUFLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: efrom=%s\n",mep->from) ;
	        debugprintf("procmsgenv: edate=>%s<\n",mep->date) ;
	}
#endif

	    if (pip->open.logfileenv) {

		logfile_printf(&pip->envsum,"%4d:%2d F %s",
			pip->msgn,i,mep->from) ;

		logfile_printf(&pip->envsum,"%4d:%2d D %s",
			pip->msgn,i,mep->date) ;

		if (mep->remote)
		    logfile_printf(&pip->envsum,"%4d:%2d R %s",
			pip->msgn,i,mep->remote) ;

	    }

	    rs = dater_setstd(&mip->edate,mep->date,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procmsgenv: dater_setstd() rs=%d\n",rs) ;
#endif

	    datebuf[0] = '\0' ;
	    if (rs >= 0) {

	        rs = dater_mkstrdig(&mip->edate,datebuf,DATEBUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: dater_mkstrdig() rs=%d\n",rs) ;
	        debugprintf("procmsgenv: edate=>%s<\n",datebuf) ;
	    }
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

	    f_remote = (mep->remote != NULL) ;
	    if (f_remote) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procmsgenv: eremote=%s\n",mep->remote) ;
#endif

	        sl = strwcpy(ap,mep->remote,(abl - 1)) - ap ;

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

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: sal=%d addrbuf=%t\n",
	            sal,addrbuf,strnlen(addrbuf,sal)) ;
	        debugprintf("procmsgenv: emainfo() \n") ;
	    }
#endif

	    rs = emainfo(&ai,mep->from,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: emainfo() rs=%d\n",rs) ;
	        debugprintf("procmsgenv: ai.host=%t ai.local=%t\n",
	            ai.host,ai.hlen,ai.local,ai.llen) ;
	    }
#endif

	    atype = rs ;
	    sl = emainfo_mktype(&ai, EMAINFO_TUUCP, ap,abl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: emainfo_mktype() rs=%d addrbuf=%t\n",
	            sl,
	            addrbuf,strnlen(addrbuf,al)) ;
	        debugprintf("procmsgenv: addr=%t\n",
	            ap,sl) ;
	    }
#endif

	    if (sl > 0) {

	        ap += sl ;
	        al += sl ;
	        abl -= sl ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procmsgenv: addrbuf=%t\n",
	            addrbuf,strnlen(addrbuf,al)) ;
	    debugprintf("procmsgenv: addr=%t\n",addr,al) ;
#endif

	    if (pip->open.logfile) {

	    logfile_printf(&pip->lh,"  | %-25s",
	        datebuf) ;

	    logfile_printf(&pip->lh,"  |   %c %t",
	        atypes[atype],addr,MIN(al,(LOGLINEBUFLEN - 8))) ;

	    }

	    ap = sap ;
	    abl = sabl ;

	} /* end for (looping through envelopes) */

	if (i > 0) {

	    sal += (sl + ((f_remote) ? 1 : 0)) ;

	    addrbuf[sal] = '\0' ;
	    mip->efrom = addrbuf ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("procmsgenv: sal=%d addrbuf=%t\n",
	            sal,addrbuf,sal) ;
	    }
#endif

	    if (pip->open.logfile)
	    logfile_printf(&pip->lh,"  > %t",
	        addrbuf,MIN(sal,(LOGLINEBUFLEN - 4))) ;

	    dater_gettime(&mip->edate,&mip->etime) ;

	} /* end if */

	return (i > 0) ? sal : 0 ;
}
/* end subroutine (procmsgenv) */


#ifdef	COMMENT

static process_wenvs(pip,afp,msgp,f_esc)
struct proginfo	*pip ;
bfile		*afp ;
MSG		*msgp ;
int		f_esc ;
{
	MSG_ENV		*envp ;
	DATER		tmpdate ;
	time_t		t ;
	int		rs = SR_OK ;
	int		n = 0 ;
	int		i ;
	char		timebuf[TIMEBUFLEN + 1] ;
	char		*cp ;

	dater_start(&tmpdate,&pip->now,pip->zname,-1) ;

	for (i = 0 ; msg_getenv(msgp,i,&envp) >= 0 ; i += 1) {
	    if (envp == NULL) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("process_wenvs: envelope %d\n",i) ;
	        debugprintf("process_wenvs: from=%s\n",envp->from) ;
	        debugprintf("process_wenvs: date=>%s<\n",envp->date) ;
	        debugprintf("process_wenvs: remote=%s\n",envp->remote) ;
	    }
#endif

	    if ((envp->from == NULL) || (envp->from[0] == '\0'))
	        continue ;

	    n += 1 ;
	    cp = NULL ;
	    if ((envp->date != NULL) && (envp->date[0] != '\0')) {

	        rs = dater_setstd(&tmpdate,envp->date,-1) ;

	        if (rs >= 0)
	            rs = dater_mkstd(&tmpdate,timebuf,TIMEBUFLEN) ;

	        if (rs >= 0)
	            cp = timebuf ;

	    }

	    if (cp == NULL)
	        cp = timestr_edate(pip->now.time,timebuf) ;

	    bprintf(afp,"%sFrom %s %s",
	        ((envp->f_esc || f_esc) ? ">" : ""),
	        envp->from,cp) ;

	    if ((envp->remote != NULL) && (envp->remote[0] != '\0'))
	        bprintf(afp," remote from %s",envp->remote) ;

	    rs = bputc(afp,'\n') ;

	} /* end for (looping through envelopes) */

	dater_finish(&tmpdate) ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (process_wenvs) */

#endif /* COMMENT */


/* write out headers */
static int ph_write(pip,afp,mhp,name,nlen,mode)
struct proginfo	*pip ;
bfile		*afp ;
MSG_HEADER	*mhp ;
char		name[] ;
int		nlen, mode ;
{
	struct msg_instance	*mip ;

	struct msg_line		*mlp ;

	int		rs, line ;
	int		tlen = 0 ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ph_write: entered, name=%s, mode=%d\n",
	        name,mode) ;
#endif

	if (mhp->vlen < 0)
	    return BAD ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ph_write: vlen=%d\n",mhp->vlen) ;
#endif

	if (mode == 0) {

/* write the header the same as we found it (possibly messy) */

	    line = 0 ;
	    mip = mhp->i ;
	    while (mip != NULL) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ph_write: line=%d\n",line) ;
#endif

	        rs = bprintf(afp,"%t: ",
	            name,nlen) ;

		if (rs >= 0)
			tlen += rs ;

	        mlp = mip->line ;
	        while (mlp != NULL) {

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("ph_write: linelen=%d\n",
		mlp->llen) ;
	    debugprintf("ph_write: line=>%t<\n",
		mlp->line,mlp->llen) ;
	    debugprintf("ph_write: f_bol=%d f_eol=%d\n",
		mlp->f_bol,mlp->f_eol) ;
	}
#endif /* CF_DEBUG */

	            if (mlp->llen > 0) {

	                if (line == 0)
	                    rs = bprintf(afp,"%t%s",
	                        mlp->line,mlp->llen,
	                        (mlp->f_eol ? "\n" : "")) ;

	                else
	                    rs = bprintf(afp,"%s%t%s",
	                        (mlp->f_bol ? "\t" : " "),
	                        mlp->line,mlp->llen,
	                        (mlp->f_eol ? "\n" : "")) ;

		if (rs >= 0)
			tlen += rs ;

	                line += 1 ;

	            }

	            mlp = mlp->next ;

	        } /* end while */

	        mip = mip->next ;

	    } /* end while */

	} else if ((mode == 1) && 
	    ((mhp->nlen + mhp->vlen + 2) <= MSG_MAXLINELEN)) {

/* write the header the as a single value string (if it fits) */

	    rs = bprintf(afp,"%t: %t\n",
	        name,nlen,
	        mhp->value,mhp->vlen) ;

		if (rs >= 0)
			tlen += rs ;

	} else if (mode == 2) {

	    int	cl, lenr, vlen ;
	    int	slen ;

	    char	*vp ;
	    char	*cp ;


	    rs = bprintf(afp,"%t:",name,nlen) ;

		if (rs >= 0)
			tlen += rs ;

	    cl = rs ;
	    if (cl < 0)
		cl = 0 ;

	    lenr = MSG_MAXLINELEN - cl ;
	    vp = mhp->value ;
	    vlen = mhp->vlen ;
	    while ((vlen > 0) && ((cl = nextfield(vp,vlen,&cp)) > 0)) {

	        if (cl > lenr) {
	            lenr = (MSG_MAXLINELEN - 7) ;
	            rs = bprintf(afp,"\n\t%t",cp,cl) ;
	        } else
	            rs = bprintf(afp," %t",cp,cl) ;

		if (rs >= 0)
			tlen += rs ;

	        slen = (cp + cl - vp) ;
	        vp += slen ;
	        vlen -= slen ;
	        lenr -= (cl + 1) ;

	    } /* end while */

	    rs = bprintf(afp,"\n") ;

		if (rs >= 0)
			tlen += rs ;

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("ph_write: default\n") ;
#endif

	    mip = mhp->i ;
	    if (mip != NULL) {

	        rs = bprintf(afp,"%t: %t",
	            name,nlen,
	            mip->value,mip->vlen) ;

		if (rs >= 0)
			tlen += rs ;

	        mip = mip->next ;
	    }

	    while (mip != NULL) {

	        rs = bprintf(afp,",\n\t%t",
	            mip->value,mip->vlen) ;

		if (rs >= 0)
			tlen += rs ;

	        mip = mip->next ;

	    } /* end while */

	    rs = bprintf(afp,"\n") ;

		if (rs >= 0)
			tlen += rs ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ph_write: exiting\n") ;
#endif

	return tlen ;
}
/* end subroutine (ph_write) */


/* extract an ID out of an ID-type header (a lot of work for a simple task) */
static int ext_id(pip,s,id)
struct proginfo	*pip ;
char		s[] ;
char		id[] ;
{
	EMA		aid ;

	EMA_ENT	*ep ;

	int	rs, i, sl ;

	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ext_id: entering\n") ;
#endif

	id[0] = '\0' ;
	rs = ema_start(&aid) ;

	if (rs < 0)
	    return rs ;

	if (ema_parse(&aid,s,-1) > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("ext_id: got some EMAs\n") ;
#endif

	    for (i = 0 ; ema_get(&aid,i,&ep) >= 0 ; i += 1) {

	        if (ep == NULL) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("ext_id: looping %d EMAs\n",i) ;
#endif

	        if ((! ep->f.error) && (ep->rlen > 0)) {

	            cp = strwcpy(id,ep->route,MIN(ADDRLEN,ep->rlen)) ;

	            sl = cp - id ;
	            break ;
	        }

	    } /* end for */

	} /* end if */

	ema_finish(&aid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("ext_id: exiting> %s\n",id) ;
#endif

	return (id[0] != '\0') ? sl : SR_NOTFOUND ;
}
/* end subroutine (ext_id) */


/* did we already "do" this header ? */
static int didheader(hp,mhp)
vecstr			*hp ;
struct msg_header	*mhp ;
{
	int	i ;

	char	*cp ;


	for (i = 0 ; vecstr_get(hp,i,&cp) >= 0 ; i += 1) {

	    if (cp == NULL) continue ;

	    if (strcasecmp(cp,mhp->name) == 0)
	        return TRUE ;

	}

	return FALSE ;
}
/* end subroutine (didheader) */


/* strip garbage around items */
static int sfcomment(sp,sl,cpp)
const char	*sp ;
int		sl ;
char		**cpp ;
{


	while ((sl > 0) && ((sp[0] == '"') || (sp[0] == '\''))) {

		sp += 1 ;
		sl -= 1 ;
	}

	while ((sl > 0) && ((sp[sl - 1] == '"') || (sp[sl - 1] == '\'')))
		sl -= 1 ;

	if (cpp != NULL)
		*cpp = (char *) sp ;

	return sl ;
}
/* end subroutine (sfcomment) */


static int cheapspamcheck(pip,buf,buflen)
struct proginfo	*pip ;
const char	buf[] ;
int		buflen ;
{
	int	sl, cl ;

	char	*sp, *cp ;


	sp = (char *) buf ;
	sl = (buflen < 0) ? strlen(buf) : buflen ;

	cp = strnchr(sp,sl,'[') ;

	if (cp == NULL)
		return FALSE ;

	sl = (sp + sl) - (cp + 1) ;
	sp = cp + 1 ;

	cl = sisub(sp,sl,"SPAM") ;
	cp = (sp+cl) ;

	if ((cl < 0) || (cp == NULL))
		return FALSE ;

	sl = (sp + sl) - (cp + 1) ;
	sp = cp + 1 ;

	cp = strnchr(sp,sl,']') ;

	return (cp == NULL) ? FALSE : TRUE ;
}
/* end subroutine (cheapspamcheck) */



