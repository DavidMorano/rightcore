/* out */

/* support building a message without output related subroutines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_PLAIN	1		/* force "plain" typed text */
#define	CF_OUTVALUE	1		/* use 'outvalue()' */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was written from scratch but based on previous
	versions of the 'mkmsg' program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Output a header.

	int outhead(pip,ofp,name,v,vlen)
	struct proginfo	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	const char	v[] ;
	int		vlen ;


	The 'outheadema' subroutine is used to print out (write to the
	mail message file being built) an E-Mail Address (EMA).

	int outheadema(pip,ofp,name,ap)
	struct proginfo	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	EMA		*ap ;


	The 'outpart' subroutine is used to print out a email body part.

	int outpart(pip,ofp,f_multipart,msgboundary,ep)
	struct proginf	*pip ;
	bfile		*ofp ;
	int		f_multipart ;
	char		msgboundary[] ;
	MSGATTACH_ENT	*ep ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ema.h>
#include	<base64.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"msgattach.h"


/* local defines */

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	BASE64LINELEN	72
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

/* types of "content encodings" */
#define	CE_7BIT		0
#define	CE_8BIT		1
#define	CE_BINARY	2
#define	CE_BASE64	3

#ifndef	FROM_ESCAPE
#define	FROM_ESCAPE	'\b'
#endif


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */

struct outline {
	int	maxlen ;
	int	rlen ;
} ;


/* forward references */

static int	outct(struct proginfo *,bfile *,MSGATTACH_ENT *) ;
static int	outbase64(struct proginfo *,bfile *,char *,int) ;
static int	outvalue(struct proginfo *,bfile *,struct outline *,
			const char *,int) ;

static int	outline_start(struct outline *,int,int) ;
static int	outline_finish(struct outline *) ;


/* local variables */


/* exported subroutines */


/* output a general header */
int outhead(pip,ofp,name,v,vlen)
struct proginfo	*pip ;
bfile		*ofp ;
const char	name[] ;
const char	v[] ;
int		vlen ;
{
	struct outline	ld ;

	int	rs ;
	int	wlen = 0 ;


	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

	if (vlen < 0)
	    vlen = strlen(v) ;

	outline_start(&ld, MAXMSGLINELEN, MAXMSGLINELEN) ;

	rs = bprintf(ofp,"%s: ",name) ;
	wlen += rs ;
	ld.rlen -= rs ;

	if (rs >= 0)
	    rs = outvalue(pip,ofp,&ld,v,vlen) ;

	wlen += rs ;
	if (rs >= 0)
	    rs = bprintf(ofp,"\n") ;

	wlen += rs ;
	outline_finish(&ld) ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outhead) */


/* output a header that comtains one or more EMAs */
int outheadema(pip,ofp,name,ap)
struct proginfo	*pip ;
bfile		*ofp ;
const char	name[] ;
EMA		*ap ;
{
	EMA_ENT	*ep ;

	struct outline	ld ;

	int	rs ;
	int	i, n, c ;
	int	nlen ;
	int	wlen = 0 ;
	int	f_linestart = FALSE ;

	const char	*fmt ;


	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

	if ((ap == NULL) || (ema_count(ap) <= 0))
	    return SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("out/outheadema: head=%s\n",name) ;
#endif

	rs = outline_start(&ld, MAXMSGLINELEN, MAXMSGLINELEN) ;

	if (rs < 0)
		goto ret0 ;

	rs = bprintf(ofp,"%s: ",name) ;
	wlen += rs ;
	ld.rlen -= rs ;

	n = ema_count(ap) ;

	c = 0 ;
	for (i = 0 ; (rs >= 0) && (ema_get(ap,i,&ep) >= 0) ; i += 1) {

	    if (ep == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("out/outheadema: EMA=>%w<\n",
	            ep->original,ep->olen) ;
#endif

/* calculate how much space (columns) that we need for this EMA */

	    nlen = (f_linestart) ? (ep->olen + 2) : ep->olen ;
	    if ((c + 1) < n)
	        nlen += 1 ;

/* see if it will fit in the remaining available line space */

	    if (nlen > ld.rlen) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("out/outheadema: new line\n") ;
#endif

	        fmt = (f_linestart) ? ",\n " : "\n " ;
	        rs = bwrite(ofp,fmt,strlen(fmt)) ;
	        wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("out/outheadema: bwrite() rs=%d\n",rs) ;
#endif

	        f_linestart = FALSE ;
	        ld.rlen = ld.maxlen - 1 ;
	    }

	    if (f_linestart) {

	        if (rs >= 0) {
	            rs = bwrite(ofp,", ",2) ;
	            wlen += rs ;
		}

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("out/outheadema: bwrite() rs=%d\n",rs) ;
#endif

	        ld.rlen -= rs ;
	    }

	    if (rs >= 0) {

#if	CF_OUTVALUE
	        rs = outvalue(pip,ofp,&ld,ep->original,ep->olen) ;
#else
	        rs = bwrite(ofp,ep->original,ep->olen) ;

	        ld.rlen -= rs ;
#endif /* CF_OUTVALUE */

	        wlen += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("out/outheadema: wrote rs=%d\n",rs) ;
#endif

	    }

	    f_linestart = TRUE ;
	    c += 1 ;

	} /* end for */

	if (rs >= 0) {
	    rs = bprintf(ofp,"\n") ;
	    wlen += rs ;
	}

	outline_finish(&ld) ;

ret0:

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("out/outheadema: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outheadema) */


/* put out a mail body part */
int outpart(pip,ofp,f_multipart,msgboundary,ep)
struct proginfo	*pip ;
bfile		*ofp ;
int		f_multipart ;
char		msgboundary[] ;
MSGATTACH_ENT	*ep ;
{
	bfile	infile, *ifp = &infile ;

	int	rs ;
	int	len ;
	int	wlen = 0 ;
	int	encoding ;
	int	f_fname ;
	int	f_encoding ;

	char	*fn ;


	fn = (ep->auxfname != NULL) ? ep->auxfname : ep->filename ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("outpart: entered fn=%s\n",fn) ;
#endif

	if ((fn == NULL) || (fn[0] == '\0') || (fn[0] == '-'))
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(ifp,fn,"r",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("outpart: input open rs=%d code=%d\n",rs,ep->code) ;
	    debugprintf("outpart: encoding=%s\n",ep->encoding) ;
	}
#endif

	if (rs >= 0) {
	    char	buf[BASE64BUFLEN + 1] ;

	    if (f_multipart) {
	        rs = bprintf(ofp, "--%s\n",msgboundary) ;
	        wlen += rs ;
	    }

/* content description */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("outpart: content description\n") ;
#endif

	    if ((rs >= 0) && (ep->description != NULL)) {
	        rs = bprintf(ofp,"content-description: %s\n",ep->description) ;
	        wlen += rs ;
	    }

/* content type */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("outpart: output content type ?\n") ;
	        debugprintf("outpart: ct=%s\n",ep->type) ;
	    }
#endif

	    f_encoding = FALSE ;
	    f_fname = TRUE ;
	    f_fname = f_fname && (ep->filename != NULL) ;
	    f_fname = f_fname && (ep->filename[0] != '\0') ;
	    f_fname = f_fname && (ep->filename[0] != '-') ;
	    f_fname = f_fname && 
	        (strncmp(ep->filename,"/dev/fd/",8) != 0) ;

	    if ((rs >= 0) && (f_multipart || f_fname ||
	        (strcmp(ep->type,"text") != 0) || (ep->code > CE_7BIT) ||
	        ((ep->subtype != NULL) && 
	        (strcmp(ep->subtype,"plain") != 0)))) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("outpart: content typing!\n") ;
#endif

	        f_encoding = TRUE ;
	        rs = outct(pip,ofp,ep) ;
	        wlen += rs ;

	    } /* end if (content type) */

/* content lines */

	    if ((rs >= 0) && (strcmp(ep->type,"text") == 0) &&
	        ((ep->subtype == NULL) || 
	        (strcmp(ep->subtype,"plain") == 0))) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("outpart: we want lines if we have them!\n") ;
#endif

	        if (ep->lines >= 0) {
	            rs = bprintf(ofp,"content-lines: %d\n",ep->lines) ;
	            wlen += rs ;
	        }

	    } /* end if (content lines) */

/* content transfer encoding */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("outpart: encoding=%s code=%d\n",
	            ep->encoding,ep->code) ;
#endif

	    encoding = ep->code ;
	    if ((rs >= 0) && (((ep->encoding != NULL) && f_encoding) ||
	        (ep->code > CE_7BIT))) {

	        rs = bprintf(ofp,
	            "content-transfer-encoding: %s\n",
	            ep->encoding) ;

	        wlen += rs ;
	        if ((strcmp(ep->encoding,"binary") == 0) ||
	            (ep->code == CE_BINARY)) {

	            if (ep->clen < 0) {
	                struct ustat	sb ;

	                if ((bcontrol(ifp,BC_STAT,&sb) >= 0) &&
	                    S_ISREG(sb.st_mode))
	                    ep->clen = (int) sb.st_size ;

	            }

	            if ((rs >= 0) && (ep->clen >= 0)) {
	                rs = bprintf(ofp,"content-length: %u\n",ep->clen) ;
	                wlen += rs ;
	            }

	        } /* end if (binary content) */

	    } /* end if (not '7bit') */

/* end-of-headers */

	    if (rs >= 0)
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

/* message part body */

	    if (rs >= 0) {

	        if (encoding == CE_BASE64) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("outpart: doing BASE64\n") ;
#endif

	            while ((rs = bread(ifp,buf,BASE64BUFLEN)) > 0) {

	                len = rs ;
	                rs = outbase64(pip,ofp,buf,len) ;
	                wlen += rs ;

	                if (rs < 0) break ;
	            } /* end while */

	        } else if ((encoding >= CE_7BIT) && 
	            (encoding < CE_BINARY)) {

	            int	f_bol, f_eol ;


#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("outpart: thinking about doing it!\n") ;
#endif

	            f_bol = TRUE ;
	            f_eol = FALSE ;
	            while ((rs = breadline(ifp,buf,BASE64BUFLEN)) > 0) {
	                len = rs ;

	                f_eol = (buf[len - 1] == '\n') ;
	                if (f_bol && (strncmp(buf,"From ",5) == 0)) {

	                    rs = bputc(ofp,FROM_ESCAPE) ;
	                    wlen += rs ;

	                } /* end if ("From" escape for dumb mailers!) */

	                if (rs >= 0) {
	                    rs = bwrite(ofp,buf,len) ;
	                    wlen += rs ;
			}

	                f_bol = f_eol ;
	                if (rs < 0) break ;
	            } /* end while */

	        } else {

	            rs = bcopyblock(ifp,ofp,-1) ;
	            wlen += rs ;

	        } /* end if */

	    } /* end if */

	    bclose(ifp) ;
	} /* end if (opened part file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("outpart: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpart) */


/* local subroutines */


/* write out in BASE64! */
static int outbase64(pip,ofp,buf,buflen)
struct proginfo	*pip ;
bfile		*ofp ;
char		buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	i = 0 ;
	int	rlen ;
	int	mlen, len ;
	int	wlen = 0 ;

	char	outbuf[BASE64LINELEN + 4] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("outbase64: entered buflen=%d\n",buflen) ;
#endif

	rlen = buflen ;
	while ((rs >= 0) && (rlen > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("outbase64: rlen=%d\n",rlen) ;
#endif

	    mlen = MIN(BASE64BUFLEN,rlen) ;
	    len = base64_e((buf + i),mlen,outbuf) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {

	        debugprintf("outbase64: mlen=%d base64_e-len=%d\n",
	            mlen,len) ;

	        debugprintf("outbase64: >%w<\n",outbuf,len) ;

	    }
#endif /* CF_DEBUG */

	    rs = bwrite(ofp,outbuf,len) ;
	    wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("outbase64: bwrite rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("outbase64: ret rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outbase64) */


/* output the content type */
static int outct(pip,ofp,ep)
struct proginfo	*pip ;
bfile		*ofp ;
MSGATTACH_ENT	*ep ;
{
	const int	code = ep->code ;
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(ofp,"content-type: %s",ep->type) ;
	wlen += rs ;

/* do we need to put out "/plain" for stupid mailers (like Netscape) ? */

#if	CF_PLAIN

	if ((ep->subtype != NULL) && (ep->subtype[0] != '\0')) {

	    if (rs >= 0)
	        rs = bprintf(ofp, "/%s",ep->subtype) ;

	    wlen += rs ;

	} else if (strcasecmp(ep->type,"text") == 0) {

	    if (rs >= 0) {
	        rs = bprintf(ofp, "/plain") ;
	        wlen += rs ;
	    }

	} /* end if */

#else /* CF_PLAIN */

	if ((ep->subtype != NULL) && (ep->subtype[0] != '\0') &&
	    (code != CE_7BIT)) {

	    if (rs >= 0) {
	        rs = bprintf(ofp, "/%s",ep->subtype) ;
	        wlen += rs ;
	    }

	} /* end if */

#endif /* CF_PLAIN */

	if (rs >= 0) {

	    if ((strcmp(ep->type,"text") == 0) && (code > CE_7BIT) &&
	        ((ep->subtype == NULL) || 
			(strcmp(ep->subtype,"plain") == 0))) {

	        rs = bprintf(ofp," ; charset=iso-8859-1") ;
		wlen += rs ;
		}

	    if ((ep->filename != NULL) && (ep->filename[0] != '-') &&
	        (ep->filename[0] != '\0') &&
	        (strncmp(ep->filename,"/dev/fd/",8) != 0)) {

	        rs = bprintf(ofp," ;\n\tname=\"%s\"",ep->filename) ;
		wlen += rs ;
		}

	} /* end if */

	if (rs >= 0) {
	    rs = bputc(ofp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outct) */


/* output a single value for a header */
static int outvalue(pip,ofp,ldp,v,vlen)
struct proginfo	*pip ;
bfile		*ofp ;
struct outline	*ldp ;
const char	v[] ;
int		vlen ;
{
	int	rs = SR_OK ;
	int	nlen ;
	int	wlen = 0 ;
	int	cl, cl2 ;
	int	f_linestart = FALSE ;

	char	*fmt ;
	char	*tp, *cp ;


	if (ldp == NULL)
	    return SR_INVALID ;

	if ((v == NULL) || (v[0] == '\0'))
	    return SR_OK ;

	if (vlen < 0)
	    vlen = strlen(v) ;

	while ((rs >= 0) && (vlen > 0)) {

	    cl = nextfield(v,vlen,&cp) ;

	    if (cl > 0) {

	        nlen = (f_linestart) ? (cl + 1) : cl ;
	        if (nlen > ldp->rlen) {

	            rs = bprintf(ofp,"\n ") ;

	            wlen += rs ;
	            ldp->rlen = ldp->maxlen - 1 ;
	            f_linestart = FALSE ;
	        }

	        fmt = (f_linestart) ? " %w" : "%w" ;
		if (rs >= 0)
	            rs = bprintf(ofp,fmt,cp,cl) ;

	        wlen += rs ;
	        ldp->rlen -= rs ;
	        cl2 = (cp + cl - v) ;
	        v += cl2 ;
	        vlen -= cl2 ;
	        f_linestart = TRUE ;

	    } else if ((tp = strnchr(v,vlen,'\n')) != NULL) {
	        vlen -= ((tp + 1) - v) ;
	        v = (tp + 1) ;
	    } else
	        vlen = 0 ;

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outvalue) */


static int outline_start(op,maxlen,rlen)
struct outline	*op ;
int		maxlen ;
int		rlen ;
{


	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(struct outline)) ;
	op->maxlen = maxlen ;
	op->rlen = rlen ;

	return SR_OK ;
}
/* end subroutine (outline_start) */


static int outline_finish(op)
struct outline	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (outline_finish) */


