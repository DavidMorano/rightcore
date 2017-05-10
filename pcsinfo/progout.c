/* progout */

/* support building a message without output related subroutines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
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
	PROGINFO	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	const char	v[] ;
	int		vlen ;


	The 'outheadema' subroutine is used to print out (write to the
	mail message file being built) an E-Mail Address (EMA).

	int outheadema(pip,ofp,name,ap)
	PROGINFO	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	EMA		*ap ;


	The 'outpart' subroutine is used to print out a email body part.

	int outpart(pip,ofp,f_multipart,msgboundary,ep)
	struct proginf	*pip ;
	bfile		*ofp ;
	int		f_multipart ;
	char		msgboundary[] ;
	MAILMSGATT_ENT	*ep ;

	Note: Just a little diatribe rant on how stupid the MIME standard
	is for hanlding plain text.  In short, it was the most stupid
	ad idiotic idea to make all mailers make plain text to end in
	<cr><nl> character pairs!  Mail was essentially invented on UNIX
	systems and everybody know (right) that lines are ended with a
	single <nl> character!  The MIME standard really made a big
	mistake with that <cr><nl> crap!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ema.h>
#include	<base64.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mailmsgatt.h"
#include	"contentencodings.h"


/* local defines */

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	BASE64LINELEN	72

#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

#ifndef	FROM_ESCAPE
#define	FROM_ESCAPE	'\b'
#endif

#ifndef	BUFLEN
#define	BUFLEN		MAX(BASE64LINELEN,LINEBUFLEN)
#endif

#define	BIGLINEBUFLEN	4096


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

static int	outct(PROGINFO *,bfile *,MAILMSGATT_ENT *) ;
static int	outvalue(PROGINFO *,bfile *,struct outline *,
			const char *,int) ;
static int	outpartbody(PROGINFO *,bfile *,bfile *,
			MAILMSGATTENT *) ;
static int	outpartbodybits(PROGINFO *,bfile *,bfile *,
			MAILMSGATTENT *) ;
static int	outbase64(PROGINFO *,bfile *,char *,int) ;

static int	outline_start(struct outline *,int,int) ;
static int	outline_finish(struct outline *) ;


/* local variables */


/* exported subroutines */


/* output a general header */
int outhead(pip,ofp,name,v,vlen)
PROGINFO	*pip ;
bfile		*ofp ;
const char	name[] ;
const char	v[] ;
int		vlen ;
{
	struct outline	ld ;
	int		rs ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (v == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	if (vlen < 0)
	    vlen = strlen(v) ;

	if ((rs = outline_start(&ld,MAXMSGLINELEN,MAXMSGLINELEN)) >= 0) {

	if (rs >= 0) {
	    rs = bprintf(ofp,"%s: ",name) ;
	    wlen += rs ;
	    ld.rlen -= rs ;
	}

	if (rs >= 0) {
	    rs = outvalue(pip,ofp,&ld,v,vlen) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,"\n") ;
	    wlen += rs ;
	}

	outline_finish(&ld) ;
	} /* end if (outline) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outhead) */


/* output a header that comtains one or more EMAs */
int outheadema(pip,ofp,name,ap)
PROGINFO	*pip ;
bfile		*ofp ;
const char	name[] ;
EMA		*ap ;
{
	struct outline	ld ;
	EMA_ENT		*ep ;
	int		rs = SR_OK ;
	int		i, n, c ;
	int		nlen ;
	int		wlen = 0 ;
	int		f_linestart = FALSE ;
	const char	*fmt ;

	if (ofp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	if (ap == NULL)
	    goto ret0 ;

	n = ema_count(ap) ;
	if (n <= 0)
	    goto ret0 ;

	if ((rs = outline_start(&ld,MAXMSGLINELEN,MAXMSGLINELEN)) >= 0) {

	if (rs >= 0) {
	    rs = bprintf(ofp,"%s: ",name) ;
	    wlen += rs ;
	    ld.rlen -= rs ;
	}

	c = 0 ;
	for (i = 0 ; (rs >= 0) && (ema_get(ap,i,&ep) >= 0) ; i += 1) {

	    if (ep == NULL) continue ;

/* calculate how much space (columns) that we need for this EMA */

	    nlen = (f_linestart) ? (ep->ol + 2) : ep->ol ;
	    if ((c + 1) < n)
	        nlen += 1 ;

/* see if it will fit in the remaining available line space */

	    if (nlen > ld.rlen) {

	        fmt = (f_linestart) ? ",\n " : "\n " ;
	        rs = bwrite(ofp,fmt,strlen(fmt)) ;
	        wlen += rs ;

	        f_linestart = FALSE ;
	        ld.rlen = ld.maxlen - 1 ;
	    }

	    if (f_linestart) {

	        if (rs >= 0) {
	            rs = bwrite(ofp,", ",2) ;
	            wlen += rs ;
	            ld.rlen -= rs ;
	        }

	    }

	    if (rs >= 0) {

#if	CF_OUTVALUE
	        rs = outvalue(pip,ofp,&ld,ep->op,ep->ol) ;
#else
	        rs = bwrite(ofp,ep->op,ep->ol) ;
	        ld.rlen -= rs ;
#endif /* CF_OUTVALUE */

	        wlen += rs ;

	    }

	    f_linestart = TRUE ;
	    c += 1 ;

	} /* end for */

	if (rs >= 0) {
	    rs = bprintf(ofp,"\n") ;
	    wlen += rs ;
	}

	outline_finish(&ld) ;
	} /* end if (outline) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outheadema) */


/* put out a mail body part */
int outpart(pip,ofp,f_multipart,msgboundary,ep)
PROGINFO	*pip ;
bfile		*ofp ;
int		f_multipart ;
char		msgboundary[] ;
MAILMSGATT_ENT	*ep ;
{
	bfile		infile, *ifp = &infile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		encoding ;
	int		wlen = 0 ;
	int		f_fname ;
	int		f_encoding ;
	int		f_pt = FALSE ;
	char		*fn ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("out/outpart: f_multi=%u\n",f_multipart) ;
#endif

	fn = (ep->auxfname != NULL) ? ep->auxfname : ep->filename ;

	if ((fn == NULL) || (fn[0] == '\0') || (fn[0] == '-')) {
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;
	} else
	    rs = bopen(ifp,fn,"r",0666) ;

	if (rs < 0)
	    goto ret0 ;

/* start printing */

	if (f_multipart) {
	    rs = bprintf(ofp, "--%s\n",msgboundary) ;
	    wlen += rs ;
	}

/* content description */

	if ((rs >= 0) && (ep->description != NULL)) {
	    rs = bprintf(ofp,"content-description: %s\n",ep->description) ;
	    wlen += rs ;
	}

/* content type */

	f_encoding = FALSE ;
	f_fname = TRUE ;
	f_fname = f_fname && (ep->filename != NULL) ;
	f_fname = f_fname && (ep->filename[0] != '\0') ;
	f_fname = f_fname && (ep->filename[0] != '-') ;
	f_fname = f_fname && 
	    (strncmp(ep->filename,"/dev/fd/",8) != 0) ;

	if (rs >= 0)
	    f_pt = ep->f_plaintext ;

	if ((rs >= 0) && (f_multipart || f_fname ||
	    (! f_pt) || (ep->cte > CE_7BIT) ||
	    (ep->subtype != NULL))) {

	    f_encoding = TRUE ;
	    rs = outct(pip,ofp,ep) ;
	    wlen += rs ;

	} /* end if (content type) */

/* content lines */

	if ((rs >= 0) && f_pt) {

	    if (ep->clines >= 0) {
	        rs = bprintf(ofp, "content-lines: %d\n", ep->clines) ;
	        wlen += rs ;
	    }

	} /* end if (content lines) */

/* content transfer encoding */

	encoding = ep->cte ;
	if ((rs >= 0) && (((ep->encoding != NULL) && f_encoding) ||
	    (ep->cte > CE_7BIT))) {

	    rs = bprintf(ofp,
	        "content-transfer-encoding: %s\n",
	        ep->encoding) ;
	    wlen += rs ;

	    if ((rs >= 0) && (ep->cte == CE_BINARY)) {

	        if (ep->clen < 0) {

	            struct ustat	sb ;

	            rs1 = bcontrol(ifp,BC_STAT,&sb) ;
	            if ((rs1 >= 0) && S_ISREG(sb.st_mode))
	                ep->clen = (int) sb.st_size ;

	        } /* end if */

	        if ((rs >= 0) && (ep->clen >= 0)) {
	            rs = bprintf(ofp, "content-length: %u\n", ep->clen) ;
	            wlen += rs ;
	        }

	    } /* end if (binary content) */

	} /* end if (not '7bit') */

/* end-of-headers */

	if (rs >= 0) {
	    rs = bputc(ofp,'\n') ;
	    wlen += rs ;
	}

/* message part body */

	if (rs >= 0) {
	    rs = outpartbody(pip,ofp,ifp,ep) ;
	    wlen += rs ;
	}

	bclose(ifp) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpart) */


/* local subroutines */


/* output the content type */
static int outct(pip,ofp,ep)
PROGINFO	*pip ;
bfile		*ofp ;
MAILMSGATT_ENT	*ep ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (ep->type == NULL)
	    goto ret0 ;

	rs = bprintf(ofp, "content-type: %s",ep->type) ;
	wlen += rs ;

/* do we need to put out "/plain" for stupid mailers (like Netscape)? */

	if ((ep->subtype != NULL) && (ep->subtype[0] != '\0')) {

	    if (rs >= 0) {
	        rs = bprintf(ofp, "/%s",ep->subtype) ;
	        wlen += rs ;
	    }

	} else if (ep->f_plaintext) {

	    if (rs >= 0) {
	        rs = bprintf(ofp, "/plain") ;
	        wlen += rs ;
	    }

	} /* end if */

	if (rs >= 0) {

	    if (ep->f_plaintext && (ep->cte > CE_7BIT)) {
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

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outct) */


/* output a single value for a header (folding lines as needed) */
static int outvalue(pip,ofp,ldp,v,vlen)
PROGINFO	*pip ;
bfile		*ofp ;
struct outline	*ldp ;
const char	v[] ;
int		vlen ;
{
	int		rs = SR_OK ;
	int		nlen ;
	int		wlen = 0 ;
	int		cl, cl2 ;
	int		f_linestart = FALSE ;
	const char	*fmt ;
	const char	*tp, *cp ;

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

	        fmt = (f_linestart) ? " %t" : "%t" ;
	        if (rs >= 0) {
	            rs = bprintf(ofp,fmt,cp,cl) ;
	            wlen += rs ;
	            ldp->rlen -= rs ;
	        }

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


static int outpartbody(pip,ofp,ifp,ep)
PROGINFO	*pip ;
bfile		*ofp ;
bfile		*ifp ;
MAILMSGATTENT	*ep ;
{
	int		rs = SR_OK ;
	int		cte = ep->cte ;
	int		len ;
	int		i ;
	int		wlen = 0 ;
	int		f_pt = ep->f_plaintext ;
	int		f_textcrnl ;
	char		buf[BUFLEN + 2] ; /* added 2 for later */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("out/outpartbody: cte=%u\n",cte) ;
#endif

	f_textcrnl = f_pt && pip->f.crnl ;
	if ((cte == CE_BASE64) && f_textcrnl) {

	    int	f_bol, f_eol ;

	    f_bol = TRUE ;
	    while ((rs = breadline(ifp,buf,BUFLEN)) > 0) {

	        len = rs ;
	        f_eol = (buf[len - 1] == '\n') ;
	        if (f_eol && (len > 1) && (buf[len-2] != '\r')) {
	            buf[len-1] = '\r' ;
	            buf[len++] = '\n' ;
	        }

	        rs = outbase64(pip,ofp,buf,len) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

	        f_bol = f_eol ;

	    } /* end while */

	} else if (cte == CE_BASE64) {

	    const int	rlen = MIN(BASE64BUFLEN,BUFLEN) ;

	    while ((rs = bread(ifp,buf,rlen)) > 0) {

	        len = rs ;
	        rs = outbase64(pip,ofp,buf,len) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

	    } /* end while */

	} else if ((cte >= CE_7BIT) && (cte < CE_BINARY)) {

	    rs = outpartbodybits(pip,ofp,ifp,ep) ;
	    wlen += rs ;

	} else {

	    rs = bcopyblock(ifp,ofp,-1) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpartbody) */


static int outpartbodybits(pip,ofp,ifp,ep)
PROGINFO	*pip ;
bfile		*ofp ;
bfile		*ifp ;
MAILMSGATTENT	*ep ;
{
	const int	cols = MAILTEXTCOLS ;
	const int	ind = 2 ;
	const int	blen = BIGLINEBUFLEN ;
	int		rs = SR_OK ;
	int		size ;
	int		len ;
	int		ll ;
	int		cte = ep->cte ;
	int		wlen = 0 ;
	int		f_bol, f_eol ;
	int		f_pt = ep->f_plaintext ;
	int		f_textcrnl ;
	const char	*lp ;
	char		*bbuf = NULL ;
	char		*p ;

	size = (blen + 1) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0)
	    goto ret0 ;

	bbuf = p ;
	f_textcrnl = pip->f.crnl && f_pt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("out/outpartbodybits: f_pt=%u\n",f_pt) ;
#endif

	f_bol = TRUE ;
	while ((rs = breadline(ifp,bbuf,blen)) > 0) {
	    len = rs ;

	    f_eol = (bbuf[len - 1] == '\n') ;

	    if (f_bol && f_pt && (strncmp(bbuf,"From ",5) == 0)) {
	        rs = bputc(ofp,FROM_ESCAPE) ;
	        wlen += rs ;
	    } /* end if ("From" escape for dumb mailers!) */

	    if (rs >= 0) {
	        if (f_textcrnl) {
		    LINEFOLD	lf ;

		    if (bbuf[len-1] == '\n') {
			len -= 1 ;
			if (bbuf[len-1] == '\r') len -= 1 ;
		    }

	            if ((rs = linefold_start(&lf,cols,ind,bbuf,len)) >= 0) {
	                int	i = 0 ; 
			while ((ll = linefold_get(&lf,i,&lp)) >= 0) {
	                    rs = bwrite(ofp,lp,ll) ;
	                    wlen += rs ;
	                    if (rs >= 0) {
				lp = "\r\n" ;
				ll = 2 ;
	                        rs = bwrite(ofp,lp,ll) ;
	                        wlen += rs ;
	                    }
 			    i += 1 ;
			    if (rs < 0) break ;
	                } /* end while */
	                linefold_finish(&lf) ;
	            } /* end if (linefold) */
	        } else {
	            rs = bwrite(ofp,bbuf,len) ;
	            wlen += rs ;
	        }
	    } /* end if */
	    f_bol = f_eol ;
	    if (rs < 0) break ;
	} /* end while (reading lines) */

ret1:
	if (bbuf != NULL)
	    uc_free(bbuf) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpartbodybits) */


/* write out in BASE64! */
static int outbase64(pip,ofp,buf,buflen)
PROGINFO	*pip ;
bfile		*ofp ;
char		buf[] ;
int		buflen ;
{
	int		rs = SR_OK ;
	int		i = 0 ;
	int		rlen ;
	int		mlen, len ;
	int		wlen = 0 ;
	char		outbuf[BASE64LINELEN + 4] ;

	rlen = buflen ;
	while ((rs >= 0) && (rlen > 0)) {

	    mlen = MIN(BASE64BUFLEN,rlen) ;
	    len = base64_e((buf + i),mlen,outbuf) ;

	    rs = bwrite(ofp,outbuf,len) ;
	    wlen += rs ;
	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outbase64) */


static int outline_start(op,maxlen,rlen)
struct outline	*op ;
int		maxlen ;
int		rlen ;
{

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(struct outline)) ;
	op->maxlen = maxlen ;
	op->rlen = rlen ;

	return SR_OK ;
}
/* end subroutine (outline_start) */


static int outline_finish(op)
struct outline	*op ;
{

	if (op == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (outline_finish) */


