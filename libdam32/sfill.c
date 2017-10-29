/* sfill */

/* text fill */


#define	CF_DEBUGS	0		/* used for little object below */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object mediates filling out text to an output file using the BFILE
	package.


*******************************************************************************/


#define	SFILL_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<fifostr.h>
#include	<localmisc.h>

#include	"sfill.h"


/* local defines */

#ifndef	WORDBUFLEN
#define	WORDBUFLEN	100
#endif


/* external subroutines */

extern int	bwriteblanks(bfile *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sfill_start(SFILL *op,int indent,bfile *ofp)
{
	int		rs ;

	memset(op,0,sizeof(SFILL)) ;
	op->ofp = ofp ;
	op->indent = indent ;

	rs = fifostr_start(&op->sq) ;

	return rs ;
}
/* end subroutine (sfill_start) */


int sfill_finish(SFILL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = fifostr_finish(&op->sq) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (sfill_finish) */


int sfill_remaining(SFILL *op)
{
	int		rs = op->clen ;
	return rs ;
}
/* end subroutine (sfill_finish) */


int sfill_proc(SFILL *op,int olinelen,cchar *linebuf,int linelen)
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		wlen = 0 ;
	const char	*sp, *cp ;

#if	CF_DEBUGS
	debugprintf("sfill_proc: line=>%t<\n",
		linebuf,strlinelen(linebuf,linelen,60)) ;
#endif

	sp = linebuf ;
	sl = linelen ;
	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    rs = fifostr_add(&op->sq,cp,cl) ;

	    op->clen += (cl + 1) ;

	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sfill_proc: i clen=%u\n",op->clen) ;
#endif

	if (rs >= 0) {

	    while ((rs >= 0) && (op->clen > olinelen)) {

	        rs = sfill_wline(op,olinelen) ;
	        wlen += rs ;

	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("sfill_proc: d clen=%u\n",op->clen) ;
#endif

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (sfill_proc) */


int sfill_wline(SFILL *op,int olinelen)
{
	bfile		*ofp = op->ofp ;
	int		rs = SR_OK ;
	int		cl ;
	int		c_words = 0 ;
	int		wlen = 0 ;
	const char	*cp ;
	char		wordbuf[WORDBUFLEN + 1] ;

	if (op->indent > 0) {
	    rs = bwriteblanks(ofp,op->indent) ;
	    wlen += rs ;
	}

	while ((rs >= 0) && (wlen < olinelen)) {
	    cp = wordbuf ;
	    cl = fifostr_headread(&op->sq,wordbuf,WORDBUFLEN) ;
	    if (cl == SR_NOENT) break ;
	    rs = cl ;
	    if (rs < 0) break ;

	    if ((wlen + (cl + 1)) > olinelen)
	        break ;

	    if (c_words++ > 0) {
	        rs = bputc(ofp,' ') ;
	        wlen += 1 ;
	    }

	    if (rs >= 0) {
	        rs = bwrite(ofp,cp,cl) ;
	        wlen += rs ;
	    }

#if	CF_DEBUGS
	    debugprintf("sfill_wline: bwrite() rs=%d clen=%d wlen=%d\n",
	        rs,op->clen,wlen) ;
#endif

	    if (rs >= 0) {
	        rs = fifostr_remove(&op->sq,NULL,0) ;
	    }

	    if (rs < 0) break ;
	} /* end while (getting words) */

	if (rs >= 0) {
	    rs = bputc(ofp,'\n') ;
	    wlen += 1 ;
	}

	if (rs >= 0) {
	    op->clen -= wlen ;
	}

#if	CF_DEBUGS
	debugprintf("sfill_wline: ret rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (sfill_wline) */


