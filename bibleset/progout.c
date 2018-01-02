/* progout */

/* perform some output processsing activities */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2009-04-01, David A­D­ Morano
	This subroutine was written as an enhancement for adding back-matter
	(end pages) to the output document.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates header and foot specifications.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<tmtime.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progoff.h"
#include	"multiout.h"		/* the "MO" object */


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdeci(char *,int,int) ;
extern int	ctdecui(char *,int,uint) ;
extern int	bprintlines(bfile *,int,const char *,int) ;
extern int	bprintline(bfile *,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern int	progfront(PROGINFO *,bfile *) ;

extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procoutbeginer(PROGINFO *,bfile *) ;
static int	progouttc(PROGINFO *,bfile *,int) ;


/* local variables */


/* exported subroutines */


int progoutmtheader(PROGINFO *pip,bfile *ofp)
{
	PROGINFO_POUT	*pop = &pip->pout ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (! pip->f.maintextheader) {
	    cchar	*pagetitle = pop->hf_pagetitle ;
	    cchar	*pageloc = pop->hf_pagelocation ;
	    pip->f.maintextheader = TRUE ;

	    if (rs >= 0) {
	        rs = progoffhf(pip,ofp,"PH",NULL,NULL,NULL) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = progoffhf(pip,ofp,"EH",pageloc,pagetitle,NULL) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = progoffhf(pip,ofp,"OH",NULL,pagetitle,pageloc) ;
	        wlen += rs ;
	    }

	} /* end if (needed) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutmtheader) */


int progoutmtfooter(PROGINFO *pip,bfile *ofp)
{
	PROGINFO_POUT	*pop = &pip->pout ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (! pip->f.maintextfooter) {
	    pip->f.maintextfooter = TRUE ;

	    if (rs >= 0) {
	        rs = progoffhf(pip,ofp,"EF",
	            pop->hf_pagelocation,pop->hf_pagetitle,pop->hf_pageinfo) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = progoffhf(pip,ofp,"OF",
	            pop->hf_pageinfo,pop->hf_pagetitle,pop->hf_pagelocation) ;
	        wlen += rs ;
	    }

	} /* end if (needed) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutmtfooter) */


int progoutbegin(PROGINFO *pip,bfile *ofp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*fmt ;

	if ((rs >= 0) && (pip->pout.pageinfo[0] == '\0')) {
	    rs = procoutbeginer(pip,ofp) ;
	    wlen += rs ;
	}

#if	CF_TROFFSETLL
	if (rs >= 0) {
	    rs = progoffsrs(pip,ofp,"linelen","ll","6.5i") ;
	    wlen += rs ;
	}
#endif /* CF_TROFFSETLL */

/* set the specified font (as necessary) into the display-evironment */

	if ((rs >= 0) && (pip->ffi > 0)) {
	    if (rs >= 0) {
	        rs = bprintf(ofp,".DS\n") ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        rs = progoffsetbasefont(pip,ofp) ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        rs = bprintf(ofp,".DE\n") ;
	        wlen += rs ;
	    }
	} /* end if */

/* no page-headers on front-matter pages (dy default) */

	if (rs >= 0) {
	    rs = progoffhf(pip,ofp,"PH",NULL,NULL,NULL) ;
	    wlen += rs ;
	}

/* we optionally have a page-footer w/ the page-number */

	if ((rs >= 0) && pip->f.pagenums) {
	    rs = progoffhf(pip,ofp,"PF","",pip->pout.hf_pagenum,"") ;
	    wlen += rs ;
	}

/* end of general document initialization */

	if (rs >= 0) {
	    fmt = "%s end of general initialization\n" ;
	    rs = bprintf(ofp,fmt,pip->troff.linecomment) ;
	    wlen += rs ;
	}

/* should we include any front-matter? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/printfront: f_fm=%u frontfname=%s\n",
	        pip->f.frontmatter,pip->frontfname) ;
#endif

	if ((rs >= 0) && pip->f.frontmatter) {
	    rs = progfront(pip,ofp) ;
	    wlen += rs ;
	}

/* start in with the main text setup */

	if (rs >= 0) {
	    fmt = "%s main-text initialization\n" ;
	    rs = bprintf(ofp,fmt,pip->troff.linecomment) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && pip->have.hyphenate) {
	    rs = progoffsrn(pip,ofp,"hyphenation","Hy",pip->f.hyphenate) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && pip->have.ha) {
	    rs = progoffsrn(pip,ofp,"ha","ha",pip->f.ha) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffdss(pip,ofp,"bT",pip->pagetitle) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffdss(pip,ofp,"bI",pip->pout.pageinfo) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffdss(pip,ofp,"bB","Book ?") ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffdsn(pip,ofp,"bC",0) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffdsn(pip,ofp,"bV",0) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutbegin) */


int progoutend(PROGINFO *pip,bfile *ofp)
{
	int		rs = SR_OK ;
	int		ncols ;
	int		wlen = 0 ;
	const char	*fmt ;

/* back-matter begin */

	if (rs >= 0) {
	    fmt = "%s back-matter begin\n" ;
	    rs = bprintf(ofp,fmt,pip->troff.linecomment) ;
	    wlen += rs ;
	}

/* turn off odd and even page headers */

	if (rs >= 0) {
	    rs = progoffhf(pip,ofp,"OH",NULL,NULL,NULL) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffhf(pip,ofp,"EH",NULL,NULL,NULL) ;
	    wlen += rs ;
	}

/* skip to the next page */

	if (rs >= 0) {
	    rs = bprintf(ofp,".SK 1\n") ;
	    wlen += rs ;
	}

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = bprintf(ofp,".1C\n") ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,"\\&   \\&\n") ;
	    wlen += rs ;
	}
#endif /* COMMENT */

/* turn off odd and even page footers */

	if (rs >= 0) {
	    rs = progoffhf(pip,ofp,"OF", NULL,NULL,NULL) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffhf(pip,ofp,"EF", NULL,NULL,NULL) ;
	    wlen += rs ;
	}

/* any table-of-contents */

	if ((rs >= 0) && pip->f.tc) {
	    ncols = (pip->c.book <= 33) ? 1 : 2 ;
	    rs = progouttc(pip,ofp,ncols) ;
	    wlen += rs ;
	}

/* skip to the next even page */

#ifdef	COMMENT /* there is no such macro to do so! */
	if (rs >= 0) {
	    rs = bprintf(ofp,".EP\n") ;
	    wlen += rs ;
	}
#endif /* COMMENT */

/* back-matter end */

	if (rs >= 0) {
	    rs = bprintf(ofp,"%s back-matter end\n",
	        pip->troff.linecomment) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutend) */


/* local subroutines */


int procoutbeginer(PROGINFO *pip,bfile *ofp)
{
	PROGINFO_POUT	*pop = &pip->pout ;
	int		rs = SR_OK ;

	if (ofp == NULL) return SR_FAULT ;

	if (pop->pageinfo[0] == '\0') {
	    if (pip->ff != NULL) {
	        TMTIME	tmt ;
	        cchar	*ff ;
	        char	tmpbuf[TIMEBUFLEN + 1] ;

	        ff = ((pip->ff != NULL) ? pip->ff : "D") ;
	        strwcpyuc(tmpbuf,ff,MIN(2,TIMEBUFLEN)) ;

/* create the page-information */

	        tmtime_localtime(&tmt,pip->daytime) ;

	        bufprintf(pop->pageinfo,TIMEBUFLEN,
	            "%s %02u%02u%02u %s %02u:%02u",
	            pip->pub,
	            (tmt.year % NYEARS_CENTURY),
	            (tmt.mon + 1),
	            tmt.mday,
	            tmpbuf,
	            pip->ps,pip->vs) ;

	        bufprintf(pop->hf_pageinfo,HDRBUFLEN,
	            "\\s-2%s%sbI %s\\nP\\s+2",
	            pip->troff.slash3,
	            pip->troff.int2,
	            pip->troff.slash3) ;

	        bufprintf(pop->hf_pagetitle,HDRBUFLEN,
	            "%s%sbT",
	            pip->troff.slash3,
	            pip->troff.int2) ;

	        bufprintf(pop->hf_pagelocation,HDRBUFLEN,
	            "%s%sbB %s%sbC:%s%sbV",
	            pip->troff.slash3,
	            pip->troff.int2,
	            pip->troff.slash3,
	            pip->troff.int2,
	            pip->troff.slash3,
	            pip->troff.int2) ;

	        bufprintf(pop->hf_pagenum,HDRBUFLEN,
	            "%s\\nP",
	            pip->troff.slash3) ;

	    } else {
	        rs = SR_NOANODE ;
	    }
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (procoutbeginer) */


static int progouttc(PROGINFO *pip,bfile *ofp,int ncols)
{
	struct troffstrs	*tsp = &pip->troff ;
	MULTIOUT	mo ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#ifdef	COMMENT
	rs = bufprintf(lbuf,LINEBUFLEN,"%s",pip->word[word_bookindex]) ;
#else
	rs = sncpy1(lbuf,LINEBUFLEN,pip->word[word_bookindex]) ;
#endif

	if (rs >= 0) {
	    rs = progoffcomment(pip,ofp,"toc begin",-1) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    if ((rs = mo_start(&mo,ofp)) >= 0) {
		cchar	*fmt ;

	        mo_printf(&mo,".OP\n") ;
	        mo_printf(&mo,".1C\n") ;
	        mo_printf(&mo,".ce 1\n") ;
		fmt = "\\s+2%s%s%s\\s0\n" ;
	        mo_printf(&mo,fmt, tsp->infont_b, lbuf, tsp->infont_p) ;
	        mo_printf(&mo,".SP 2\n") ;
	        mo_printf(&mo,".2C\n") ;

	        rs1 = mo_finish(&mo) ;
	        if (rs >= 0) rs = rs1 ;
	        wlen += rs1 ;
	    } /* end if (mo) */
	} /* end if */

	if (rs >= 0) {
	    rs = progofftcmk(pip,ofp,ncols) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,".2C\n") ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = progoffcomment(pip,ofp,"toc end",-1) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procouttc) */


