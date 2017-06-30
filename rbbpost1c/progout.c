/* progout */

/* support building a message without output related subroutines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_OUTVALUE	1		/* use 'outvalue()' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Output a header.

	int progouthead(pip,ofp,name,vp,vl)
	PROGINFO	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	const char	vp[] ;
	int		vl ;


	The 'progoutheadema' subroutine is used to print out (write to the
	mail message file being built) an E-Mail Address (EMA).

	int progoutheadema(pip,ofp,name,ap)
	PROGINFO	*pip ;
	bfile		*ofp ;
	const char	name[] ;
	EMA		*ap ;


	The 'progoutpart' subroutine is used to print out a email body part.

	int progoutpart(pip,ofp,f_multipart,msgboundary,ep)
	struct proginf	*pip ;
	bfile		*ofp ;
	int		f_multipart ;
	char		msgboundary[] ;
	MAILMSGATTENT	*ep ;

	Note: Just a little diatribe rant on how stupid the MIME standard is
	for hanlding plain text.  In short, it was the most stupid and idiotic
	idea ever to make all mailers make plain text to end in <cr><nl>
	character pairs!  Mail was essentially invented on UNIX® systems and
	everybody knows (right well) that lines are ended with a single <nl>
	character!  The MIME standard really made a big mistake with that
	<cr><nl> crap!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ema.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<base64.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mailmsgatt.h"
#include	"contentencodings.h"


/* local defines */

#ifndef	MAILMSGLINELEN
#define	MAILMSGLINELEN	76
#endif

#ifndef	MAILTEXTCOLS
#define	MAILTEXTCOLS	998
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

#define	OUTLINE		struct outline


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;
extern int	buffer_stropaque(BUFFER *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */

struct outline {
	int		maxlen ;
	int		rlen ;
} ;


/* forward references */

static int	outct(PROGINFO *,bfile *,MAILMSGATTENT *) ;
static int	outentry(PROGINFO *,bfile *,OUTLINE *,BUFFER *,EMA_ENT *) ;
static int	outvalue(PROGINFO *,bfile *,OUTLINE *,cchar *,int) ;
static int	outpartbody(PROGINFO *,bfile *,bfile *,MAILMSGATTENT *) ;
static int	outpartbodybits(PROGINFO *,bfile *,bfile *,MAILMSGATTENT *) ;
static int	outbase64(PROGINFO *,bfile *,const char *,int) ;

static int	outline_start(OUTLINE *,int,int) ;
static int	outline_finish(OUTLINE *) ;

static int	strestlen(const char *,int) ;


/* local variables */


/* exported subroutines */


/* output a general header */
int progouthead(PROGINFO *pip,bfile *ofp,cchar *name,cchar *vp,int vl)
{
	OUTLINE		ld ;
	const int	llen = MAILMSGLINELEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	if (vl < 0)
	    vl = strlen(vp) ;

	if ((rs = outline_start(&ld,llen,llen)) >= 0) {

	    if (rs >= 0) {
	        rs = bprintf(ofp,"%s: ",name) ;
	        wlen += rs ;
	        ld.rlen -= rs ;
	    }

	    if (rs >= 0) {
	        rs = outvalue(pip,ofp,&ld,vp,vl) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	    rs1 = outline_finish(&ld) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (outline) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progouthead) */


/* output a header that comtains one or more EMAs */
int progoutheadema(PROGINFO *pip,bfile *ofp,cchar *name,EMA *ap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progout/_headema: ent name=%s\n",name) ;
#endif

	if (ofp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	if (ap != NULL) {
	    BUFFER	b ;
	    if ((rs = buffer_start(&b,80)) >= 0) {
	        if ((rs = ema_count(ap)) > 0) {
	            OUTLINE	ld ;
	            const int	llen = MAILMSGLINELEN ;
	            const int	n = rs ;
	            if ((rs = outline_start(&ld,llen,llen)) >= 0) {
	                int	c = 0 ;

	                if (rs >= 0) {
	                    rs = bprintf(ofp,"%s: ",name) ;
	                    wlen += rs ;
	                    ld.rlen -= rs ;
	                }

	                if (rs >= 0) {
	                    EMA_ENT	*ep ;
	                    const char	*fmt ;
	                    int		nlen ;
	                    int		elen ;
	                    int		i ;
	                    int		f_linestart = FALSE ;
	                    for (i = 0 ; ema_get(ap,i,&ep) >= 0 ; i += 1) {
	                        if (ep != NULL) {

	                        elen = strestlen(ep->op,ep->ol) ;

/* calculate how much space (columns) need for this EMA */

	                        nlen = (f_linestart) ? (elen+2) : elen ;
	                        if ((c + 1) < n) nlen += 1 ;

/* see if it will fit in remaining available space */

	                        if (nlen > ld.rlen) {
	                            fmt = (f_linestart) ? ",\n " : "\n " ;
	                            rs = bwrite(ofp,fmt,strlen(fmt)) ;
	                            wlen += rs ;
	                            f_linestart = FALSE ;
	                            ld.rlen = (ld.maxlen - 1) ;
	                        }

	                        if ((rs >= 0) && f_linestart) {
	                            rs = bwrite(ofp,", ",2) ;
	                            wlen += rs ;
	                            ld.rlen -= rs ;
	                        } /* end if */

	                        if (rs >= 0) {

#if	CF_OUTVALUE
	                            rs = outentry(pip,ofp,&ld,&b,ep) ;
	                            wlen += rs ;
#else
	                            rs = bwrite(ofp,ep->op,ep->ol) ;
	                            wlen += rs ;
	                            ld.rlen -= rs ;
#endif /* CF_OUTVALUE */

	                        } /* end if */

	                        f_linestart = TRUE ;
	                        c += 1 ;

				}
	                        if (rs < 0) break ;
	                    } /* end for */
	                } /* end if (ok) */

	                if (rs >= 0) {
	                    rs = bprintf(ofp,"\n") ;
	                    wlen += rs ;
	                }

	                rs1 = outline_finish(&ld) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (outline) */
	        } /* end if (ema-count) */
	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} /* end if (non-null) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutheadema) */


/* put out a mail body part */
int progoutpart(pip,ofp,f_multipart,msgboundary,ep)
PROGINFO	*pip ;
bfile		*ofp ;
int		f_multipart ;
cchar		*msgboundary ;
MAILMSGATTENT	*ep ;
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*fn ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progoutpart: ent\n") ;
	    debugprintf("progoutpart: f_multi=%u\n",f_multipart) ;
	    debugprintf("progoutpart: type=%s\n",ep->type) ;
	    debugprintf("progoutpart: subtype=%s\n",ep->subtype) ;
	}
#endif

	fn = (ep->auxfname != NULL) ? ep->auxfname : ep->attfname ;
	if ((fn == NULL) || (fn[0] == '\0') || (fn[0] == '-'))
	    fn = BFILE_STDIN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progoutpart: fn=%s\n",fn) ;
#endif

	if ((rs = bopen(ifp,fn,"r",0666)) >= 0) {
	    const int	cte = ep->cte ;
	    const int	f_pt = ep->f_plaintext ;
	    int		f_fname ;
	    int		f_enc = FALSE ;
	    cchar	*dn = "/dev/fd/" ;
	    cchar	*enc = ep->encoding ;
	    cchar	*kn ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progoutpart: f_pt=%u\n",f_pt) ;
#endif

/* start printing */

	    if (f_multipart) {
	        rs = bprintf(ofp,"--%s\n",msgboundary) ;
	        wlen += rs ;
	    }

/* content description */

	    if ((rs >= 0) && (ep->description != NULL)) {
	        cchar	*desc = ep->description ;
	        kn = "content-description" ;
	        rs = bprintf(ofp,"%s: %s\n",kn,desc) ;
	        wlen += rs ;
	    }

/* content type */

	    f_fname = TRUE ;
	    f_fname = f_fname && (ep->attfname != NULL) ;
	    f_fname = f_fname && (ep->attfname[0] != '\0') ;
	    f_fname = f_fname && (ep->attfname[0] != '-') ;
	    f_fname = f_fname && (strncmp(ep->attfname,dn,8) != 0) ;

	    if (rs >= 0) {
	        const int	f0 = (ep->cte >= CE_7BIT) ;
	        const int	f1 = (ep->subtype != NULL) ;
	        if (f_multipart || f_fname || (! f_pt) || f0 || f1) {
	            f_enc = TRUE ;
	            rs = outct(pip,ofp,ep) ;
	            wlen += rs ;
	        }
	    } /* end if (content type) */

/* content disposition */

	    if ((rs >= 0) && f_multipart && pip->f.dis_inline) {
	        cchar	*val = "inline" ;
	        kn = "content-disposition" ;
	        rs = bprintf(ofp, "%s: %s\n",kn,val) ;
	        wlen += rs ;
	    } /* end if (content-disposition) */

/* content transfer encoding */

	    if ((rs >= 0) && (((enc != NULL) && f_enc) || (cte >= CE_7BIT))) {

	        if (enc != NULL) {
	            kn = "content-transfer-encoding" ;
	            rs = bprintf(ofp,"%s: %s\n",kn,enc) ;
	            wlen += rs ;
	        }

	        if ((rs >= 0) && (ep->cte == CE_BINARY)) {

	            if (ep->clen < 0) {
	                struct ustat	sb ;
	                if ((rs1 = bcontrol(ifp,BC_STAT,&sb)) >= 0) {
	                    if (S_ISREG(sb.st_mode)) {
	                        ep->clen = (int) sb.st_size ;
	                    }
	                }
	            } /* end if (try to get CLEN) */

	            if ((rs >= 0) && (ep->clen >= 0)) {
	                kn = "content-length" ;
	                rs = bprintf(ofp, "%s: %u\n",kn,ep->clen) ;
	                wlen += rs ;
	            }

	        } /* end if (binary content) */

	    } /* end if (not '7bit') */

/* content lines */

	    if ((rs >= 0) && f_pt) {
	        if (ep->clines >= 0) {
	            kn = "content-lines" ;
	            rs = bprintf(ofp,"%s: %d\n",kn,ep->clines) ;
	            wlen += rs ;
	        }
	    } /* end if (content lines) */

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

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progoutpart: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutpart) */


/* local subroutines */


static int outentry(pip,ofp,ldp,bufp,ep)
PROGINFO	*pip ;
bfile		*ofp ;
OUTLINE		*ldp ;
BUFFER		*bufp ;
EMA_ENT		*ep ;
{
	int		rs ;
	int		wlen = 0 ;

	if ((rs = buffer_reset(bufp)) >= 0) {
	    const char	*bp ;
	    int		bl ;
	    int		c = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("outentry: a=>%t<\n",
	            ep->ap,strlinelen(ep->ap,ep->al,40)) ;
#endif

	    if ((rs >= 0) && (ep->ap != NULL) && (ep->al > 0)) {
	        if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	        if (rs >= 0)
	            rs = buffer_stropaque(bufp,ep->ap,ep->al) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("outentry: r=>%t<\n",
	            ep->rp,strlinelen(ep->rp,ep->rl,40)) ;
#endif

	    if ((rs >= 0) && (ep->rp != NULL) && (ep->rl > 0)) {
	        if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_LANGLE) ;
	        if (rs >= 0)
	            rs = buffer_stropaque(bufp,ep->rp,ep->rl) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_RANGLE) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("outentry: c=>%t<\n",
	            ep->cp,strlinelen(ep->cp,ep->cl,40)) ;
#endif

	    if ((rs >= 0) && (ep->cp != NULL) && (ep->cl > 0)) {
	        if (c++ > 0) rs = buffer_char(bufp,CH_SP) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_LPAREN) ;
	        if (rs >= 0)
	            rs = buffer_strw(bufp,ep->cp,ep->cl) ;
	        if (rs >= 0)
	            rs = buffer_char(bufp,CH_RPAREN) ;
	    }

	    if ((rs = buffer_get(bufp,&bp)) > 0) {
	        bl = rs ;
	        rs = outvalue(pip,ofp,ldp,bp,bl) ;
	        wlen += rs ;
	    }

	} /* end if (buffer-reset) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("outentry: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outentry) */


/* output a single value for a header (folding lines as needed) */
static int outvalue(pip,ofp,ldp,vp,vl)
PROGINFO	*pip ;
bfile		*ofp ;
OUTLINE		*ldp ;
const char	vp[] ;
int		vl ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (ldp == NULL) return SR_INVALID ;

	if ((vp != NULL) && (vp[0] != '\0')) {
	    int		nlen ;
	    int		cl, cl2 ;
	    int		f_linestart = FALSE ;
	    const char	*fmt ;
	    const char	*tp, *cp ;

	    if (vl < 0) vl = strlen(vp) ;

	    while ((rs >= 0) && (vl > 0)) {

	        if ((cl = nextfield(vp,vl,&cp)) > 0) {

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

	            cl2 = (cp + cl - vp) ;
	            vp += cl2 ;
	            vl -= cl2 ;
	            f_linestart = TRUE ;

	        } else if ((tp = strnchr(vp,vl,'\n')) != NULL) {
	            vl -= ((tp + 1) - vp) ;
	            vp = (tp + 1) ;
	        } else {
	            vl = 0 ;
		}

	    } /* end while */

	} /* end if (non-empty) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outvalue) */


/* output the content type */
static int outct(PROGINFO *pip,bfile *ofp,MAILMSGATTENT *ep)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		cte = ep->cte ;
	const int	f_pt = ep->f_plaintext ;
	int		f_mime = pip->f.mime ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progout/outct: ent f_mime=%u\n",f_mime) ;
#endif

	if ((ep->type != NULL) && (f_mime || (! f_pt) || (cte >= CE_7BIT))) {
	    cchar	*kn ;

	    kn = "content-type" ;
	    if ((rs = bprintf(ofp,"%s:\n",kn)) >= 0) {
	        wlen += rs ;
	        rs = bprintf(ofp," %s",ep->type) ;
	        wlen += rs ;
	    }

/* do we need to put out "/plain" for stupid mailers (like Netscape)? */

	    if (rs >= 0) {
	        if ((ep->subtype != NULL) && (ep->subtype[0] != '\0')) {
	            rs = bprintf(ofp,"/%s",ep->subtype) ;
	            wlen += rs ;
	        } else if (ep->f_plaintext) {
	            rs = bprintf(ofp,"/plain") ;
	            wlen += rs ;
	        } /* end if */
	    } /* end if */

	    if (rs >= 0) {

	        if (ep->f_plaintext && (ep->cte >= CE_7BIT)) {
	            cchar *cs = "ISO-8859-1" ;
	            if (ep->cte == CE_7BIT) cs = "US-ASCII" ;
	            rs = bprintf(ofp," ; charset=%s",cs) ;
	            wlen += rs ;
	        }

	        if (rs >= 0) {
	            cchar	*dn = "/dev/fd/" ;
	            cchar	*fn = ep->attfname ;
	            if ((fn != NULL) && (fn[0] != '-') && (fn[0] != '\0')) {
	                if (strncmp(fn,dn,8) != 0) {
	                    if ((rs = bprintf(ofp,";\n")) >= 0) {
	                        wlen += rs ;
	                        rs = bprintf(ofp," name=\"%s\"",fn) ;
	                        wlen += rs ;
	                    }
	                }
	            }
	        }

	    } /* end if */

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	} /* end if (non-null) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progout/outct: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outct) */


static int outpartbody(PROGINFO *pip,bfile *ofp,bfile *ifp,MAILMSGATTENT *ep)
{
	const int	cte = ep->cte ;
	const int	rlen = BUFLEN ;
	int		rs = SR_OK ;
	int		len ;
	int		wlen = 0 ;
	int		f_textcrnl = (ep->f_plaintext && pip->f.crnl) ;
	char		rbuf[BUFLEN + 2] ; /* added 2 rather than 1 for later */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progoutbody: ent cte=%u f_textcrnl=%u\n",
	        cte,f_textcrnl) ;
#endif

	if ((cte == CE_BASE64) && f_textcrnl) {
	    int	f_eol ;

	    while ((rs = breadline(ifp,rbuf,rlen)) > 0) {
	        len = rs ;

	        f_eol = (rbuf[len - 1] == '\n') ;
	        if (f_eol && (len > 1) && (rbuf[len-2] != '\r')) {
	            rbuf[len-1] = '\r' ;
	            rbuf[len++] = '\n' ;
	        }

	        rs = outbase64(pip,ofp,rbuf,len) ;
	        wlen += rs ;

	        if (rs < 0) break ;
	    } /* end while */

	} else if (cte == CE_BASE64) {
	    const int	ml = MIN(BASE64BUFLEN,BUFLEN) ;

	    while ((rs = bread(ifp,rbuf,ml)) > 0) {
	        len = rs ;

	        rs = outbase64(pip,ofp,rbuf,len) ;
	        wlen += rs ;

	        if (rs < 0) break ;
	    } /* end while */

	} else if ((cte >= CE_7BIT) && (cte < CE_BINARY)) {

	    rs = outpartbodybits(pip,ofp,ifp,ep) ;
	    wlen += rs ;

	} else {

	    rs = bcopyblock(ifp,ofp,-1) ;
	    wlen += rs ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progoutbody: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpartbody) */


static int outpartbodybits(PROGINFO *pip,bfile *ofp,bfile *ifp,
	MAILMSGATTENT *ep)
{
	const int	ind = 2 ;
	const int	bllen = BIGLINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const int	f_pt = ep->f_plaintext ;
	char		*blbuf = NULL ;
	char		*p ;

	if ((rs = uc_malloc((bllen+1),&p)) >= 0) {
	    LINEFOLD	lf ;
	    const int	cols = MAILTEXTCOLS ;
	    const int	f_textcrnl = (pip->f.crnl && f_pt) ;
	    int		len ;
	    int		ll ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    cchar	*lp ;
	    blbuf = p ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("out/outpartbodybits: f_pt=%u\n",f_pt) ;
	        debugprintf("out/outpartbodybits: f_textcrnl=%u\n",f_textcrnl) ;
	    }
#endif

	    while ((rs = breadline(ifp,blbuf,bllen)) > 0) {
	        len = rs ;

	        f_eol = (blbuf[len - 1] == '\n') ;
	        if (f_bol && f_pt && (strncmp(blbuf,"From ",5) == 0)) {
	            rs = bputc(ofp,FROM_ESCAPE) ;
	            wlen += rs ;
	        } /* end if ("From" escape for dumb mailers!) */

	        if (rs >= 0) {
	            if (f_textcrnl) {

	                if (blbuf[len-1] == '\n') {
	                    len -= 1 ;
	                    if (blbuf[len-1] == '\r') len -= 1 ;
	                }

	                if (len > 0) {
	                    const int	c = cols ;
	                    const int	l = len ;
	                    char	*blb = blbuf ;
	                    if ((rs = linefold_start(&lf,c,ind,blb,l)) >= 0) {
	                        int	i = 0 ;

	                        while ((ll = linefold_get(&lf,i,&lp)) >= 0) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(5))
	                                debugprintf("out/outpartbodybits: "
	                                    "i=%u l=>%t<\n",
	                                    i,lp,strlinelen(lp,ll,40)) ;
#endif

	                            if (ll > 0) {
	                                rs = bwrite(ofp,lp,ll) ;
	                                wlen += rs ;
	                            }

	                            if (rs >= 0) {
	                                lp = "\r\n" ;
	                                ll = 2 ;
	                                rs = bwrite(ofp,lp,ll) ;
	                                wlen += rs ;
	                            }

	                            i += 1 ;
	                        } /* end while */

	                        rs1 = linefold_finish(&lf) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (linefold) */
	                } else {
	                    lp = "\r\n" ;
	                    ll = 2 ;
	                    rs = bwrite(ofp,lp,ll) ;
	                    wlen += rs ;
	                }

	            } else {
	                rs = bwrite(ofp,blbuf,len) ;
	                wlen += rs ;
	            }
	        } /* end if (ok) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = uc_free(blbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("out/outpartbodybits: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outpartbodybits) */


/* write out in BASE64! */
static int outbase64(PROGINFO *pip,bfile *ofp,cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	int		rlen = slen ;
	int		len ;
	int		wlen = 0 ;
	char		outbuf[BASE64LINELEN + 4] ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progout/outbase64: ent slen=%u\n",slen) ;
#endif

	while ((rs >= 0) && (rlen > 0)) {
	    const int	mlen = MIN(BASE64BUFLEN,rlen) ;
	    len = base64_e((sbuf + i),mlen,outbuf) ;
	    rs = bprintline(ofp,outbuf,len) ;
	    wlen += rs ;
	    rlen -= mlen ;
	    i += mlen ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progout/outbase64: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outbase64) */


static int outline_start(OUTLINE *op,int maxlen,int rlen)
{

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(OUTLINE)) ;
	op->maxlen = maxlen ;
	op->rlen = rlen ;

	return SR_OK ;
}
/* end subroutine (outline_start) */


static int outline_finish(OUTLINE *op)
{

	if (op == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (outline_finish) */


/* calculate an estimated length */
static int strestlen(cchar *sp,int sl)
{
	int		len = 0 ;
	int		cl ;
	const char	*cp ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {
	    len += (cl+1) ;
	    sl -= ((cp+cl)-sp) ;
	    sp = (cp+cl) ;
	} /* end while */
	return len ;
}
/* end subroutine (strestlen) */


