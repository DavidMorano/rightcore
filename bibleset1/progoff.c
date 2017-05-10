/* progoff */

/* send commands (by printing) to TROFF */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We deal with TROFF matters.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progoff.h"
#include	"multiout.h"


/* local defines */

#ifndef	PROGINFO_POFF
#define	PROGINFO_POFF	struct proginfo_poff
#endif

#define	TRMAC_TCL	"tL"
#define	TRMAC_TCR	"tR"


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	bufprintf(const char *,int,...) ;

extern char	*timestr_logz(time_t,char *) ;


/* local structures */

struct fontfamily {
	const char	*family ;
	const char	*code[4] ;
	const char	*name ;
} ;

struct chartran {
	int		ch ;
	const char	*troffstr ;
} ;


/* forward references */

static int	proctransbegin(struct proginfo *) ;
static int	proctransend(struct proginfo *) ;
static int	procfontset(struct proginfo *,bfile *) ;
static int	procfontcode(struct proginfo *,const char *,const char **) ;
static int	proctcbegin(struct proginfo *,bfile *) ;

static int	troffswitch(char *,int,const char *) ;
static int	isallnul(const char *,const char *,const char *) ;


/* local variables */

struct fontfamily	families[] = {
	{ "T" , { "R", "I", "B", "BI" }, "times" },
	{ "P" , { "PA", "PI", "PB", "PX" }, "palatina" },
	{ "B" , { "KR", "KI", "KB", "KX" }, "bookman" },
	{ "N" , { "NR", "NI", "NB", "NX" }, "new-century-schoolbook" },
	{ "H" , { "H", "HI", "HB", "HX" }, "helvetica" },
	{ "R" , { "Hr", "Hi", "Hb", "Hx" }, "helvetica-narrow" },
	{ "A" , { "AR", "AI", "AB", "AX" }, "avant-garde" },
	{ "C" , { "CO", "CI", "CB", "CX" }, "courier" },
	{ NULL, { NULL, NULL, NULL, NULL }, NULL }
} ;

static const char troff_slash3[] = {
	'\\',
	'\\',
	'\\',
	'\0'
} ;

static const char troff_int2[] = {
	'\\',
	'*',
	CH_LPAREN,
	'\0'
} ;

static const char troff_linecomment[] = {
	'.',
	'\\',
	CH_DQUOTE,
	'_',
	'\0'
} ;

static const char troff_infont_cw[] = {
	'\\',
	'f',
	CH_LPAREN,
	'C',
	'O',
	'\0'
} ;

static const char troff_infont_p[] = {
	'\\',
	'f',
	'P',
	'\0'
} ;

static const char troff_infont_r[] = {
	'\\',
	'f',
	'R',
	'\0'
} ;

static const char troff_infont_i[] = {
	'\\',
	'f',
	'I',
	'\0'
} ;

static const char troff_infont_b[] = {
	'\\',
	'f',
	'B',
	'\0'
} ;

static const char troff_infont_x[] = {
	'\\',
	'f',
	CH_LPAREN,
	'B',
	'I',
	'\0'
} ;

static const char	*hftypes[] = {
	"PH",
	"EH",
	"OH",
	"PF",
	"EF",
	"OF",
	NULL
} ;

static const char	nul[] = {
	'\0'
} ;

static struct chartran	trofftrans[] = {
	{ ' ', "\\ " },			/* NBSP -> TROFF unpaddable space */
	{ 'À', "\\z\\(gaA" },		/* over-strike */
	{ 'Á', "\\z\\(aaA" },		/* over-strike */
	{ 'Â', "\\z^A" },		/* over-strike */
	{ 'Ã', "\\z\\u~\\dA" },		/* over-strike */
	{ 'Ä', "\\z\\(dqA" },		/* over-strike */
	{ 'Å', "\\z\\(deA" },		/* over-strike */
	{ 'Ç', "\\z\\(tsC" },		/* over-strike */
	{ 'È', "\\z\\(gaE" },		/* over-strike */
	{ 'É', "\\z\\(aaE" },		/* over-strike */
	{ 'Ê', "\\z^E" },		/* over-strike */
	{ 'Ë', "\\z\\(dqE" },		/* over-strike */
	{ 'Ì', "\\z\\(gaI" },		/* over-strike */
	{ 'Í', "\\z\\(aaI" },		/* over-strike */
	{ 'Î', "\\z^I" },		/* over-strike */
	{ 'Ï', "\\z\\(dqI" },		/* over-strike */
	{ 'Ð', "\\z\\-D" },		/* over-strike */
	{ 'Ñ', "\\z\\u~\\dN" },		/* over-strike */
	{ 'Ò', "\\z\\(gaO" },		/* over-strike */
	{ 'Ó', "\\z\\(aaO" },		/* over-strike */
	{ 'Ô', "\\z^O" },		/* over-strike */
	{ 'Õ', "\\z\\u~\\dO" },		/* over-strike */
	{ 'Ö', "\\z\\(dqO" },		/* over-strike */
	{ 'Ù', "\\z\\(gaU" },		/* over-strike */
	{ 'Ú', "\\z\\(aaU" },		/* over-strike */
	{ 'Û', "\\z^U" },		/* over-strike */
	{ 'Ü', "U\\*;" },		/* MM interpolated string */
	{ 'Ý', "\\z\\(aaY" },		/* over-strike */
	{ 'à', "\\z\\(gaa" },		/* over-strike */
	{ 'á', "\\z\\(aaa" },		/* over-strike */
	{ 'â', "\\z^a" },		/* over-strike */
	{ 'ã', "\\z\\u~\\da" },		/* over-strike */
	{ 'ä', "\\z\\(dqa" },		/* over-strike */
	{ 'å', "\\z\\(dea" },		/* over-strike */
	{ 'ç', "c\\*," },		/* MM interpolated string */
	{ 'è', "e\\*`" },		/* MM interpolated string */
	{ 'é', "e\\*'" },		/* MM interpolated string */
	{ 'ê', "\\ze^" },		/* over-strike */
	{ 'ë', "\\z\\(dqe" },		/* over-strike */
	{ 'ì', "\\z\\(gai" },		/* over-strike */
	{ 'í', "\\z\\(aai" },		/* over-strike */
	{ 'î', "\\z^i" },		/* over-strike */
	{ 'ï', "\\z\\(dqi" },		/* over-strike */
	{ 'ñ', "n\\*~" },		/* MM interpolated string */
	{ 'ò', "\\z\\(gao" },		/* over-strike */
	{ 'ó', "\\z\\(aao" },		/* over-strike */
	{ 'ô', "o\\*^" },		/* MM interpolated string */
	{ 'õ', "\\z\\u~\\do" },		/* over-strike */
	{ 'ö', "\\z\\(dqo" },		/* over-strike */
	{ 'ñ', "\\z\\u~\\dn" },		/* over-strike */
	{ 'ù', "\\z\\(gau" },		/* over-strike */
	{ 'ú', "\\z\\(aau" },		/* over-strike */
	{ 'û', "\\z^u" },		/* over-strike */
	{ 'ü', "u\\*:" },		/* MM interpolated string */
	{ 'ý', "\\z\\(aay" },		/* over-strike */
	{ 'ÿ', "\\z\\(dqy" },		/* over-strike */
	{ 'µ', "\\*m" },		/* TROFF Greek */
	{ 'ø', "\\*f" },		/* TROFF Greek */
	{ '­', "\\(hy" },		/* TROFF special character */
	{ '´', "\\(aa" },		/* TROFF special character */
	{ '¼', "\\(14" },		/* TROFF special character */
	{ '½', "\\(12" },		/* TROFF special character */
	{ '®', "\\(rg" },		/* TROFF special character */
	{ '©', "\\(co" },		/* TROFF special character */
	{ '§', "\\(sc" },		/* TROFF special character */
	{ '¶', "\\(pg" },		/* TROFF special character */
	{ '¢', "\\(ct" },		/* TROFF special character */
	{ '£', "\\(ps" },		/* TROFF special character */
	{ '¥', "\\(yn" },		/* TROFF special character */
	{ '¤', "\\(cr" },		/* TROFF special character */
	{ '·', "\\(bu" },		/* TROFF special character */
	{ '¡', "\\(!!" },		/* TROFF special character */
	{ '¿', "\\(??" },		/* TROFF special character */
	{ '°', "\\(de" },		/* TROFF special character */
	{ '¬', "\\(no" },		/* TROFF special character */
	{ '±', "\\(+-" },		/* TROFF special character */
	{ '×', "\\(mu" },		/* TROFF special character */
	{ '÷', "\\(di" },		/* TROFF special character */
	{ '¯', "\\(rn" },		/* TROFF special character */
	{ 'Ø', "\\(es" },		/* TROFF special character */
	{ '¢', "\\(ct" },		/* TROFF special character */
	{ '£', "\\(ps" },		/* TROFF special character */
	{ '¥', "\\(yn" },		/* TROFF special character */
	{ '\0', NULL }
} ;


/* exported subroutines */


int progoffbegin(pip,ofp)
struct proginfo	*pip ;
bfile		*ofp ;
{
	struct troffstrs	*tsp = &pip->troff ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoffbegin: entered\n") ;
#endif

	if (ofp == NULL)
	    return SR_FAULT ;

/* make character translation table for TROFF */

	if ((rs = proctransbegin(pip)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progoffbegin: proctransbegin() rs=%d\n",rs) ;
#endif

/* load TROFF miscellaneous strings */

	    tsp->slash3 = (const char *) troff_slash3 ;
	    tsp->int2 = (const char *) troff_int2 ;
	    tsp->linecomment = (const char *) troff_linecomment ;
	    tsp->infont_p = (const char *) troff_infont_p ;
	    tsp->infont_cw = (const char *) troff_infont_cw ;

/* load TROFF default ("Times") font-change strings */

	    tsp->infont_r = (const char *) troff_infont_r ;
	    tsp->infont_i = (const char *) troff_infont_i ;
	    tsp->infont_b = (const char *) troff_infont_b ;
	    tsp->infont_x = (const char *) troff_infont_x ;

/* output the foundational stuff to TROFF */

	    if (rs >= 0) {
	        const char	*ts = timestr_logz(pip->daytime,timebuf) ;
	        const char	*lc = pip->troff.linecomment ;
	        rs = bprintf(ofp,"%s BIBLESET %s starting\n%s\n",lc,ts,lc) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (pip->ps != DEFPS)) {
	        rs = bprintf(ofp,".S %u %u\n",pip->ps,pip->vs) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = progoffsrn(pip,ofp,"header_style","N",4) ;
	        wlen += rs ;
	    }

/* find the font-family that we have been given (or assumed) */

	    if (rs >= 0) {
	        rs = procfontset(pip,ofp) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && pip->f.tc) {
	        rs = proctcbegin(pip,ofp) ;
	        wlen += rs ;
	    }

	} /* end if (proctransbegin) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoffbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progoffbegin) */


int progoffend(pip)
struct proginfo	*pip ;
{
	PROGINFO_POFF	*pop = &pip->poff ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pop->chartrans != NULL) {
	    rs1 = proctransend(pip) ;
	    if (rs >= 0) rs = rs1 ;
	    pop->chartrans = NULL ;
	}

	return rs ;
}
/* end subroutine (progoffend) */


int progoffcomment(pip,ofp,sp,sl)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if (rs >= 0) {
	    const char	*lc = pip->troff.linecomment ;
	    rs = bwrite(ofp,lc,-1) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    bputc(ofp,' ') ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    bwrite(ofp,sp,sl) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    bputc(ofp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffcomment) */


/* TROFF define-string from string argument */
int progoffdss(pip,ofp,n,v)
struct proginfo	*pip ;
bfile		*ofp ;
const char	n[] ;
const char	*v ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	if (n == NULL) return SR_FAULT ;

	if (n[0] == '\0') return SR_INVALID ;

	rs = bprintf(ofp,".ds %s %s\n", n, v) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffdss) */


/* TROFF define-string from numeric (integer) argument */
int progoffdsn(pip,ofp,n,v)
struct proginfo	*pip ;
bfile		*ofp ;
const char	n[] ;
int		v ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	if (n == NULL) return SR_FAULT ;

	if (n[0] == '\0') return SR_INVALID ;

	rs = bprintf(ofp,".ds %s %d\n", n, v) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffdsn) */


/* TROFF set-register from string argument */
int progoffsrs(pip,ofp,comment,rn,rv)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*comment ;
const char	*rn ;
const char	*rv ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	if (rn == NULL) return SR_FAULT ;

	if ((rs >= 0) && (comment != NULL)) {
	    cchar	*lc = pip->troff.linecomment ;
	    rs = bprintf(ofp,"%s %s=%s\n",lc,comment,rv) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,".nr %s %s\n", rn,rv) ;
	    wlen += rs ;
	}

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = bprintf(ofp,"%s\n",
	        pip->troff.linecomment) ;
	    wlen += rs ;
	}
#endif /* COMMENT */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffsrs) */


/* TROFF set-register from numeric (integer) argument */
int progoffsrn(pip,ofp,comment,rn,rv)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*comment ;
const char	*rn ;
int		rv ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	if (rn == NULL) return SR_FAULT ;

	if ((rs >= 0) && (comment != NULL)) {
	    cchar	*lc = pip->troff.linecomment ;
	    rs = bprintf(ofp,"%s %s=%d\n",lc,comment,rv) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,".nr %s %d\n", rn,rv) ;
	    wlen += rs ;
	}

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = bprintf(ofp,"%s\n",
	        pip->troff.linecomment) ;
	    wlen += rs ;
	}
#endif /* COMMENT */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffsrn) */


/* TROFF header-format from 'left', 'center', and 'right' */
int progoffhf(pip,ofp,type,s_l,s_m,s_r)
struct proginfo	*pip ;
bfile		*ofp ;
const char	type[] ;
const char	s_l[] ;
const char	s_m[] ;
const char	s_r[] ;
{
	const int	qch = CH_DQUOTE ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;
	if (type == NULL) return SR_FAULT ;

	if (type[0] == '\0') return SR_INVALID ;

	if (matstr(hftypes,type,-1) < 0)
	    return SR_INVALID ;

	if (s_l == NULL) s_l = nul ;
	if (s_m == NULL) s_m = nul ;
	if (s_r == NULL) s_r = nul ;

	if (isallnul(s_l,s_m,s_r)) {
#ifdef	COMMENT
	    rs = bprintf(ofp,".%s %c%c\n", type,qch,qch) ;
#else
	    rs = bprintf(ofp,".%s\n",type) ;
#endif /* COMMENT */
	    wlen += rs ;
	} else {
	    cchar	*fmt = ".%s %c'%s'%s'%s'%c\n" ;
	    rs = bprintf(ofp,fmt,type,qch,s_l,s_m,s_r,qch) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffhf) */


/* set the base font of the current font family */
int progoffsetbasefont(pip,ofp)
struct proginfo	*pip ;
bfile		*ofp ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if ((pip->ff != NULL) && (pip->ffi > 0)) {
	    cchar	*fcp = families[pip->ffi].code[0] ;
	    rs = bprintf(ofp,".ft %s\n",fcp) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffsetbasefont) */


int progoffwrite(pip,ofp,sp,sl)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*sp ;
int		sl ;
{
	PROGINFO_POFF	*pop = &pip->poff ;
	uint		ch ;
	int		rs = SR_OK ;
	int		i ;
	int		slr ;
	int		f ;
	int		wlen = 0 ;
	const char	**trans ;

	if (ofp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	trans = pop->chartrans ;
	if (trans != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progoff/write: translating l=>%t<\n",
	            sp,strlinelen(sp,sl,30)) ;
#endif

	    for (i = 0 ; i < sl ; i += 1) {
	        ch = sp[i] & 0xff ;
	        slr = (sl - i) ;
	        f = (slr > 1) && (sp[i] == '-') && (sp[i+1] == '-') ;
	        f = f && ((i == 0) || (sp[-1] != '\\')) ;
	        if (f) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("progoff/write: dash ch=>%c<\n",ch) ;
#endif

	            i += 1 ;
	            rs = bwrite(ofp,"\\(em",-1) ;
	        } else {
	            if (trans[ch] != NULL) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("progoff/write: "
	                        "trans ch=>%c< trans=>%s<\n",
	                        ch,trans[ch]) ;
#endif

	                rs = bwrite(ofp,trans[ch],-1) ;
	            } else
	                rs = bputc(ofp,ch) ;
	        }
	        wlen += rs ;
	    } /* end for */

	} else
	    rs = SR_NOANODE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progoff/write: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoffwrite) */


int progofftcadd(pip,ofp,col,cname)
struct proginfo	*pip ;
bfile		*ofp ;
int		col ;
const char	cname[] ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (rs >= 0) {
	    rs = bprintf(ofp,".am %s\n",
	        ((col == 0) ? TRMAC_TCL : TRMAC_TCR)) ;
	    wlen += rs ;
	}

/* interpolate register 'P' */

	if (rs >= 0) {
	    rs = bprintf(ofp,"\t\\nP\t%s\n.br\n",cname) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(ofp,"..\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progofftcadd) */


int progofftcmk(pip,ofp,ncols)
struct proginfo	*pip ;
bfile		*ofp ;
int		ncols ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (ncols > 0) {

	    if (rs >= 0) {
	        rs = bprintf(ofp,
	            ".nr tt ((\\n(.l-1.6i)*0.5i/1.6i) \n") ;
	        wlen += rs ;
	    }

#ifdef	COMMENT
	    if (rs >= 0) {
	        rs = bprintf(ofp," tt=\\n(tt ll=\\n(.l\n",-1) ;
	        wlen += rs ;
	    }
#endif

	    if (rs >= 0) {
	        rs = bprintf(ofp,".nf\n") ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bprintf(ofp,".in +\\n(ttu\n") ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bprintf(ofp,".ta +0.5iR +0.2i \n") ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bprintf(ofp,".%s\n",TRMAC_TCL) ;
	        wlen += rs ;
	    }

	    if (ncols >= 2) {

	        if (rs >= 0) {
	            rs = bprintf(ofp,".SP 4i\n") ;
	            wlen += rs ;
	        }

	        if (rs >= 0) {
	            rs = bprintf(ofp,".%s\n",TRMAC_TCR) ;
	            wlen += rs ;
	        }

	    } /* end if */

	    if (rs >= 0) {
	        rs = bprintf(ofp,".in -\\n(ttu\n") ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bprintf(ofp,".fi\n") ;
	        wlen += rs ;
	    }

	} /* end if (positive) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progofftcmk) */


/* local subroutines */


static int proctransbegin(pip)
struct proginfo	*pip ;
{
	PROGINFO_POFF	*pop = &pip->poff ;
	uint		ch ;
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	int		c = 0 ;

	size = 256 * sizeof(uchar *) ;
	rs = uc_malloc(size,&pop->chartrans) ;
	if (rs < 0)
	    goto ret0 ;

	memset(pop->chartrans,0,size) ;

	{
	const char	*ts ;
	for (i = 0 ; trofftrans[i].ch != '\0' ; i += 1) {
	    ch = (trofftrans[i].ch & 0xff) ;
	    if (pop->chartrans[ch] == NULL) {
	        c += 1 ;
	        ts = trofftrans[i].troffstr ;
	        pop->chartrans[ch] = ts ;
	    }
	} /* end for */
	} /* end block */

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progoff/proctransbegin: trans n=%u\n",c) ;
	    for (i = 0 ; i < 256 ; i += 1) {
	        if (pop->chartrans[i] != NULL)
	            debugprintf("progoff/proctransbegin: i=%u trans=>%s<\n",i,
	                pop->chartrans[i]) ;
	    }
	}
#endif /* CF_DEBUG */

ret0:
	return (rs >= 0) ? c : rs ;

/* bad stuff */
bad1:
	proctransend(pip) ;
	goto ret0 ;
}
/* end subroutine (proctransbegin) */


static int proctransend(pip)
struct proginfo	*pip ;
{
	PROGINFO_POFF	*pop = &pip->poff ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pop->chartrans != NULL) {
	    rs1 = uc_free(pop->chartrans) ;
	    if (rs >= 0) rs = rs1 ;
	    pop->chartrans = NULL ;
	} /* end if */

	return rs ;
}
/* end subroutine (proctransend) */


int procfontset(pip,ofp)
struct proginfo	*pip ;
bfile		*ofp ;
{
	struct troffstrs	*tsp = &pip->troff ;
	int		rs = SR_OK ;
	int		i = -1 ;
	int		wlen = 0 ;
	const char	*ff ;
	const char	*fcp ;

	pip->ffi = -1 ;

/* default */

	tsp->infont_r = (const char *) troff_infont_r ;
	tsp->infont_i = (const char *) troff_infont_i ;
	tsp->infont_b = (const char *) troff_infont_b ;
	tsp->infont_x = (const char *) troff_infont_x ;

/* check for specified argument */

	ff = (const char *) pip->ff ;
	if (ff == NULL) {
	    goto ret1 ;
	}

	for (i = 0 ; families[i].family != NULL ; i += 1) {
	    if (strcasecmp(ff,families[i].family) == 0)
	        break ;
	} /* end for */

	if (families[i].family != NULL) {

	    if (rs >= 0) {
	        fcp = families[i].code[0] ;
	        rs = procfontcode(pip,fcp,&tsp->infont_r) ;
	    }

	    if (rs >= 0) {
	        fcp = families[i].code[1] ;
	        rs = procfontcode(pip,fcp,&tsp->infont_i) ;
	    }

	    if (rs >= 0) {
	        fcp = families[i].code[2] ;
	        rs = procfontcode(pip,fcp,&tsp->infont_b) ;
	    }

	    if (rs >= 0) {
	        fcp = families[i].code[3] ;
	        rs = procfontcode(pip,fcp,&tsp->infont_x) ;
	    }

	} else {

	    i = -1 ;
	    if (! pip->f.quiet)
	        bprintf(pip->efp,"%s: font-family not available (%s)\n",
	            pip->progname,ff) ;

	} /* end if */

/* set the "font-family-index" from the 'i' value above */

	pip->ffi = i ;

ret1:
	if (rs >= 0) {
	    const char	*fn = NULL ;

	    if ((ff != NULL) && (i >= 0))
	        fn = families[i].name ;

	    rs = bprintf(ofp,"%s font-family=%s (%s)\n",
	        pip->troff.linecomment,
	        ((ff != NULL) ? fn : "*default*"),
	        ((ff != NULL) ? ff : "D")) ;
	    wlen += rs ;
	}

	if ((rs >= 0) && (ff != NULL) && (i > 0)) {
	    fcp = families[i].code[0] ;
	    rs = bprintf(ofp,".ft %s\n",fcp) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procoffsetfont) */


static int procfontcode(pip,fc,rpp)
struct proginfo	*pip ;
const char	fc[] ;
const char	**rpp ;
{
	int		rs ;
	int		tbl = 0 ;
	char		tmpbuf[TIMEBUFLEN + 1] ;

	if ((rs = troffswitch(tmpbuf,TIMEBUFLEN,fc)) >= 0) {
	    tbl = rs ;
	    if ((rs = vecstr_add(&pip->stores,tmpbuf,tbl)) >= 0) {
		const char	*cp ;
	        const int	si = rs ;
	        if ((rs = vecstr_get(&pip->stores,si,&cp)) >= 0) {
	            *rpp = (const char *) cp ;
		}
	    }
	}

	return (rs >= 0) ? tbl : rs ;
}
/* end subroutine (procfontcode) */


static int proctcbegin(pip,ofp)
struct proginfo	*pip ;
bfile		*ofp ;
{
	MULTIOUT	mo ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofp == NULL)
	    return SR_FAULT ;

	if ((rs = mo_start(&mo,ofp)) >= 0) {

	    mo_printf(&mo,".de %s\n",TRMAC_TCL) ;
	    mo_printf(&mo,"	\\s-1%s\\s0	\\s-1%s\\s0\n",
	        pip->word[word_page],pip->word[word_booktitle]) ;
	    mo_printf(&mo,"	\\u\\l'0.4i'\\d	\\u\\l'1.0i'\\d\n") ;
	    mo_printf(&mo,"..\n") ;
	    mo_printf(&mo,".de %s\n",TRMAC_TCR) ;
#ifdef	COMMENT
	    mo_printf(&mo,"	\\s-1page\\s0	\\s-1book title\\s0\n") ;
#else
	    mo_printf(&mo,"	\\s-1%s\\s0	\\s-1%s\\s0\n",
	        pip->word[word_page],pip->word[word_booktitle]) ;
#endif
	    mo_printf(&mo,"	\\u\\l'0.4i'\\d	\\u\\l'1.0i'\\d\n") ;
	    mo_printf(&mo,"..\n") ;

	    rs1 = mo_finish(&mo) ;
	    wlen += rs1 ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mo) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proctcbegin) */


static int troffswitch(buf,buflen,fontcode)
char		buf[] ;
int		buflen ;
const char	fontcode[] ;
{
	int		rs = SR_OK ;
	int		nc ;
	int		len = 0 ;

	if (buf == NULL) return SR_FAULT ;
	if (fontcode == NULL) return SR_FAULT ;

	if (fontcode[0] == '\0') return SR_INVALID ;

	if (buflen <= 0) return SR_OVERFLOW ;

	nc = strlen(fontcode) ;

	switch (nc) {
	case 1:
	    rs = bufprintf(buf,buflen,"\\f%s",fontcode) ;
	    len = rs ;
	    break ;
	case 2:
	    rs = bufprintf(buf,buflen,"\\f%c%s",CH_LPAREN,fontcode) ;
	    len = rs ;
	    break ;
	default:
	    rs = SR_NOTSUP ;
	    break ;
	} /* end switch */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (troffswitch) */


/* is-all-strings-null? (OK, maybe the grammer is wrong) */
static int isallnul(s1,s2,s3)
const char	s1[], s2[], s3[] ;
{
	int		f = TRUE ;

	f = f && (s1[0] == '\0') ;
	f = f && (s2[0] == '\0') ;
	f = f && (s3[0] == '\0') ;

	return f ;
}
/* end subroutine (isallnul) */


