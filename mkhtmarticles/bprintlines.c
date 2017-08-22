/* bprintlines */

/* print out lines but filled to the margin */


#define	CF_DEBUGS	0		/* run-time debug print-outs */


/* revision history:

	= 2004-02-09, David A­D­ Morano
        I snarfed this from another program of mine.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine prints out some lines but not to exceed the line-folding
        length (specified by the caller).

	Synopsis:

	int bprintlines(fp,flen,sp,sl)
	bfile		*fp ;
	int		flen ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	fp		file pointer (BIO)
	flen		line folding length (maximum line length)
	sp		buffer to print out
	sl		amount of data in buffer to print out

	Returns:

	>=0		amount of data printed out
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif


/* external subroutines */

extern int	bprintline(bfile *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bprintlines(bfile *fp,int flen,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*fbuf = NULL ;

	if (fp == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (fp->f.nullfile) goto ret0 ;

/* continue */

	if (flen < 2)
	    flen = LINEBUFLEN ;

	if (llen < 0)
	    llen = strlen(lbuf) ;

#if	CF_DEBUGS
	debugprintf("bprintlines: flen=%u\n",flen) ;
#endif

	if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	    SBUF	b ;
	    int		ll = llen ;
	    int		sl, cl ;
	    int		wc, len ;
	    int		f_sbuf = FALSE ;
	    const char	*lp = lbuf ;
	    const char	*tp, *sp ;
	    const char	*cp ;

#if	CF_DEBUGS
	    debugprintf("bprintlines: buf=>%t<\n",
	        lbuf,
	        MIN(60,((lbuf[llen - 1] == '\n') ? (llen - 1) : llen))) ;
#endif

	    while ((rs >= 0) && (ll > 0)) {

	        sp = lp ;
	        sl = ll ;
	        wc = 0 ;
	        len = 0 ;
	        rs = sbuf_start(&b,fbuf,flen) ;
	        f_sbuf = (rs >= 0) ;

	        while ((rs >= 0) && ((cl = nextfield(sp,sl,&cp)) > 0)) {

#if	CF_DEBUGS
	            debugprintf("bprintlines: field=>%t<\n",cp,cl) ;
#endif

	            if ((len + (cl + 1)) > flen) {

#if	CF_DEBUGS
	                debugprintf("bprintlines: line-overflow len=%u\n",
	                    len) ;
#endif

	                f_sbuf = FALSE ;
	                sbuf_finish(&b) ;

	                if ((rs = bprintline(fp,fbuf,len)) >= 0) {
	                    wlen += rs ;
	                    wc = 0 ;
	                    len = 0 ;
	                    rs = sbuf_start(&b,fbuf,flen) ;
	                    f_sbuf = (rs >= 0) ;
	                }

	            } /* end if (doing a flush) */

	            if (rs >= 0) {

	                if (wc++ > 0) {
	                    len += 1 ;
	                    sbuf_char(&b,' ') ;
	                }

#if	CF_DEBUGS
	                debugprintf("bprintlines: storing=>%t<\n",
				cp,cl) ;
#endif

	                len += cl ;
	                rs = sbuf_strw(&b,cp,cl) ;

#if	CF_DEBUGS
	                debugprintf("bprintlines: sbuf_strw() rs=%d\n",
				rs) ;
#endif

	            } /* end if */

	            sl -= ((cp + cl) - sp) ;
	            sp = (cp + cl) ;

	        } /* end while (grabbing white-space separated words) */

	        if (f_sbuf) {
	            f_sbuf = FALSE ;
	            sbuf_finish(&b) ;
	        }

	        if ((rs >= 0) && (len > 0)) {
	            rs = bprintline(fp,fbuf,len) ;
	            wlen += rs ;
	        }

	        if ((tp = strnchr(sp,sl,'\n')) == NULL) {
	            tp = (sp + sl - 1) ;
	 	}

	        ll -= ((tp + 1) - lp) ;
	        lp = (tp + 1) ;

#if	CF_DEBUGS
	        debugprintf("bprintlines: bottom rs=%d\n",rs) ;
#endif

	    } /* end while (outer) */

	    if ((rs >= 0) && (wlen == 0)) {
	        rs = bputc(fp,'\n') ;
	        wlen += rs ;
	    }

	    rs1 = uc_free(fbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a) */

ret0:

#if	CF_DEBUGS
	debugprintf("bprintlines: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintlines) */


