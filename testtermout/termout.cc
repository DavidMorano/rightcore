/* termout */
/* lang=C++98 */

/* perform terminal outputting */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This object module was originally written to create a logging mechanism
	for PCS application programs.  This is a second go at this object.  The
	first was written in C89.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a module that operates on termials (to be determined) for the
	purposes of putting out small messages to them.  Subroutines in this
	module are:

		termout_start
		termout_load
		termout_get
		termout_finish

	Implementation note:

	Note (on the C++ language):
        Note how we had to create a mess out of our character database below
        because of how f**ked up the C++ language is. There is no way to provide
        an unsigned char literal in the language without an incredible mess!
        This does not occur with the regular C language.


*******************************************************************************/


#define	TERMOUT_MASTER	0		/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<initializer_list>
#include	<vector>
#include	<string>
#include	<new>

#include	<vsystem.h>
#include	<ascii.h>
#include	<ansigr.h>
#include	<buffer.h>
#include	<findbit.h>		/* for |flbsi(3dam)| */
#include	<localmisc.h>

#include	"termout.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(ch)	((ch) & UCHAR_MAX)
#endif

#undef	GCH
#define	GCH		struct termout_gch

#undef	SCH
#define	SCH		struct termout_sch

#undef	LINE
#define	LINE		struct termout_line

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

struct termout_terminfo {
	const char	*name ;
	int		attr ;
} ;

struct termout_sch {
	uchar		ch1, ch2, ft, ch ;
} ;

struct termout_gch {
	uchar		gr ;
	uchar		ft ;
	uchar		ch ;
	termout_gch(int i=0) : gr(i), ft(i), ch(i) { } ;
	termout_gch(int ngr,int nft,int nch) : gr(ngr), ft(nft), ch(nch) { } ;
	termout_gch &operator = (int a) {
	    gr = 0 ;
	    ft = 0 ;
	    ch = a ;
	    return (*this) ;
	} ;
	termout_gch(initializer_list<int> &list) {
	    load(list) ;
	} ;
	termout_gch &operator = (initializer_list<int> &list) {
	    load(list) ;
	    return (*this) ;
	} ;
	termout_gch &set(int i=0) {
	    gr = i ;
	    ft = i ;
	    ch = i ;
	    return (*this) ;
	} ;
	termout_gch &set(int ngr,int nft,int nch) {
	    gr = ngr ;
	    ft = nft ;
	    ch = nch ;
	    return (*this) ;
	} ;
	termout_gch &set(uchar ngr,uchar nft,uchar nch) {
	    gr = ngr ;
	    ft = nft ;
	    ch = nch ;
	    return (*this) ;
	} ;
	void load(initializer_list<int> &list) {
	    int	i = 0 ;
	    for (auto a : list) {
		if (i++ >= 3) break ;
	        switch (i) {
		case 0:
		    gr = a ;
		    break ;
		case 1:
		    ft = a ;
		    break ;
		case 2:
		    ch = a ;
		    break ;
		} /* end switch */
	    } /* end for */
	} ; /* end method (load) */
} ;

class termout_line {
	const char	*lbuf ;
	int		llen ;
} ;


/* forward references */

static int	termout_process(TERMOUT *,const char *,int) ;
static int	termout_loadline(TERMOUT *,int,int) ;

static int	termout_loadgr(TERMOUT *,string &,int,int) ;
static int	termout_loadch(TERMOUT *,string &,int,int) ;
static int	termout_loadcs(TERMOUT *,string &,int,const char *,int) ;

static int	gettermattr(const char *,int) ;
static int	isspecial(SCH *,uchar,uchar) ;


/* local variables */

static const struct termout_terminfo	terms[] = {
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

static const struct termout_sch	specials[] = {
	{ '1', '4', 0, UC('¼') },
	{ '1', '2', 0, UC('½') },
	{ '3', '4', 0, UC('¾') },
	{ CH_BQUOTE, 'A', 0, UC('À') },
	{ CH_BQUOTE, 'E', 0, UC('È') },
	{ CH_BQUOTE, 'I', 0, UC('Ì') },
	{ CH_BQUOTE, 'O', 0, UC('Ò') },
	{ CH_BQUOTE, 'U', 0, UC('Ù') },
	{ CH_BQUOTE, 'a', 0, UC('à') },
	{ CH_BQUOTE, 'e', 0, UC('è') },
	{ CH_BQUOTE, 'i', 0, UC('ì') },
	{ CH_BQUOTE, 'o', 0, UC('ò') },
	{ CH_BQUOTE, 'u', 0, UC('ù') },
	{ CH_SQUOTE, 'A', 0, UC('Á') },
	{ CH_SQUOTE, 'E', 0, UC('É') },
	{ CH_SQUOTE, 'I', 0, UC('Í') },
	{ CH_SQUOTE, 'O', 0, UC('Ó') },
	{ CH_SQUOTE, 'U', 0, UC('Ú') },
	{ CH_SQUOTE, 'Y', 0, UC('Ý') },
	{ CH_SQUOTE, 'a', 0, UC('á') },
	{ CH_SQUOTE, 'e', 0, UC('é') },
	{ CH_SQUOTE, 'i', 0, UC('í') },
	{ CH_SQUOTE, 'o', 0, UC('ó') },
	{ CH_SQUOTE, 'u', 0, UC('ú') },
	{ CH_SQUOTE, 'y', 0, UC('ý') },
	{ CH_DQUOTE, 'A', 0, UC('Ä') },
	{ CH_DQUOTE, 'E', 0, UC('Ë') },
	{ CH_DQUOTE, 'I', 0, UC('Ï') },
	{ CH_DQUOTE, 'O', 0, UC('Ö') },
	{ CH_DQUOTE, 'U', 0, UC('Ü') },
	{ CH_DQUOTE, 'a', 0, UC('ä') },
	{ CH_DQUOTE, 'e', 0, UC('ë') },
	{ CH_DQUOTE, 'i', 0, UC('ï') },
	{ CH_DQUOTE, 'o', 0, UC('ö') },
	{ CH_DQUOTE, 'u', 0, UC('ü') },
	{ CH_DQUOTE, 'y', 0, UC('ÿ') },
	{ '^', 'A', 0, UC('Â') },
	{ '^', 'E', 0, UC('Ê') },
	{ '^', 'I', 0, UC('Î') },
	{ '^', 'O', 0, UC('Ô') },
	{ '^', 'U', 0, UC('Û') },
	{ '^', 'a', 0, UC('â') },
	{ '^', 'e', 0, UC('ê') },
	{ '^', 'i', 0, UC('î') },
	{ '^', 'o', 0, UC('ô') },
	{ '^', 'u', 0, UC('û') },
	{ '~', 'A', 0, UC('Ã') },
	{ '~', 'O', 0, UC('Õ') },
	{ '~', 'N', 0, UC('Ñ') },
	{ '~', 'a', 0, UC('ã') },
	{ '~', 'o', 0, UC('õ') },
	{ '~', 'n', 0, UC('ñ') },
	{ 'A', 'E', 0, UC('Æ') },
	{ 'a', 'e', 0, UC('æ') },
	{ 'C', ',', 0, UC('Ç') },
	{ 'c', ',', 0, UC('ç') },
	{ 'O', '/', 0, UC('Ø') },
	{ 'o', '/', 0, UC('ø') },
	{ 'P', 'B', 0, UC('Þ') },
	{ 'p', 'b', 0, UC('þ') },
	{ '+', '-', 0, UC('±') },
	{ 'D', '-', 0, UC('Ð') },
	{ '^', '1', 0, UC('¹') },
	{ '^', '2', 0, UC('²') },
	{ '^', '3', 0, UC('³') },
	{ 'x', '*', 2, 0x60 },
	{ UC('×'), '*', 2, 0x60 },
	{ 'X', '*', 2, 0x60 },
	{ '=', '/', 2, 0x7c },
	{ '/', '=', 2, 0x7c },
	{ '<', '=', 2, 0x79 },
	{ '>', '=', 2, 0x7a },
	{ '<', '_', 2, 0x79 },
	{ '>', '_', 2, 0x7a },
	{ '|', '-', 2, 0x6e },
	{ '|', '|', 2, 0x78 },
	{ UC('¯'), '_', 2, 0x61 },
	{ 'n', UC('¯'), 3, 0x50 },
	{ 'n', '-', 3, 0x70 },
	{ '{', '|', 3, 0x2f },
	{ '}', '|', 3, 0x30 },
	{ 'Y', '|', 3, 0x51 },
	{ 'y', '|', 3, 0x71 },
	{ 'f', '-', 3, 0x76 },
	{ '=', '-', 3, 0x4f },
	{ 'O', UC('·'), 3, 0x4a },
	{ '~', '-', 3, 0x49 },
	{ 'o', 'c', 3, 0x41 },
	{ ':', '.', 3, 0x40 },
	{ 'e', '-', 3, 0x65 },
	{ '-', ':', 3, 0x43 },
	{ 'd', UC('¯'), 3, 0x64 },
	{ '<', '-', 3, 0x7b },
	{ '>', '-', 3, 0x7d },
	{ '|', '^', 3, 0x7c },
	{ '^', '|', 3, 0x7c },
	{ '|', 'v', 3, 0x7e },
	{ 'v', '|', 3, 0x7e },
	{ 'v', '^', 3, 0x46 },
	{ '^', 'v', 3, 0x46 },
	{ 0, 0, 0, 0 }
} ;


/* exported subroutines */


int termout_start(TERMOUT *op,const char *tstr,int tlen,int ncols)
{
	int		rs = SR_OK ;
	vector<GCH>	*cvp ;

	if (op == NULL) return SR_FAULT ;

	if (ncols <= 0) return SR_INVALID ;

	memset(op,0,sizeof(TERMOUT)) ;
	op->ncols = ncols ;
	op->termattr = gettermattr(tstr,tlen) ;

	if ((cvp = new(nothrow) vector<GCH>) != NULL) {
	    op->cvp = (void *) cvp ;
	    op->magic = TERMOUT_MAGIC ;
	} else
	    rs = SR_NOMEM ;

	return rs ;
}
/* end subroutine (termout_start) */


int termout_finish(TERMOUT *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMOUT_MAGIC) return SR_NOTOPEN ;

	if (op->lvp != NULL) {
	    vector<string> *lvp = (vector<string> *) op->lvp ;
	    delete lvp ; /* calls all 'string' destructors */
	    op->lvp = NULL ;
	}

	if (op->cvp != NULL) {
	    vector<GCH> *cvp = (vector<GCH> *) op->cvp ;
	    delete cvp ;
	    op->cvp = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (termout_finish) */


int termout_load(TERMOUT *op,const char *sbuf,int slen)
{
	vector<string>	*lvp ;
	int		rs = SR_OK ;
	int		ln = 0 ;

#if	CF_DEBUGS
	debugprintf("termout_load: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	if (slen < 0) slen = strlen(sbuf) ;

#if	CF_DEBUGS
	debugprintf("termout_load: s=>%t<\n",
		sbuf,strlinelen(sbuf,slen,60)) ;
#endif

	if ((lvp = (vector<string> *) op->lvp) == NULL) {
	    if ((lvp = new(nothrow) vector<string>) != NULL) {
	        op->lvp = lvp ;
	    } else
		rs = SR_NOMEM ;
	}

/* process given string into the staging area */

#if	CF_DEBUGS
	debugprintf("termout_load: process slen=%d\n",slen) ;
#endif

	if (rs >= 0) {
	    rs = termout_process(op,sbuf,slen) ;
	    ln = rs ;
	}

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termout_load) */


int termout_getline(TERMOUT *op,int i,const char **lpp)
{
	vector<string>	*lvp ;
	uint		ui = i ;
	int		rs = SR_OK ;
	int		ll = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lpp == NULL) return SR_FAULT ;

	if ((lvp = ((vector<string> *) op->lvp)) != NULL) {
	    if (ui < lvp->size()) {
	        *lpp = lvp->at(i).c_str() ;
	        ll = lvp->at(i).size() ;
	    } else
	        rs = SR_NOTFOUND ;
	} else
	    rs = SR_BUGCHECK ;

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (termout_getline) */


/* private subroutines */


static int termout_process(TERMOUT *op,const char *sbuf,int slen)
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
	debugprintf("termout_process: ent slen=%d\n",slen) ;
#endif

#ifdef	COMMENT
	{
	    int	sz, rsz ;
	    sz = cvp->size() ;
	    rsz = (slen+10) ;
	    if (sz < rsz) cvp->reserve(rsz) ;
	}
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("termout: slen=%d s=>%t<\n",
		slen,sbuf,strlinelen(sbuf,slen,40)) ;
	debugprintf("termout: cvp->clear()\n") ;
#endif

	cvp->clear() ;

#if	CF_DEBUGS
	debugprintf("termout: for-before\n") ;
#endif

	for (i = 0 ; i < slen ; i += 1) {
	    ch = MKCHAR(sbuf[i]) ;
#if	CF_DEBUGS
	debugprintf("termout: i=%u switch ch=%c (\\%02x)\n",i,ch,ch) ;
#endif
	    switch (ch) {
	    case '\r':
		j = 0 ;
		break ;
	    case '\n':
	    case '\v':
	    case '\f':
		rs = termout_loadline(op,ln++,max) ;
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
	debugprintf("termout_process: mid rs=%d max=%u\n",rs,max) ;
#endif

	if ((rs >= 0) && (max > 0)) {
	    rs = termout_loadline(op,ln++,max) ;
	    len += rs ;
	}

#if	CF_DEBUGS
	debugprintf("termout_process: ret rs=%d ln=%u len=%u\n",rs,ln,len) ;
#endif

	return (rs >= 0) ? ln : rs ;
}
/* end subroutine (termout_process) */


static int termout_loadline(TERMOUT *op,int ln,int max)
{
	vector<GCH>	*cvp = (vector<GCH> *) op->cvp ;
	vector<string>	*lvp = (vector<string> *) op->lvp ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("termout_loadline: ent ln=%u max=%u\n",ln,max) ;
#endif

	lvp->resize(ln+1) ;		/* instantiates new 'string' */
	{
	    int		ft, ch, gr ;
	    int		i ;
	    int		pgr = 0 ; /* holds Previous-Graphic-Rendition */
	    string	&line = lvp->at(ln) ;
	    line.reserve(120) ;
	    line.clear() ;
	    for (i = 0 ; (rs >= 0) && (i < max) ; i += 1) {
		ft = cvp->at(i).ft ; /* font */
		ch = cvp->at(i).ch ; /* character */
		gr = cvp->at(i).gr ; /* graphic-rendition */
		if ((rs = termout_loadgr(op,line,pgr,gr)) >= 0) {
		    len += rs ;
		    if (ch > 0) {
		        rs = termout_loadch(op,line,ft,ch) ;
		        len += rs ;
		    }
		}
		pgr = gr ;
	    } /* end for */
	    if ((rs >= 0) && (pgr != 0)) {
		rs = termout_loadgr(op,line,pgr,0) ;
		len += rs ;
	    }
	} /* end block */

#if	CF_DEBUGS
	debugprintf("termout_loadline: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termout_loadline) */


static int termout_loadgr(TERMOUT *op,string &line,int pgr,int gr)
{
	const int	grmask = ( GR_MBOLD| GR_MUNDER| GR_MBLINK| GR_MREV) ;
	int		rs = SR_OK ;
	int		n ;
	int		size ;
	int		len = 0 ;
	char		*grbuf ;

	n = flbsi(grmask) + 1 ;

	size = ((2*n)+1+1) ; /* size according to algorithm below */
	if ((grbuf = new(nothrow) char[size]) != NULL) {
	    int		gl = 0 ;
	    int		ogr = pgr ;
	    int		bgr = pgr ;
	    int		cc ;
	    int		i ;

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
	        rs = termout_loadcs(op,line,'m',grbuf,gl) ;
	        len = rs ;
	    }
    
	    delete[] grbuf ;
	} else
	    rs = SR_NOMEM ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termout_loadgr) */


static int termout_loadcs(TERMOUT *op,string &line,int n,cchar *pp,int pl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	while ((rs >= 0) && (i < pl)) {
	    const int	ml = MIN(4,(pl-i)) ;
	    int		a1 = -1 ;
	    int		a2 = -1 ;
	    int		a3 = -1 ;
	    int		a4 = -1 ;
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
		const int	dlen = DBUFLEN ;
		char		dbuf[DBUFLEN+1] ;
		if ((rs = termconseq(dbuf,dlen,n,a1,a2,a3,a4)) >= 0) {
		    const int	dl = rs ;
		    i += ml ;
		    len += dl ;
		    line.append(dbuf,dl) ;
		}
	    }
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termout_loadcs) */


static int termout_loadch(TERMOUT *op,string &line,int ft,int ch)
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

	} /* end if (non-zero) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termout_loadch) */


static int gettermattr(const char *tstr,int tlen)
{
	int		ta = 0 ;

	if (tstr != NULL) {
	    int		i ;
	    const char	*np ;

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


