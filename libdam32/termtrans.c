/* termtrans */
/* lang=C++98 */

/* terminal-character-translation management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debug print-outs */


/* revision history:

	= 1998-02-01, David Morano
	This object module was originally written.

*/

/* Copyright © 1998 David Morano.  All rights reserved. */

/*******************************************************************************

	We translate UCS characters (in 'wchar_t' form) to byte sequences for
	output to a terminal (specified).


*******************************************************************************/


#define	TERMTRANS_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vector>
#include	<string>
#include	<new>

#include	<vsystem.h>
#include	<ascii.h>
#include	<ansigr.h>
#include	<buffer.h>
#include	<findbit.h>
#include	<localmisc.h>

#include	"termtrans.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#define	TERMTRANS_FCS		"UCS-4"
#define	TERMTRANS_TCS		"ISO8859-1"
#define	TERMTRANS_DEBFNAME	"termtrans.deb"

#ifdef	_BIG_ENDIAN
#define	TERMTRANS_SUF		"BE"
#else
#define	TERMTRANS_SUF		"LE"
#endif

#define	TERMTRANS_ENDIAN	TERMTRANS_FCS TERMTRANS_SUF

#undef	GCH
#define	GCH			struct termtrans_gch

#undef	SCH
#define	SCH			struct termtrans_sch

#undef	LINE
#define	LINE			struct termtrans_line

#ifndef	CSNLEN
#define	CSNLEN		MAXNAMELEN
#endif

#undef	GHBUFLEN
#define	GHBUFLEN	20	/* should be large enough to hold ~10 */

#undef	DBUFLEN
#define	DBUFLEN		((4*GHBUFLEN) + 5) /* each is 3 decimal digits + ';' */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#undef	OUTBUFLEN
#define	OUTBUFLEN	(10 * LINEBUFLEN)

/* terminal-type attributes */

/* basic capability attributes */
#define	TA_MNONE	0		/* no capabilities */
#define	TA_MBOLD	(1<<0)		/* supports BOLD */
#define	TA_MUNDER	(1<<1)		/* supports UNDER */
#define	TA_MBLINK	(1<<2)		/* supports BLINK */
#define	TA_MREV		(1<<3)		/* supports reverse-video */
#define	TA_MOFFIND	(1<<4)		/* supports individual-off */
#define	TA_MHIGH	(1<<5)		/* supports double high */
#define	TA_MWIDE	(1<<6)		/* supports double wide */
#define	TA_MFT2		(1<<7)		/* supports font-2 (SS2) */
#define	TA_MFT3		(1<<8)		/* supports font-3 (SS3) */

/* derived combinations */
#define	TA_MBASE	(TA_MBOLD | TA_MUNDER | TA_MBLINK)
#define	TA_MDOUBLE	(TA_MHIGH | TA_MWIDE)
#define	TA_MFONTS	(TA_MFT2 | TA_MFT3)
#define	TA_MALL		(TA_MBASE|TA_MOFFIND|TA_MDOUBLE|TA_MREV|TA_MFONTS)

/* source graphic rendition specifier characters */

#define	GRCH_BOLD	'*'		/* bold */
#define	GRCH_UNDER	'_'		/* underline */
#define	GRCH_BLINK	':'		/* blinking */
#define	GRCH_REV	'`'		/* reverse-video */
#define	GRCH_HIGH	'|'		/* double-high */
#define	GRCH_WIDE	'-'		/* double-wide */

/* our graphic renditions */

#define	GR_VBOLD	0		/* indicate BOLD */
#define	GR_VUNDER	1		/* indicate UNDER */
#define	GR_VBLINK	2		/* indicate BLINK */
#define	GR_VREV		3		/* reverse-video */
#define	GR_VHIGH	4		/* double high */
#define	GR_VWIDE	5		/* double wide */
#define	GR_VOVERLAST	6		/* over-last */

#define	GR_MBOLD	(1<<GR_VBOLD)	/* indicate BOLD */
#define	GR_MUNDER	(1<<GR_VUNDER)	/* indicate UNDER */
#define	GR_MBLINK	(1<<GR_VBLINK)	/* indicate BLINK */
#define	GR_MREV		(1<<GR_VREV)	/* reverse-video */
#define	GR_MHIGH	(1<<GR_VHIGH)	/* double high */
#define	GR_MWIDE	(1<<GR_VWIDE)	/* double wide */


/* default name spaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	snsd(char *,int,const char *,uint) ;
extern "C" int	snsdd(char *,int,const char *,uint) ;
extern "C" int	sncpy1(char *,int,const char *) ;
extern "C" int	sncpy2(char *,int,const char *,const char *) ;
extern "C" int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern "C" int	sncpy1w(char *,int,const char *,int) ;
extern "C" int	mkpath2(char *,const char *,const char *) ;
extern "C" int	mkpath3(char *,const char *,const char *,const char *) ;
extern "C" int	mkpath2w(char *,const char *,const char *,int) ;
extern "C" int	termconseq(char *,int,int,int,int,int,int) ;
extern "C" int	isprintlatin(int) ;
extern "C" int	iceil(int,int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	nprintf(const char *,const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strwset(char *,int,int) ;
extern "C" char	*strnchr(const char *,int,int) ;
extern "C" char	*strwcmp(const char *,const char *,int) ;


/* external variables */


/* local structures */

struct termtrans_terminfo {
	const char	*name ;
	int		attr ;
} ;

struct termtrans_sch {
	uchar		ch1, ch2, ft, ch ;
} ;

struct termtrans_gch {
	uchar		gr ;
	uchar		ft ;
	uchar		ch ;
	termtrans_gch(int i=0) : gr(i), ft(i), ch(i) { } ;
	termtrans_gch(int ngr,int nft,int nch) : gr(ngr), ft(nft), ch(nch) { } ;
	termtrans_gch &set(int i=0) {
		gr = i ;
		ft = i ;
		ch = i ;
		return (*this) ;
	} ;
	termtrans_gch &set(int ngr,int nft,int nch) {
		gr = ngr ;
		ft = nft ;
		ch = nch ;
		return (*this) ;
	} ;
	termtrans_gch &set(uchar ngr,uchar nft,uchar nch) {
		gr = ngr ;
		ft = nft ;
		ch = nch ;
		return (*this) ;
	} ;
} ;

class termtrans_line {
	const char	*lbuf ;
	int		llen ;
} ;


/* forward references */

static int	termtrans_process(TERMTRANS *,const wchar_t *,int) ;
static int	termtrans_procline(TERMTRANS *,char *,int,const wchar_t *,int) ;
static int	termtrans_proclinepost(TERMTRANS *,const char *,int) ;
static int	termtrans_loadline(TERMTRANS *,int,int) ;

static int	termtrans_loadgr(TERMTRANS *,string &,int,int) ;
static int	termtrans_loadch(TERMTRANS *,string &,int,int) ;
static int	termtrans_loadcs(TERMTRANS *,string &,int,const char *,int) ;

static int	gettermattr(const char *,int) ;
static int	wsgetline(const wchar_t *,int) ;
static int	isspecial(SCH *,uchar,uchar) ;


/* local variables */

static const struct termtrans_terminfo	terms[] = {
	{ "sun", 0 },
	{ "ansi", 0 },
	{ "xterm", TA_MBASE },
	{ "xterm-color", TA_MBASE },
	{ "screen", (TA_MBASE | TA_MREV | TA_MOFFIND | TA_MFONTS) },
	{ "vt100", (TA_MBASE | TA_MDOUBLE) },
	{ "vt101", (TA_MBASE | TA_MDOUBLE) },
	{ "vt102", (TA_MBASE | TA_MDOUBLE) },
	{ "vt220", (TA_MBASE | TA_MDOUBLE) },
	{ "vt220", (TA_MBASE | TA_MDOUBLE) },
	{ "vt240", (TA_MBASE | TA_MDOUBLE) },
	{ "vt320", TA_MALL },
	{ "vt330", TA_MALL },
	{ "vt340", TA_MALL },
	{ "vt420", TA_MALL },
	{ "vt430", TA_MALL },
	{ "vt440", TA_MALL },
	{ "vt520", TA_MALL },
	{ "vt530", TA_MALL },
	{ "vt540", TA_MALL },
	{ NULL, 0 }
} ;

static const struct termtrans_sch	specials[] = {
	{ UC('x'), '*', 2, 0x60 },
	{ UC('×'), '*', 2, 0x60 },
	{ 'X', '*', 2, 0x60 },
	{ '=', '/', 2, UC(0x7c) },
	{ '/', '=', 2, UC(0x7c) },
	{ '<', '=', 2, UC(0x79) },
	{ '>', '=', 2, UC(0x7a) },
	{ '<', '_', 2, UC(0x79) },
	{ '>', '_', 2, UC(0x7a) },
	{ '<', '-', 2, UC(0x79) },
	{ '>', '-', 2, UC(0x7a) },
	{ '|', '-', 2, UC(0x6e) },
	{ '|', '|', 2, UC(0x78) },
	{ UC('¯'), '_', 2, UC(0x61) },
	{ 'n', UC('¯'), 3, UC(0x50) },
	{ 'n', '-', 3, UC(0x70) },
	{ '{', '|', 3, UC(0x2f) },
	{ '}', '|', 3, 0x30 },
	{ 'Y', '|', 3, 0x51 },
	{ 'y', '|', 3, UC(0x71) },
	{ 'f', '-', 3, 0x76 },
	{ '=', '-', 3, 0x4f },
	{ 'O', UC('·'), 3, 0x4a },
	{ '~', '-', 3, 0x49 },
	{ 'o', 'c', 3, 0x41 },
	{ ':', '.', 3, 0x40 },
	{ 'e', '-', 3, 0x65 },
	{ '-', ':', 3, 0x43 },
	{ 0, 0, 0, 0 }
} ;


/* exported subroutines */


int termtrans_start(TERMTRANS *op,cchar *pr,cchar *tstr,int tlen,int ncols)
{
	int		rs = SR_OK ;
	vector<GCH>	*cvp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (ncols <= 0) return SR_INVALID ;

	memset(op,0,sizeof(TERMTRANS)) ;
	op->ncols = ncols ;
	op->termattr = gettermattr(tstr,tlen) ;

	if ((cvp = new(nothrow) vector<GCH>) != NULL) {
	    const int	fcslen = CSNLEN ;
	    const char	*fcs = TERMTRANS_FCS ;
	    const char	*suf = TERMTRANS_SUF ;
	    char	fcsbuf[CSNLEN+1] ;
	    op->cvp = (void *) cvp ;
	    if ((rs = sncpy2(fcsbuf,fcslen,fcs,suf)) >= 0) {
	        const char	*tcsp = TERMTRANS_TCS ;
	        if ((rs = uiconv_open(&op->id,tcsp,fcsbuf)) >= 0) {
		    op->magic = TERMTRANS_MAGIC ;
	        }
	    }
	    if (rs < 0) {
		delete cvp ;
		op->cvp = NULL ;
	    }
	} else
	    rs = SR_NOMEM ;

	return rs ;
}
/* end subroutine (termtrans_start) */


int termtrans_finish(TERMTRANS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMTRANS_MAGIC) return SR_NOTOPEN ;

	rs1 = uiconv_close(&op->id) ;
	if (rs >= 0) rs = rs1 ;

	if (op->lvp != NULL) {
	    vector<string>	*lvp = (vector<string> *) op->lvp ;
	    delete lvp ; /* calls all 'string' destructors */
	    op->lvp = NULL ;
	}

	if (op->cvp != NULL) {
	    vector<GCH>	*cvp = (vector<GCH> *) op->cvp ;
	    delete cvp ;
	    op->cvp = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (termtrans_finish) */


int termtrans_load(TERMTRANS *op,const wchar_t *wbuf,int wlen)
{
	vector<string>	*lvp ;
	int		rs = SR_OK ;
	int		ln = 0 ;

#if	CF_DEBUGS
	debugprintf("termtrans_load: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (wbuf == NULL) return SR_FAULT ;

	lvp = (vector<string> *) op->lvp ;
	if (lvp == NULL) {
	    if ((lvp = new(nothrow) vector<string>) != NULL) {
	        op->lvp = lvp ;
	    } else
		rs = SR_NOMEM ;
	}

/* process given string into the staging area */

#if	CF_DEBUGS
	debugprintf("termtrans_load: process wlen=%d\n",wlen) ;
#endif

	if (rs >= 0) {
	    rs = termtrans_process(op,wbuf,wlen) ;
	    ln = rs ;
	}

#if	CF_DEBUGS
	debugprintf("termtrans_load: ret rs=%d ln=%u\n",rs,ln) ;
#endif

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termtrans_load) */


int termtrans_getline(TERMTRANS *op,int i,cchar **lpp)
{
	vector<string>	*lvp ;
	int		rs = SR_OK ;
	int		ll = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lpp == NULL) return SR_FAULT ;

	lvp = (vector<string> *) op->lvp ;
	if (i < lvp->size()) {
	    *lpp = lvp->at(i).c_str() ;
	    ll = lvp->at(i).size() ;
	}

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (termtrans_getline) */


/* private subroutines */


static int termtrans_process(TERMTRANS *op,const wchar_t *wbuf,int wlen)
{
	const int	olen = (3*LINEBUFLEN) ;
	int		rs = SR_OK ;
	int		osize ;
	int		ln = 0 ;
	char		*obuf ;

#if	CF_DEBUGS
	debugprintf("termtrans_process: ent wlen=%d\n",wlen) ;
#endif

	osize = (olen+1) ;
	if ((rs = uc_malloc(osize,&obuf)) >= 0) {
	    int	wl ;

	    while ((rs = wsgetline(wbuf,wlen)) > 0) {
		wl = rs ;
		rs = termtrans_procline(op,obuf,olen,wbuf,wl) ;
		ln += rs ;
		wbuf += wl ;
		wlen -= wl ;
		if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (wlen > 0)) {
		rs = termtrans_procline(op,obuf,olen,wbuf,wlen) ;
		ln += rs ;
	    }

	    uc_free(obuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("termtrans_process: ret rs=%d ln=%u \n",rs,ln) ;
#endif

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termtrans_process) */


static int termtrans_procline(TERMTRANS *op,char *obuf,int olen,
		const wchar_t *wbuf,int wlen)
{
	vector<GCH>	*cvp = (vector<GCH> *) op->cvp ;
	int		rs = SR_OK ;
	int		i ;
	int		ch ;
	int		j = 0 ;
	int		ln = 0 ;
	int		max = 0 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("termtrans_procline: ent wlen=%d\n",wlen) ;
#endif

	cvp->clear() ;

#if	CF_DEBUGS
	debugprintf("termtrans_procline: for-before\n") ;
#endif

	{
	    int		istart = (wlen * sizeof(wchar_t)) ;
	    int		ileft = istart ;
	    int		ostart = olen ;
	    int		oleft ;
	    int		ofill ;
	    const char	*ibp = (const char *) wbuf ;
	    char	*obp = obuf ;
	    oleft = ostart ;
	    rs = uiconv_trans(&op->id,&ibp,&ileft,&obp,&oleft) ;

	    ofill = (ostart-oleft) ;

#if	CF_DEBUGS
	{
		int	ifill = (istart-ileft) ;
		int	wi = (ifill >> 2) ;
	debugprintf("termtrans_procline: uiconv_trans() rs=%d\n",rs) ;
	debugprintf("termtrans_procline: ileft=%lu ifill=%lu\n",
		ileft,ifill) ;
	debugprintf("termtrans_procline: oleft=%lu ofill=%lu\n",
		oleft,ofill) ;
		debugprintf("termtrans_procline: wc[wi]=%08x\n",wbuf[wi]) ;
		if (wi > 0) 
		debugprintf("termtrans_procline: wc[wi-1]=%08x\n",wbuf[wi-1]) ;
	}
#endif

	    if (rs >= 0) {
		int	ol = ofill ;
		rs = termtrans_proclinepost(op,obuf,ol) ;
		ln += rs ;
	    }

	} /* end block */

#if	CF_DEBUGS
	debugprintf("termtrans_procline: ret rs=%d ln=%u \n",rs,ln) ;
#endif

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termtrans_procline) */


static int termtrans_proclinepost(TERMTRANS *op,cchar *obuf,int olen)
{
	vector<GCH>	*cvp = (vector<GCH> *) op->cvp ;
	int		rs = SR_OK ;
	int		i ;
	int		ch ;
	int		j = 0 ;
	int		ln = 0 ;
	int		max = 0 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("termtrans_proclinepost: olen=%d\n",olen) ;
#endif

	cvp->clear() ;

	for (i = 0 ; i < olen ; i += 1) {
	    ch = (obuf[i] & 0xff) ;
#if	CF_DEBUGS
	debugprintf("termtrans_proclinepost: "
		"i=%u switch ch=%c (\\x%02x)\n",i,ch,ch) ;
#endif
	    switch (ch) {
	    case '\r':
		j = 0 ;
		break ;
	    case '\n':
	    case '\v':
	    case '\f':
		rs = termtrans_loadline(op,ln++,max) ;
		len += rs ;
	 	cvp->clear() ;
		j = 0 ;
		max = 0 ;
		break ;
	    case '\b':
		if (j > 0) j -= 1 ;
#if	CF_DEBUGS
				debugprintf("termout: BS\n") ;
#endif
		break ;
	    default:
		{
		    GCH	gch(0) ;
#if	CF_DEBUGS
		        debugprintf("termout: def max=%u j=%u ch=%c(\\x%02x)\n",
			max,j,ch,ch) ;
#endif
		    if (j < max) {
			SCH	sch ;
		        int	ft = 0 ;
			int	pch = (j < max) ? cvp->at(j).ch : 0 ;
		        int	gr = cvp->at(j).gr ;
#if	CF_DEBUGS
		        debugprintf("termout: def j=%u pch=%c(\\x%02x)\n",
				j,pch,pch) ;
		        debugprintf("termout: def ch=%c(\\x%02x)\n",
				ch,ch) ;
#endif
		        switch (pch) {
		        case GRCH_BOLD :
			    if (op->termattr & TA_MBOLD) gr |= GR_MBOLD ;
			    break ;
		        case GRCH_UNDER:
			    if (op->termattr & TA_MUNDER) gr |= GR_MUNDER ;
			    break ;
		        case GRCH_BLINK:
			    if (op->termattr & TA_MBLINK) gr |= GR_MBLINK ;
			    break ;
		        case GRCH_REV:
			    if (op->termattr & TA_MREV) gr |= GR_MREV ;
			    break ;
		        default:
		            if ((pch == GRCH_HIGH) && (ch == '#')) {
			        if (op->termattr & TA_MHIGH) gr |= GR_MHIGH ;
			        ch = 0 ;
#if	CF_DEBUGS
				debugprintf("termout: DoubleHigh(pre)\n") ;
#endif
		            } else if ((pch == GRCH_WIDE) && (ch == '#')) {
			        if (op->termattr & TA_MWIDE) gr |= GR_MWIDE ;
			        ch = 0 ;
#if	CF_DEBUGS
				debugprintf("termout: DoubleWide(pre)\n") ;
#endif
		            } else if (pch == ch) {
			        if (op->termattr & TA_MBOLD) gr |= GR_MBOLD ;
#if	CF_DEBUGS
				debugprintf("termout: bold(pre)\n") ;
#endif
			    } else if (isspecial(&sch,pch,ch)) {
				ft = sch.ft ;
				ch = sch.ch ;
				switch (ft) {
				case 2:
			            if (! (op->termattr & TA_MFT2)) {
					ft = 0 ;
					ch = '¿' ;
				    }
				    break ;
				case 3:
			            if (! (op->termattr & TA_MFT3)) {
					ft = 0 ;
					ch = '¿' ;
				    }
				    break ;
				} /* end switch */
			    } /* end if */
			    break ;
		        case 0:
			    break ;
		        } /* end switch */
			gch.set(gr,ft,ch) ;
	                cvp->at(j) = gch ;
	                j += 1 ;
		    } else {
#if	CF_DEBUGS
		        debugprintf("termout: add j=%u ch=%c(\\x%02x)\n",
				j,ch,ch) ;
#endif
			gch.ch = ch ;
	                cvp->push_back(gch) ;
	                j += 1 ;
		    } /* end if */
		} /* end block */
		break ;
	    } /* end switch */
	    if (j > max) max = j ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("termtrans_proclinepost: mid rs=%d max=%u\n",rs,max) ;
#endif

	if ((rs >= 0) && (max > 0)) {
	    rs = termtrans_loadline(op,ln++,max) ;
	    len += rs ;
	}

#if	CF_DEBUGS
	debugprintf("termtrans_proclinepost: ret rs=%d ln=%u \n",rs,ln) ;
#endif

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termtrans_proclinepost) */


static int termtrans_loadline(TERMTRANS *op,int ln,int max)
{
	vector<GCH>	*cvp = (vector<GCH> *) op->cvp ;
	vector<string>	*lvp = (vector<string> *) op->lvp ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("termtrans_loadline: ent ln=%u max=%u\n",ln,max) ;
#endif

	lvp->resize(ln+1) ;		/* instantiates new 'string' */
	{
	    int		ft, ch, gr ;
	    int		i ;
	    int		pgr = 0 ;
	    string	&line = lvp->at(ln) ;
#if	CF_DEBUGS
	debugprintf("termtrans_loadline: line.reserve()\n") ;
#endif
	    line.reserve(120) ;
#if	CF_DEBUGS && 0
	debugprintf("termtrans_loadline: line.clear()\n") ;
#endif
	    line.clear() ;
#if	CF_DEBUGS && 0
	debugprintf("termtrans_loadline: for-before\n") ;
#endif
	    for (i = 0 ; (rs >= 0) && (i < max) ; i += 1) {
		ft = cvp->at(i).ft ;
		ch = cvp->at(i).ch ;
		gr = cvp->at(i).gr ;

#if	CF_DEBUGS
	debugprintf("termtrans_loadline: _loadgr() ch=%c(\\x%02x)\n",ch,ch) ;
#endif

		rs = termtrans_loadgr(op,line,pgr,gr) ;
		len += rs ;

#if	CF_DEBUGS
	debugprintf("termtrans_loadline: _loadgr() rs=%d\n",rs) ;
#endif

		if ((rs >= 0) && (ch > 0)) {
		    rs = termtrans_loadch(op,line,ft,ch) ;
		    len += rs ;
		}

#if	CF_DEBUGS
	debugprintf("termtrans_loadline: _loadch() rs=%d\n",rs) ;
#endif

		pgr = gr ;
	    } /* end for */
	    if ((rs >= 0) && (pgr != 0)) {
		rs = termtrans_loadgr(op,line,pgr,0) ;
		len += rs ;
	    }
	} /* end block */

/* done */

#if	CF_DEBUGS
	debugprintf("termtrans_loadline: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termtrans_loadline) */


static int termtrans_loadgr(TERMTRANS *op,string &line,int pgr,int gr)
{
	const int	grmask = ( GR_MBOLD| GR_MUNDER| GR_MBLINK| GR_MREV) ;
	int		rs = SR_OK ;
	int		ogr = pgr ;
	int		bgr = pgr ;
	int		cc ;
	int		n, i ;
	int		size ;
	int		gl = 0 ;
	int		len = 0 ;
	char		*grbuf ;

	n = flbsi(grmask) + 1 ;

	size = ((2*n)+1+1) ; /* size according to algorithm below */
	if ((grbuf = new(nothrow) char[size]) != NULL) {

	    bgr &= grmask ;
	    ogr = (bgr & (~ gr) & grmask) ;
	    if (ogr != (bgr & grmask)) { /* partial gr-off */
	        if (op->termattr & TA_MOFFIND) {
		    for (i = 0 ; i < n ; i += 1) {
		        if ((ogr>>i)&1) {
			    switch (i) {
			    case GR_VBOLD:
			        cc = ANSIGR_OFFBOLD ;
			        break ;
			    case GR_VUNDER:
			        cc = ANSIGR_OFFUNDER ;
			        break ;
			    case GR_VBLINK:
			        cc = ANSIGR_OFFBLINK ;
			        break ;
			    case GR_VREV:
			        cc = ANSIGR_OFFREV ;
			        break ;
			    } /* end switch */
			    grbuf[gl++] = cc ;
			    bgr &= (~(1<<i)) ;
		        } /* end if */
		    } /* end for */
	        } else {
		    grbuf[gl++] = ANSIGR_OFFALL ;
		    bgr &= (~ grmask) ;
	        } /* end if */
	    } /* end if (partial gr-off) */
    
	    ogr = (bgr & (~ gr) & grmask) ;
	    if (ogr & grmask) { /* all OFF-GR */
		grbuf[gl++] = ANSIGR_OFFALL ;
		bgr &= (~ grmask) ;
	    } /* end if */
    
	    ogr = (gr & (~ bgr) & grmask) ; /* ON-GR */
	    if (ogr) {
		for (i = 0 ; i < n ; i += 1) {
		    if ((ogr>>i)&1) {
			switch (i) {
			case GR_VBOLD:
			    cc = ANSIGR_BOLD ;
			    break ;
			case GR_VUNDER:
			    cc = ANSIGR_UNDER ;
			    break ;
			case GR_VBLINK:
			    cc = ANSIGR_BLINK ;
			    break ;
			case GR_VREV:
			    cc = ANSIGR_REV ;
			    break ;
			} /* end switch */
			grbuf[gl++] = cc ;
			bgr &= (~(1<<i)) ;
		    } /* end if */
		} /* end for */
	    } /* end if (gr-on) */
    
	    if (rs >= 0) {
	        rs = termtrans_loadcs(op,line,'m',grbuf,gl) ;
	        len = rs ;
	    }
    
	    delete[] grbuf ;
	} else
	    rs = SR_NOMEM ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termtrans_loadgr) */


static int termtrans_loadcs(TERMTRANS *op,string &line,int n,cchar *pp,int pl)
{
	const int	dlen = DBUFLEN ;
	int		rs = SR_OK ;
	int		ml ;
	int		dl ;
	int		i = 0 ;
	int		len = 0 ;
	char		dbuf[DBUFLEN+1] ;

	if (op == NULL) return SR_FAULT ;

	while ((rs >= 0) && (i < pl)) {
	    int	a1 = -1 ;
	    int	a2 = -1 ;
	    int	a3 = -1 ;
	    int	a4 = -1 ;
	    ml = MIN(4,(pl-i)) ;
	    switch (ml) {
/* FALLTHROUGH */
	    case 4:
		a4 = pp[i+3] ;
/* FALLTHROUGH */
	    case 3:
		a3 = pp[i+2] ;
/* FALLTHROUGH */
	    case 2:
		a2 = pp[i+1] ;
/* FALLTHROUGH */
	    case 1:
		a1 = pp[i+0] ;
/* FALLTHROUGH */
	    case 0:
		break ;
	    } /* end switch */
	    if (ml > 0) {
		rs = termconseq(dbuf,dlen,n,a1,a2,a3,a4) ;
		dl = rs ;
		i += ml ;
		if (rs >= 0) {
		    len += dl ;
		    line.append(dbuf,dl) ;
		}
	    }
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termtrans_loadcs) */


static int termtrans_loadch(TERMTRANS *op,string &line,int ft,int ch)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (ch > 0) {
	    int	sch = 0 ;

	    if (ft > 0) {
	        switch (ft) {
	        case 1:
			sch = CH_SO ;
			break ;
	        case 2:
			sch = CH_SS2 ;
			break ;
	        case 3:
			sch = CH_SS3 ;
			break ;
	        } /* end witch */
	        line.push_back(sch) ;
	        len += 1 ;
	    } /* end if (font handling) */
    
	    line.push_back(ch) ;
	    len += 1 ;

	    if (ft == 1) {
		sch = CH_SI ;
		line.push_back(sch) ;
		len += 1 ;
	    }

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termtrans_loadch) */


static int gettermattr(cchar *tstr,int tlen)
{
	int		ta = 0 ;
	const char	*np ;

	if (tstr != NULL) {
	    int	i ;

	    if (tlen < 0) tlen = strlen(tstr) ;

	    for (i = 0 ; terms[i].name != NULL ; i += 1) {
	        np = terms[i].name ;
	        if (strwcmp(np,tstr,tlen) == 0) {
		    ta = terms[i].attr ;
		    break ;
	        }
	    } /* end for */

	} /* end if (non-NULL terminal-string) */

	return ta ;
}
/* end subroutine (gettermattr) */


static int wsgetline(const wchar_t *wbuf,int wlen)
{
	int		wl ;
	int		f = FALSE ;

	for (wl = 0 ; wl < wlen ; wl += 1) {
	    f = ((wbuf[wl] == CH_NL) || (wbuf[wl] == CH_FF)) ;
	    if (f) break ;
	} /* end for */
	if (f) wl += 1 ;

	return (f) ? wl : 0 ;
}
/* end subroutine (wsgetline) */


static int isspecial(SCH *scp,uchar ch1,uchar ch2)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; specials[i].ch1 > 0 ; i += 1) {
	    f = ((specials[i].ch1 == ch1) && (specials[i].ch2 == ch2)) ;
	    if (f) break ;
	} /* end for */

	if (f) {
	    scp->ft = specials[i].ft ;
	    scp->ch = specials[i].ch ;
	}

	return f ;
}
/* end subroutine (isspecial) */


