/* buildmsg */

/* part of the 'mkmsg' program */


#define	CF_DEBUG	0
#define	CF_DEBUGS	0


/* revision history:

	= Dave Morano, March 1996

	The program was written from scratch to do what
	the previous program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	buldmsg

	This subroutine builds the message out of the input 
	and the attachments.

	NOTE: According to maybe RFC822 (STD11), headers (if present)
	should appear in the order :

		date
		from
		subject
		to



*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<ema.h>
#include	<mimetypes.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"attach.h"
#include	"config.h"
#include	"defs.h"
#include	"headaddr.h"


/* local defines */

#define	MAXMSGLINELEN	72

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	40
#endif

#define	BASE64LINELEN	72
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

#define	CE_7BIT		0
#define	CE_8BIT		1
#define	CE_BINARY	2
#define	CE_BASE64	3


/* external subroutines */

extern int	pcsmsgid(char *,char *,int) ;

extern char	*strbasename(char *), *strshrink(char *) ;
extern char	*malloc_str(char *), *malloc_strn(char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	outema(struct global *,bfile *,char *,EMA *) ;
static int	outpart(struct global *,bfile *,int,char *,ATTACH_ENT *ep) ;
static int	outct(struct global *,bfile *,ATTACH_ENT *) ;
static int	outbase64(struct global *,bfile *,char *,int) ;

static int	attachentry_code(ATTACH_ENT *,char *) ;
static int	attachentry_analyze(ATTACH_ENT *,char *) ;
static int	is7bit(uint), is8bit(uint), isbinary(uint) ;


/* local variables */

static const char	*encodings[5] = {
	    "7bit",
	    "8bit",
	    "binary",
	    "base64",
	    NULL
} ;


/* exported subroutines */


int buildmsg(gp,adds,mtp,iep,alp,ofp)
struct global	*gp ;
EMA		adds[] ;
MIMETYPES	*mtp ;
ATTACH_ENT	*iep ;
ATTACH		*alp ;
bfile		*ofp ;
{
	ATTACH_ENT	*ep ;

	time_t	daytime ;

	int	rs, i, n = 0 ;
	int	code ;
	int	f_multipart = FALSE ;
	int	f_mime = FALSE ;

	char	msgid[PCS_MSGIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


/* do we need "multipart/mixed" ? */

	if (! gp->f.noinput)
	    n = 1 ;

	if ((rs = attach_count(alp)) > 0)
	    n += rs ;

	if (n > 1)
	    f_mime = f_multipart = TRUE ;


/* find content types for all of our components */

	if (! gp->f.noinput)
	    (void) attachentry_type(iep,mtp) ;

	rs = attach_type(alp,mtp) ;

	if (rs < 0)
	    return rs ;


#if	CF_DEBUG
	if (gp->debuglevel > 1) {

	    debugprintf("buildmsg: types\n") ;

	    if (! gp->f.noinput)
	        debugprintf("buildmsg: type=%s subtype=%s\n",
	            iep->type,iep->subtype) ;

	    for (i = 0 ; attach_enum(alp,i,&ep) >= 0 ; i += 1) {

	        if (ep == NULL) continue ;

	        debugprintf("buildmsg: type=%s subtype=%s\n",
	            ep->type,ep->subtype) ;

	    }
	}
#endif /* CF_DEBUG */


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("buildmsg: input ?\n") ;
#endif

	if (! gp->f.noinput) {

	    code = attachentry_code(iep,gp->tmpdir) ;

	    if (code > CE_7BIT)
	        f_mime = TRUE ;

	}

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("buildmsg: attachments\n") ;
#endif

	for (i = 0 ; attach_enum(alp,i,&ep) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

	    code = attachentry_code(ep,gp->tmpdir) ;

	    if (code > CE_7BIT)
	        f_mime = TRUE ;

	} /* end for */


/* create the message ID */

	(void) pcsmsgid(gp->programroot,msgid,PCS_MSGIDLEN) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("buildmsg: msgid=%s\n",msgid) ;
#endif

/* put out the message ID */

	bprintf(ofp,"message-id: <%s>\n",msgid) ;

	if (f_mime)
	    bprintf(ofp,"MIME-version: %s\n",MIMEVERSION) ;

	if (gp->header_mailer != NULL)
	    bprintf(ofp,"x-mailer: %s\n",gp->header_mailer) ;

	(void) time(&daytime) ;

	bprintf(ofp,"Date: %s\n",
	    timestr_hdate(daytime,timebuf)) ;


/* put out the "from" headers */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("buildmsg: FROM header\n") ;
#endif

	outema(gp,ofp,"From",&adds[A_FROM]) ;

	if (gp->header_subject != NULL)
	    bprintf(ofp,"Subject: %s\n",gp->header_subject) ;


/* put out the "to" headers */

	outema(gp,ofp,"To",&adds[A_TO]) ;


/* put out the "cc" headers */

	outema(gp,ofp,"Cc",&adds[A_CC]) ;


/* put out the "bcc" headers */

	outema(gp,ofp,"Bcc",&adds[A_BCC]) ;


/* do we need mulitpart ? */

	if (f_multipart) {

	    bprintf(ofp,"content-type: multipart/mixed ;\n") ;

	    bprintf(ofp,"\tboundary=\"%s\"\n",msgid) ;

/* put the END-OF-HEADERS mark */

	    bputc(ofp,'\n') ;

	} /* end if (multipart) */


/* if we have multiparts, put out the information note about MIME */

	if (f_multipart) {

	    bprintf(ofp,"This is a MIME formatted mail message.\n") ;

	}


/* do the main input text */

	if (! gp->f.noinput) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("buildmsg: outputting input part\n") ;
#endif

	    outpart(gp,ofp,f_multipart,msgid,iep) ;

	}


/* do the attachments */

	for (i = 0 ; attach_enum(alp,i,&ep) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("buildmsg: doing attachment %d\n",i) ;
#endif

	    outpart(gp,ofp,f_multipart,msgid,ep) ;

	} /* end for */


	if (f_multipart)
	    bprintf(ofp,"--%s--\n",msgid) ;

	return 0 ;
}
/* end subroutine (buildmsg) */


static int outema(gp,ofp,name,ap)
struct global	*gp ;
bfile	*ofp ;
char	name[] ;
EMA	*ap ;
{
	EMA_ENT	*ep ;

	int	rs, i ;
	int	wlen = 0, rlen = MAXMSGLINELEN ;
	int	f_continue = FALSE ;
	int	f_started = FALSE ;


	if ((name == NULL) || (ap == NULL) || (ema_count(ap) <= 0))
	    return SR_OK ;

	rs = bprintf(ofp,"%s: ",name) ;

	if (rs < 0)
	    return rs ;

	rlen -= rs ;
	wlen += rs ;

	for (i = 0 ; ema_get(ap,i,&ep) >= 0 ; i += 1) {

	    if (ep->olen >= rlen) {

	        f_continue = TRUE ;
	        rs = bprintf(ofp,",\n") ;

	        if (rs < 0)
	            return rs ;

	        wlen += rs ;

	        f_started = FALSE ;
	        rs = bprintf(ofp,"\t") ;

	        if (rs < 0)
	            return rs ;

	        rlen = MAXMSGLINELEN - 8 ;
	        wlen += rs ;
	    }

	    if (f_started) {

	        rs = bprintf(ofp,", ") ;

	        if (rs < 0)
	            return rs ;

	        rlen -= rs ;
	        wlen += rs ;
	    }

	    rs = bwrite(ofp,ep->original,ep->olen) ;

	    if (rs < 0)
	        return rs ;

	    rlen -= rs ;
	    wlen += rs ;
	    f_started = TRUE ;

	} /* end for */

	if ((rs = bprintf(ofp,"\n")) < 0)
	    return rs ;

	wlen += rs ;
	return wlen ;
}
/* end subroutine (outema) */


static int outpart(gp,ofp,f_multipart,msgid,ep)
struct global	*gp ;
bfile		*ofp ;
int		f_multipart ;
char		msgid[] ;
ATTACH_ENT	*ep ;
{
	bfile	infile, *ifp = &infile ;

	int	rs, len, wlen = 0 ;
	int	f_base64 = TRUE ;

	char	*fn ;


	fn = (ep->auxfile != NULL) ? fn = ep->auxfile : ep->filename ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("outpart: entered, fn=%s\n",fn) ;
#endif

	if ((fn == NULL) || (fn[0] == '\0') || (fn[0] == '-'))
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(ifp,fn,"r",0666) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("outpart: input open rs=%d code=%d\n",rs,ep->code) ;
#endif

	if (rs >= 0) {

	    char	buf[BASE64BUFLEN + 1] ;


	    if (f_multipart) {

	        rs = bprintf(ofp,
	            "--%s\n",msgid) ;

	        if (rs > 0)
	            wlen += rs ;

	    }

	    rs = outct(gp,ofp,ep) ;

	    if (rs > 0)
	        wlen += rs ;

	    if (ep->code > CE_7BIT) {

	        rs = bprintf(ofp,
	            "content-transfer-encoding: %s\n",
	            ep->encoding) ;

	        if (rs > 0)
	            wlen += rs ;


	        if (ep->clen < 0) {

	            struct ustat	sb ;


	            if ((bcontrol(ifp,BC_STAT,&sb) >= 0) &&
	                S_ISREG(sb.st_mode))
	                ep->clen = (int) sb.st_size ;

	        }

	        if ((ep->clen >= 0) && (ep->code == CE_BINARY)) {

	            rs = bprintf(ofp,
	                "content-length: %d\n",
	                ep->clen) ;

	            if (rs > 0)
	                wlen += rs ;

	        }

	    } /* end if (not '7bit') */

/* end-of-headers */

	    rs = bputc(ofp,'\n') ;

	    if (rs > 0)
	        wlen += rs ;

	    if ((ep->code == CE_BASE64) || (ep->code < 0)) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("outpart: doing BASE64\n") ;
#endif

	        while ((len = bread(ifp,buf,BASE64BUFLEN)) > 0)
	            rs = outbase64(gp,ofp,buf,len) ;

	    } else
	        rs = bcopyblock(ifp,ofp,-1) ;

	    if (rs > 0)
	        wlen += rs ;

	    bclose(ifp) ;

	} /* end if (opened part file) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("outpart: exiting, rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return ((rs >= 0) ? wlen : rs) ;
}
/* end subroutine (outpart) */


/* write out in BASE64 ! */
static int outbase64(gp,ofp,buf,buflen)
struct global	*gp ;
bfile	*ofp ;
char	buf[] ;
int	buflen ;
{
	int	rs = SR_OK , i = 0 ;
	int	rlen = buflen ;
	int	wlen = 0 ;
	int	mlen, len ;

	char	outbuf[BASE64LINELEN + 4] ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("outbase64: entered buflen=%d\n",buflen) ;
#endif

	while (rlen > 0) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("outbase64: rlen=%d\n",rlen) ;
#endif

	    mlen = MIN(BASE64BUFLEN,rlen) ;
	    len = base64_e(buf + i,mlen,outbuf) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1) {

	        debugprintf("outbase64: mlen=%d base64_e-len=%d\n",
	            mlen,len) ;

	        debugprintf("outbase64: >%W<\n",outbuf,len) ;

	    }
#endif

	    rs = bwrite(ofp,outbuf,len) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("outbase64: bwrite rs=%d\n",rs) ;
#endif

	    wlen += rs ;
	    if (rs < 0)
	        break ;

	    rs = bputc(ofp,'\n') ;

	    wlen += rs ;
	    if (rs < 0)
	        break ;

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("outbase64: exiting rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return ((rs < 0) ? rs : wlen) ;
}
/* end subroutine (outbase64) */


static int outct(gp,ofp,ep)
struct global	*gp ;
bfile		*ofp ;
ATTACH_ENT	*ep ;
{
	int	rs, wlen = 0 ;
	int	code = ep->code ;


	if (ep->description != NULL)
	    bprintf(ofp,"content-description: %s\n",ep->description) ;

	rs = bprintf(ofp,
	    "content-type: %s",ep->type) ;

	if (rs > 0)
	    wlen += rs ;

	if ((ep->subtype != NULL) && (ep->subtype[0] != '\0') &&
	    (code != CE_7BIT)) {

	    rs = bprintf(ofp,
	        "/%s",ep->subtype) ;

	    if (rs > 0)
	        wlen += rs ;

	}

	if ((strcmp(ep->type,"text") == 0) && (code > CE_7BIT) &&
	    ((ep->subtype == NULL) || (strcmp(ep->subtype,"plain") == 0)))
	    rs = bprintf(ofp," ; charset=iso-8859-1") ;

	if ((ep->filename != NULL) && (ep->filename[0] != '-') &&
	    (ep->filename[0] != '\0'))
	    rs = bprintf(ofp," ;\n\tname=\"%s\"",ep->filename) ;

	if (rs > 0)
	    wlen += rs ;

	rs = bputc(ofp,'\n') ;

	if (rs > 0)
	    wlen += rs ;

	return ((rs >= 0) ? wlen : rs) ;
}
/* end subroutine (outct) */


static int attachentry_code(ep,tmpdir)
ATTACH_ENT	*ep ;
char		tmpdir[] ;
{
	int	f_plaintext = FALSE ;

	int	code = -1 ;


#if	CF_DEBUGS
	debugprintf("attachentry_code: entered, file=%s type=%s\n",
	    ep->filename,ep->type) ;
#endif

	if (ep->type != NULL) {

	    if ((strcasecmp(ep->type,"text") == 0) &&
	        ((ep->subtype == NULL) || 
	        (strcasecmp(ep->subtype,"plain") == 0)))
	        f_plaintext = TRUE ;

	} else
	    f_plaintext = TRUE ;

	if (! f_plaintext)
	    code = CE_BASE64 ;

#if	CF_DEBUGS
	debugprintf("attachentry_code: f_plaintext=%d code=%d\n",
	    f_plaintext,code) ;
#endif

	if (ep->encoding == NULL) {

	    if (f_plaintext) {

	        code = attachentry_analyze(ep,tmpdir) ;

	        if (code >= CE_7BIT) {

	            ep->encoding = malloc_str(encodings[code]) ;

	        } /* end if */

	    } else
	        ep->encoding = malloc_str("base64") ;

	} /* end if (encoding determination) */

	if (code >= 0)
	    ep->code = code ;

#if	CF_DEBUGS
	debugprintf("attachentry_code: code=%d encoding=%s\n",
	    code,ep->encoding) ;
#endif

	return code ;
}
/* end subroutine (attachentry_code) */


int attachentry_analyze(ep,tmpdir)
ATTACH_ENT	*ep ;
char		tmpdir[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	auxfile, *afp = &auxfile ;

	struct ustat	sb ;

	int	f_needaux = FALSE ;
	int	clen = 0 ;
	int	rs, i ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	auxfname[MAXPATHLEN + 1] ;


	if ((ep->filename == NULL) || (ep->filename[0] == '\0') ||
	    (ep->filename[0] == '-'))
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(ifp,ep->filename,"r",0666) ;

	if (rs < 0)
	    return rs ;

	rs = bcontrol(ifp,BC_STAT,&sb) ;

	f_needaux = TRUE ;
	if ((rs >= 0) && S_ISREG(sb.st_mode)) {

	    ep->clen = (int) sb.st_size ;
	    f_needaux = FALSE ;

	}

#if	CF_DEBUGS
	debugprintf("attachentry_analyze: needaux=%d\n",f_needaux) ;
#endif

	if (f_needaux) {

	    (void) bufprintf(tmpfname,MAXPATHLEN,"%s/mksmgXXXXXXXXX",
	        tmpdir) ;

	    rs = mktmpfile(tmpfname,0660,auxfname) ;

#if	CF_DEBUGS
	    debugprintf("attachentry_analyze: mktmpfile rs=%d tmpfname=%s\n",
	        rs,tmpfname) ;
#endif

	    if (rs < 0)
	        goto bad1 ;

	    if ((ep->auxfile = malloc_str(auxfname)) == NULL)
	        goto bad1 ;

	    rs = bopen(afp,ep->auxfile,"wct",0666) ;

	}

	if (rs >= 0) {

	    int	code = 0 ;
	    int	len ;

	    char	buf[BUFLEN + 1] ;


#if	CF_DEBUGS
	    debugprintf("attachentry_analyze: looping through file\n") ;
#endif

	    while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	        if (code < 2) {

	            for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUGS
	                debugprintf("attachentry_analyze: c=%02X (%c)\n",
	                    buf[i],
	                    buf[i]) ;
#endif
	                if (isbinary(buf[i])) break ;

	                if ((code < 1) && is8bit(buf[i])) {

#if	CF_DEBUGS
	                    debugprintf("attachentry_analyze: got a 8bit c=%02X\n",buf[i]) ;
#endif

	                    code = 1 ;

	                }

	            } /* end for */

	            if ((i < len) && (isbinary(buf[i]))) {

#if	CF_DEBUGS
	                debugprintf("attachentry_analyze: got a binary c=%02X\n",buf[i]) ;
#endif

	                code = 2 ;

	            }
	        } /* end if */

	        if (! f_needaux) {

	            if (code == 2)
	                break ;

	        } else
	            (void) bwrite(afp,buf,len) ;

	        clen += len ;

	    } /* end while */

	    rs = code ;

	} /* end if */

	if (f_needaux) {

	    if (ep->clen < 0)
	        ep->clen = clen ;

	    bclose(afp) ;

	} else
	    bseek(ifp,0L,SEEK_SET) ;

	bclose(ifp) ;

#if	CF_DEBUGS
	debugprintf("attachentry_analyze: exiting OK rs=%d\n",rs) ;
#endif

	return rs ;

bad1:
	bclose(ifp) ;

#if	CF_DEBUGS
	debugprintf("attachentry_analyze: exiting BAD rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (attachentry_analyze) */


static int is7bit(c)
uint	c ;
{


	return (isascii(c) && isprint(c)) ;
}


static int is8bit(c)
uint	c ;
{


	return ((c & 0x80) && ((c & (~ 31)) != 0x80)) ;
}


static int isbinary(c)
uint	c ;
{


	if (isspace(c))
	    return FALSE ;

	return (((c & (~ 31)) == 0x00) || ((c & (~ 31)) == 0x80)) ;
}


