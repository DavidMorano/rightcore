/* progbuildmsg */

/* part of the MKMSG program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_FORCEINPUT	0		/* ?? */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine builds the message out of the input and the attachments.

        NOTE: According to maybe RFC822 (STD11), headers (if present) should
        appear in the order:

		date
		from
		subject
		to

	I don't know if we are caring a rat's butt about this below!

	As an aside, personally I always preferred the order:

		from
		to
		cc
		bcc
		date
		subject


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<sbuf.h>
#include	<ema.h>
#include	<mimetypes.h>
#include	<paramopt.h>
#include	<pcsconf.h>		/* for 'PCS_MSGIDLEN' */
#include	<dater.h>
#include	<localmisc.h>

#include	"mailmsgatt.h"
#include	"mailmsgattent.h"
#include	"config.h"
#include	"defs.h"
#include	"ha.h"
#include	"contentencodings.h"


/* local defines */

#ifndef	MSGBOUNDLEN
#define	MSGBOUNDLEN	70		/* RFC-2046 requirement */
#endif

#ifndef	FACEBUFLEN
#define	FACEBUFLEN	(4 * 80)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;

extern int	progouthead(PROGINFO *,bfile *,cchar *,cchar *,int) ;
extern int	progoutheadema(PROGINFO *,bfile *,const char *,EMA *) ;
extern int	progoutpart(PROGINFO *,bfile *,int,cchar *,MAILMSGATTENT *) ;

extern int	mkmsgbound(PROGINFO *,char *,int) ;
extern int	mkface(PROGINFO *,char *,int) ;
extern int	outhead(PROGINFO *,bfile *,const char *,cchar *,int) ;
extern int	outheadema(PROGINFO *,bfile *,char *,EMA *) ;
extern int	outpart(PROGINFO *,bfile *,int,char *,MAILMSGATTENT *ep) ;
extern int	tolc(int) ;

#ifdef	COMMENT
extern int	outct(PROGINFO *,bfile *,MAILMSGATTENT *) ;
extern int	outbase64(PROGINFO *,bfile *,char *,int) ;
#endif /* COMMENT */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_hdate(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	progbuildmsghdrs(PROGINFO *,EMA *,PARAMOPT *,int) ;
static int	progbuildmsgothers(PROGINFO *,PARAMOPT *,int) ;
static int	printct(PROGINFO *,cchar *,cchar *,cchar *) ;
static int	ishigh(int) ;


/* local variables */


/* exported subroutines */


int progbuildmsg(pip,adds,hlp,mtp,iep,alp)
PROGINFO	*pip ;
EMA		adds[] ;
PARAMOPT	*hlp ;
MIMETYPES	*mtp ;
MAILMSGATTENT	*iep ;
MAILMSGATT	*alp ;
{
	MAILMSGATTENT	*ep ;
	int		rs, rs1 ;
	int		n = 0 ;
	int		i ;
	int		code = 0 ;
	int		wlen = 0 ;
	int		f_multipart = FALSE ;
	int		f_mime = pip->f.mime ;
	const char	*ccp ;
	char		msgbound[MSGBOUNDLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: ent f_mime=%u\n",f_mime) ;
#endif

/* do we need "multipart/mixed"? */

	if (! pip->f.noinput) n = 1 ;
	rs = mailmsgatt_count(alp) ;
	n += rs ;

	if (n > 1) {
	    f_mime = TRUE ;
	    f_multipart = TRUE ;
	}

/* find content types for all of our components */

/* type the input part */

	if ((rs >= 0) && (! pip->f.noinput)) {
	    int	f_def = FALSE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("progbuildmsg: input 1 ct=%s\n",iep->type) ;
	        debugprintf("progbuildmsg: input 1 ext=%s\n",iep->ext) ;
	    }
#endif

	    rs = mailmsgattent_type(iep,mtp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("progbuildmsg: mailmsgattent_type() rs=%d\n",rs) ;
	        debugprintf("progbuildmsg: f_pt=%u\n",iep->f_plaintext) ;
	        debugprintf("progbuildmsg: ct=%s\n",iep->type) ;
	        debugprintf("progbuildmsg: cte=%d\n",iep->cte) ;
	    }
#endif

	    f_def = f_def || (iep->type == NULL) ;
#if	CF_FORCEINPUT
	    f_def = f_def || (strcasecmp(iep->type,"binary") == 0) ;
#endif /* CF_FORCEINPUT */

	    if ((rs >= 0) && f_def) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progbuildmsg: forcing CT to text\n") ;
#endif
	        rs = mailmsgattent_typeset(iep,"text","plain") ;
	    } /* end if (forced typing) */

	} /* end if (typing the input) */

/* type all of the other parts */

	if (rs >= 0) {
	    rs = mailmsgatt_typeatts(alp,mtp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: mailmsgatt_typeatts() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progbuildmsg: types¬\n") ;
	    if (! pip->f.noinput) {
	        debugprintf("progbuildmsg: type=%s subtype=%s\n",
	            iep->type,iep->subtype) ;
	    }
	    for (i = 0 ; mailmsgatt_enum(alp,i,&ep) >= 0 ; i += 1) {
	        if (ep == NULL) continue ;
	        debugprintf("progbuildmsg: type=%s subtype=%s\n",
	            ep->type,ep->subtype) ;
	    }
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: input?\n") ;
#endif

	if ((rs >= 0) && (! pip->f.noinput)) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progbuildmsg: coding input!\n") ;
#endif
	    rs = mailmsgattent_code(iep,pip->jobdname) ;
	    code = rs ;
	    if (code >= CE_7BIT) f_mime = TRUE ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("progbuildmsg: _code() rs=%d\n",rs) ;
	        debugprintf("progbuildmsg: f_mime=%u\n",f_mime) ;
	    }
#endif /* CF_DEBUG */
	    if ((rs >= 0) && f_multipart && (code == 0)) {
		if ((rs = mailmsgattent_isplaintext(iep)) > 0) {
	    	    rs = mailmsgattent_setcode(iep,CE_7BIT) ;
		}
	    }
	} /* end if (had input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: coding attachments\n") ;
#endif

	if (rs >= 0) {
	    for (i = 0 ; (rs1 = mailmsgatt_enum(alp,i,&ep)) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs = mailmsgattent_code(ep,pip->jobdname) ;
	            code = rs ;
	            if (code >= CE_7BIT) f_mime = TRUE ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	} /* end if (ok) */

/* now we put out most of the message headers */

	if (rs >= 0) {
	    rs = progbuildmsghdrs(pip,adds,hlp,f_mime) ;
	    wlen += rs ;
	}

/* are we mulitpart? */

	if ((rs >= 0) && f_multipart) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progbuildmsg: multipart\n") ;
#endif

	    if (rs >= 0) {
	        rs = mkmsgbound(pip,msgbound,MSGBOUNDLEN) ;
	    } /* end if (multipart message-boundary) */

	    if (rs >= 0) {
	        cchar	*kn = "content-type" ;
	        cchar	*val = "multipart/mixed" ;
		rs = printct(pip,kn,val,msgbound) ;
		wlen += rs ;
	    }

/* put the END-OF-HEADERS mark */

	    if (rs >= 0) {
	        rs = bputc(pip->ofp,'\n') ;
	        wlen += rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progbuildmsg: disclaimer\n") ;
#endif

	    if (rs >= 0) {
	        const char	*dis = pip->disclaimer ;

	        if ((dis != NULL) && (dis[0] != '\0')) {
	            bfile	dfile ;
	            if ((rs = bopen(&dfile,dis,"r",0666)) >= 0) {
	                rs = bcopyblock(&dfile,pip->ofp,-1) ;
	                wlen += rs ;
	                rs1 = bclose(&dfile) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (file) */
	        } else {
	            ccp = "This is a MIME formatted mail message.\n" ;
	            rs = bprintf(pip->ofp,ccp) ;
	            wlen += rs ;
	        }

	    } /* end if (disclaimer) */

	} /* end if (MIME multipart needed) */

/* do the main input text */

	if ((rs >= 0) && (! pip->f.noinput)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("progbuildmsg: outputting input part\n") ;
	        debugprintf("progbuildmsg: type=%s\n",iep->type) ;
	    }
#endif

	    rs = progoutpart(pip,pip->ofp,f_multipart,msgbound,iep) ;
	    wlen += rs ;

	} /* end if */

/* do the attachments */

	if (rs >= 0) {
	    for (i = 0 ; mailmsgatt_enum(alp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs = progoutpart(pip,pip->ofp,f_multipart,msgbound,ep) ;
	            wlen += rs ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

/* finish up and get out! */

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progbuildmsg: finish-up\n") ;
#endif

	    if (f_multipart) {
	        rs = bprintf(pip->ofp,"--%s--\n",msgbound) ;
	        wlen += rs ;
	    } else {
	        if (pip->f.noinput || (n <= 0)) {
	            rs = bputc(pip->ofp,'\n') ;
	            wlen += rs ;
	        }
	    } /* end if */

	} /* end if (final marker) */

	pip->serial += 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progbuildmsg) */


/* local subroutines */


static int progbuildmsghdrs(PROGINFO *pip,EMA *adds,PARAMOPT *hlp,int f_mime)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_date = FALSE ;
	cchar		*kn ;
	char		timebuf[TIMEBUFLEN + 1] ;

/* header-MESSAGEID */

	if ((rs >= 0) && (pip->hdr_mid != NULL)) {
	    kn = "message-id" ;
	    rs = bprintf(pip->ofp,"%s: <%s>\n",kn,pip->hdr_mid,-1) ;
	    wlen += rs ;
	} /* end if (header-MESSAGEID) */

/* header-MIMEVERSION */

	if ((rs >= 0) && f_mime) {
	    kn = "MIME-version" ;
	    rs = bprintf(pip->ofp,"%s: %s\n",kn,MIMEVERSION) ;
	    wlen += rs ;
	} /* end if (header-MIMEVERSION) */

/* header-XMAILER */

	if ((rs >= 0) && (pip->hdr_mailer != NULL)) {
	    kn = "x-mailer" ;
	    rs = progouthead(pip,pip->ofp,kn,pip->hdr_mailer,-1) ;
	    wlen += rs ;
	} /* end if (header-XMAILER) */

/* header-FACE */

	if ((rs >= 0) && pip->f.add_face) {
	    const int	flen = FACEBUFLEN ;
	    char	fbuf[FACEBUFLEN + 1] ;
	    fbuf[0] = '\0' ;
	    if ((rs = mkface(pip,fbuf,flen)) > 0) { /* positive */
	        if (fbuf[0] != '\0') {
		    kn = "x-face" ;
	            rs = progouthead(pip,pip->ofp,kn,fbuf,rs) ;
	            wlen += rs ;
		}
	    }
	} /* end if (header-FACE) */

/* header-ORGANIZATION */

	if ((rs >= 0) && (pip->org != NULL)) {
	    if (pip->have.h_org && pip->f.h_org) {
	        int	ol = strlen(pip->org) ;
	        cchar	*op = pip->org ;
	        kn = "organization" ;
	        while (ol && ishigh(op[ol-1])) ol -= 1 ;
	        if (ol > 0) {
	            rs = progouthead(pip,pip->ofp,kn,op,ol) ;
	            wlen += rs ;
	        }
	    }
	} /* end if (header-ORGANIZATION) */

/* header-SENDER */

	if ((rs >= 0) && pip->have.h_sender && pip->f.h_sender) {
	    kn = "sender" ;
	    rs = progoutheadema(pip,pip->ofp,kn,(adds+ha_sender)) ;
	    wlen += rs ;
	} /* end if (header-SENDER) */

/* header-REPLYTO */

	if ((rs >= 0) && pip->have.h_replyto && pip->f.h_replyto) {
	    kn = "reply-to" ;
	    rs = progoutheadema(pip,pip->ofp,kn,(adds+ha_replyto)) ;
	    wlen += rs ;
	} /* end if (header-REPLYTO) */

/* header-DATE (RFC822 & STD11) */

	if ((rs >= 0) && pip->f.mdate) {
	    kn = "date" ;
	    f_date = TRUE ;
	    dater_mkmsg(&pip->mdate,timebuf,-1) ;
	    rs = bprintf(pip->ofp,"%s: %s\n",kn,timebuf) ;
	    wlen += rs ;
	} else if (paramopt_havekey(hlp,"date") < 0) {
	    f_date = TRUE ;
	    if (pip->daytime == 0) pip->daytime = time(NULL) ;
	    timestr_hdate(pip->daytime,timebuf) ;
	    rs = bprintf(pip->ofp,"Date: %s\n",timebuf) ;
	    wlen += rs ;
	} /* end if (date handling) */

/* header-FROM */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: FROM header\n") ;
#endif

	if (rs >= 0) {
	    kn = "From" ;
	    rs = progoutheadema(pip,pip->ofp,kn,(adds+ha_from)) ;
	    wlen += rs ;
	}

/* the SUBJECT header (if we have one) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: SUBJECT header\n") ;
#endif

	if ((rs >= 0) && (pip->hdr_subject != NULL)) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(pip->hdr_subject,-1,&cp)) > 0) {
		kn = "Subject" ;
	        rs = progouthead(pip,pip->ofp,kn,cp,cl) ;
	        wlen += rs ;
	    }
	} /* end if (header-SUBJECT) */

/* put out the TO headers */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: TO header\n") ;
#endif

	if (rs >= 0) {
	    kn = "To" ;
	    rs = progoutheadema(pip,pip->ofp,kn,(adds+ha_to)) ;
	    wlen += rs ;
	}

/* put out the CC headers */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: CC header\n") ;
#endif

	if (rs >= 0) {
	    kn = "Cc" ;
	    rs = progoutheadema(pip,pip->ofp,kn,(adds+ha_cc)) ;
	    wlen += rs ;
	}

/* put out the BCC headers */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: BCC header\n") ;
#endif

	if (rs >= 0) {
	    kn = "Bcc" ;
	    rs = progoutheadema(pip,pip->ofp,"Bcc",(adds+ha_bcc)) ;
	    wlen += rs ;
	}

/* put out any headers that were specified explicitly */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progbuildmsg: other headers\n") ;
#endif

	if (rs >= 0) {
	    rs = progbuildmsgothers(pip,hlp,f_date) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progbuildmsghdrs) */


static int progbuildmsgothers(PROGINFO *pip,PARAMOPT *hlp,int f_date)
{
	PARAMOPT_CUR	kcur, vcur ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progbuildmsgothers: f_date=%u\n",f_date) ;
#endif

	if ((rs = paramopt_curbegin(hlp,&kcur)) >= 0) {
	    int		kl ;
	    int		vl ;
	    cchar	*kp, *vp ;

	    while (rs >= 0) {
	        kl = paramopt_enumkeys(hlp,&kcur,&kp) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs <= 0) break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progbuildmsgothers: k=>%t<\n",kp,kl) ;
#endif

	        if ((rs = paramopt_curbegin(hlp,&vcur)) >= 0) {

	            while (rs >= 0) {
	                vl = paramopt_enumvalues(hlp,kp,&vcur,&vp) ;
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progbuildmsgothers: rs=%d v=>%t<\n",
				rs,vp,vl) ;
#endif

	                if (rs >= 0) {
			    const int	kc = MKCHAR(kp[0]) ;
	                    if ((tolc(kc) != 'd') ||
	                        (strcasecmp(kp,"date") != 0) ||
	                        (! f_date)) {

	                        rs = progouthead(pip,pip->ofp,kp,vp,vl) ;
	                        wlen += rs ;

	                    } /* end if */
	                } /* end if (ok) */

	            } /* end while */

	            paramopt_curend(hlp,&vcur) ;
	        } /* end if (cursor) */

	    } /* end while */

	    paramopt_curend(hlp,&kcur) ;
	} /* end if (cursor) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progbuildmsgothers: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progbuildmsgothers) */


static int printct(PROGINFO *pip,cchar *kn,cchar *val,cchar *msgbound)
{
	int		rs ;
	int		wlen = 0 ;
	if ((rs = bprintf(pip->ofp,"%s:\n",kn)) >= 0) {
	    wlen += rs ;
	    if ((rs = bprintf(pip->ofp," %s;\n",val)) >= 0) {
	        wlen += rs ;
	        rs = bprintf(pip->ofp," boundary=\"%s\"\n",msgbound) ;
	        wlen += rs ;
	    }
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printct) */


static int ishigh(int ch)
{
	return (ch & 0x80) ;
}
/* end subroutine (ishigh) */


