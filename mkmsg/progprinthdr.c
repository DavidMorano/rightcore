/* progprinthdr */

/* print messages addresses */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_OUTVALUE	1		/* use 'outvalue()' */
#define	CF_HDRILINE	1		/* use 'mailmsg_hdriline()' */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
        I added a little code to "post" articles that do not have a valid
        newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module processes one or more mail messages (in appropriate mailbox
        format if more than one) on STDIN. The output is a single file that is
        ready to be added to each individual mailbox in the spool area.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<char.h>
#include	<mailmsg.h>
#include	<ema.h>
#include	<emainfo.h>
#include	<outline.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"contentencodings.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	LOGLINELEN	(80 - 16)

#define	DATEBUFLEN	80
#define	STACKADDRBUFLEN	(2 * 1024)

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#ifndef	MAILMSGLINELEN
#define	MAILMSGLINELEN	76
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#define	BASE64LINELEN	72
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

#ifndef	FROM_ESCAPE
#define	FROM_ESCAPE	'\b'
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))


/* external subroutines */

extern uint	uceil(uint,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

int		progprinthdremas(PROGINFO *,bfile *,const char *,EMA *) ;

static int	procprintline(PROGINFO *,bfile *,int,const char *,int) ;

#ifdef	COMMENT
static int	procoutvalue(PROGINFO *,bfile *,OUTLINE *,
			const char *,int) ;
#endif

static int	nexttoken(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int progprinthdraddrs(pip,ofp,mp,hdr)
PROGINFO	*pip ;
bfile		*ofp ;
MAILMSG		*mp ;
const char	hdr[] ;
{
	EMA		a ;
	int		rs = SR_OK ;
	int		i ;
	int		sl ;
	int		wlen = 0 ;
	const char	*sp ;

	if (ofp == NULL) return SR_FAULT ;
	if (mp == NULL) return SR_FAULT ;
	if (hdr == NULL) return SR_FAULT ;

	if (hdr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progprinthdraddrs: hdr=%s\n",hdr) ;
#endif

	for (i = 0 ; (sl = mailmsg_hdrival(mp,hdr,i,&sp)) >= 0 ; i += 1) {
	    if ((sl > 0) && (sp != NULL)) {
	        if ((rs = ema_start(&a)) >= 0) {
	            if ((rs = ema_parse(&a,sp,sl)) >= 0) {
	                rs = progprinthdremas(pip,ofp,hdr,&a) ;
	                wlen += rs ;
	            }
	            ema_finish(&a) ;
	        } /* end if */
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progprinthdraddrs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progprinthdraddrs) */


/* output a header that comtains one or more EMAs */
int progprinthdremas(pip,ofp,hdr,ap)
PROGINFO	*pip ;
bfile		*ofp ;
const char	hdr[] ;
EMA		*ap ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (hdr == NULL) return SR_FAULT ;

	if (hdr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progprinthdremas: hdr=%s\n",hdr) ;
#endif

	if (ap != NULL) {
	    if ((rs = ema_count(ap)) > 0) {
	        OUTLINE	ld ;
	        if ((rs = outline_start(&ld,ofp,MAILMSGLINELEN)) >= 0) {
	            if ((rs = outline_printf(&ld,"%s: ",hdr)) >= 0) {
			EMA_ENT	*ep ;
	                int	i ;
			wlen += rs ;
	                for (i = 0 ; ema_get(ap,i,&ep) >= 0 ; i += 1) {
	                    if (ep != NULL) {
	                        if (ep->ol > 0) {
	                            rs = outline_item(&ld,ep->op,ep->ol) ;
				    wlen += rs ;
	                        }
			    }
		            if (rs < 0) break ;
	                } /* end for */
	            } /* end if */
	            rs1 = outline_finish(&ld) ;
	            if (rs >= 0) rs = rs1 ;
		    wlen += rs1 ;
	        } /* end if (outline) */
	    } /* end if (ema-count) */
	} /* end if (non-null) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progprinthdremas: ret rs=%d c=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progprinthdremas) */


/* output all header instances of a given keyname */
int progprinthdrs(pip,ofp,mp,hdr)
PROGINFO	*pip ;
bfile		*ofp ;
MAILMSG		*mp ;
const char	hdr[] ;
{
	int		rs = SR_OK ;
	int		n ;
	int		vlen ;
	int		olenr ;
	int		sl, cl ;
	int		wlen = 0 ;
	const char	*sp ;
	const char	*cp ;

	if (mp == NULL) return SR_FAULT ;
	if (hdr == NULL) return SR_FAULT ;

	if (hdr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progprinthdrs: hdr=%s\n",hdr) ;
#endif

	if ((n = mailmsg_hdrcount(mp,hdr)) >= 0) {
	    int	i, j ;
	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {

	    vlen = 0 ;
	    olenr = MAILMSGLINELEN ;
	    rs = bprintf(ofp,"%s:",hdr) ;
	    wlen += rs ;
	    olenr -= rs ;

#if	CF_HDRILINE

	    j = 0 ;
	    while (rs >= 0) {
	        sl = mailmsg_hdriline(mp,hdr,i,j,&sp) ;
	        if (sl == SR_NOTFOUND) break ;
		rs = sl ;

/* shrink and skip empty lines (some new RFC says to skip empty lines!) */

	        if ((rs >= 0) && (sl > 0)) {
	            if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	                rs = procprintline(pip,ofp,olenr,cp,cl) ;
	                wlen += rs ;
	                vlen += rs ;
	                olenr = MAILMSGLINELEN ;
	            }
	        } /* end if */

	        j += 1 ;
	    } /* end while (lines within a header instance) */

#else /* CF_HDRILINE */

	    if ((rs >= 0) && ((sl = mailmsg_hdrival(mp,hdr,i,&sp)) >= 0)) {
	        if ((sp != NULL) && ((cl = sfshrink(sp,sl,&cp)) > 0)) {
	            rs = procprintline(pip,ofp,olenr,cp,cl) ;
	            wlen += rs ;
	            vlen += rs ;
	        }
	    } /* end if */

#endif /* CF_HDRILINE */

	    if ((rs >= 0) && (vlen == 0)) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	} /* end for (header instances) */
	} /* end if (non-zero count) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprinthdrs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progprinthdrs) */


/* output a general header */
int progprinthdr(pip,ofp,hdr,vp,vl)
PROGINFO	*pip ;
bfile		*ofp ;
const char	hdr[] ;
const char	vp[] ;
int		vl ;
{
	OUTLINE		ld ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (hdr == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (hdr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprinthdr: hdr=%s\n",hdr) ;
#endif

	if (vl < 0) vl = strlen(vp) ;

	if ((rs = outline_start(&ld,ofp,MAILMSGLINELEN)) >= 0) {

	    if ((rs = outline_printf(&ld,"%s:",hdr)) >= 0) {
		wlen += rs ;
	        rs = outline_value(&ld,vp,vl) ;
		wlen += rs ;
	    }

	    rs1 = outline_finish(&ld) ;
	    if (rs >= 0) rs = rs1 ;
	    wlen += rs1 ;
	} /* end if (outline) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprinthdr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progprinthdr) */


/* local subroutines */


/* output a header line (and do some safety checking along the way) */
static int procprintline(pip,ofp,olenr,lp,ll)
PROGINFO	*pip ;
bfile		*ofp ;
int		olenr ;
const char	*lp ;
int		ll ;
{
	const int	linewidth = MAILMSGLINELEN ;
	int		rs = SR_OK ;
	int		li ;
	int		tl, cl ;
	int		stepcols, havecols ;
	int		ch ;
	int		line = 0 ;
	int		wlen = 0 ;
	int		f_rmwhite = TRUE ;
	int		f_badchar = FALSE ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procprintline: olenr=%d\n",olenr) ;
#endif

/* assume that we know where we are in the current line! */

	li = (linewidth - olenr) ;

/* output the leading space */

	if (ll > 0) {
	    rs = bputc(ofp,' ') ;
	    wlen += rs ;
	    li += 1 ;
	    olenr -= 1 ;
	} /* end if */

/* output the rest of the line given us */

	while ((rs >= 0) && (ll > 0)) {

	    cl = nexttoken(lp,ll,&cp) ;
	    tl = cl ;
	    if (cl <= 0) break ;

/* determine what sort of character we have (if only one) */

	    ch = MKCHAR(cp[0]) ;
	    f_badchar = FALSE ;
	    if (ch == '\t') {

	        stepcols = uceil(((li % NTABCOLS) + 1),NTABCOLS) ;

	    } else if ((cl == 1) && (! isprintlatin(ch))) {

	        f_badchar = TRUE ;
	        stepcols = 1 ;
	        cl = 1 ;

	    } else
	        stepcols = cl ;

/* calculation on whether we are out-of-space within the line */

	    if ((line == 0) || (li > NTABCOLS)) {
	        havecols = olenr ;
	    } else
	        havecols = linewidth - NTABCOLS ;

	    if ((stepcols > havecols) && (li > NTABCOLS)) {

	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	        li = 0 ;
	        olenr = linewidth ;
	        line += 1 ;

	    } /* end if */

/* do we have white-space at a boundary? */

	    if (f_rmwhite && (cl > 0) && (li == 0)) {

	        if (CHAR_ISWHITE(ch) || f_badchar) {
	            cl = 0 ;
	            stepcols = 0 ;
	        }

	    } /* end if */

/* print out some stuff */

	    if (cl > 0) {

	        if ((line > 0) && (li == 0)) {

	            if (rs >= 0) {
	                rs = bputc(ofp,'\t') ;
	                wlen += rs ;
	            }

	            li = NTABCOLS ;
	            olenr = linewidth - li ;

	        } /* end if (outputting indent) */

	        if (rs >= 0) {

	            if (f_badchar) {
	                rs = bputc(ofp,' ') ;
	            } else
	                rs = bwrite(ofp,cp,cl) ;

	            wlen += rs ;
	            li += stepcols ;
	            olenr -= stepcols ;

	        } /* end if */

	    } /* end if */

	    lp += tl ;
	    ll -= tl ;

	} /* end while (processing tokens) */

	if ((rs >= 0) && (wlen > 0)) {

	    rs = bputc(ofp,'\n') ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintline) */


#ifdef	COMMENT

/* output a single value for a header (folding lines as needed) */
static int procoutvalue(pip,ofp,ldp,v,vlen)
PROGINFO	*pip ;
bfile		*ofp ;
OUTLINE		*ldp ;
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

	    if ((cl = nextfield(v,vlen,&cp)) > 0) {

	        nlen = (f_linestart) ? (cl + 1) : cl ;
	        if (nlen > ldp->rlen) {

	            rs = bprintf(ofp,"\n ") ;
	            wlen += rs ;

	            ldp->rlen = ldp->maxlen ;
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
/* end subroutine (procoutvalue) */

#endif /* COMMENT */


static int nexttoken(cchar *sp,int sl,cchar **rpp)
{
	int		len = 0 ;
	*rpp = sp ;
	if (sl > 0) {
	    if (! CHAR_TOKSEP(*sp)) {
	        while ((sl > 0) && (! CHAR_TOKSEP(*sp))) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	    } else {
	        sp += 1 ;
	    }
	    len = (sp - (*rpp)) ;
	} /* end if */
	return len ;
}
/* end subroutine (nexttoken) */


