/* mailmsghdrfold */

/* manage folding of a line */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_STRNBREAK	1		/* use 'strnbreak()' */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module takes a line of text as input and breaks it up into
        pieces that are folded so as to fit in a specified number of
        print-output columns.

	Synopsis:

	int mailmsghdrfold_start(op,mcols,ln,sp,sl)
	MAILMSGHDRFOLD	*op ;
	int		mcols ;
	int		ln ;
	const char	*sp ;
	int		sl ;

	Arguments:

	op		object pointer
	mcols		number of total columns available for this line
	ln		line-number within header instance
	sp		header-value string pointer
	sl		header-value string length

	Returns:

	<0		bad
	>=0		OK


	Synopsis:

	int mailmsghdrfold_get(op,ncol,rpp)
	MAILMSGHDRFOLD	*op ;
	int		ncol ;
	const char	**rpp ;

	Arguments:

	op		object pointer
	mcols		number of total columns available for this line
	ln		line-number within header instance
	ncol		current column number (from beginning of line)
	rpp		pointer to resulting line

	Returns:

	<0		bad
	==0		done
	>0		length of line


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>

#include	"mailmsghdrfold.h"


/* local defines */

#undef	LINER
#define	LINER		struct liner

#undef	PARAMS
#define	PARAMS		struct params

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output cols (should be 80) */
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sidquote(const char *,int) ;
extern int	ncolstr(int,int,const char *,int) ;
extern int	ncolchar(int,int,int) ;
extern int	iceil(int,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	findpieces(MAILMSGHDRFOLD *,int,const char **) ;
static int	findbreaks(MAILMSGHDRFOLD *,int,const char **) ;
static int	findbreakers(MAILMSGHDRFOLD *,int,int,const char **) ;
static int	findslices(MAILMSGHDRFOLD *,int,const char **) ;
static int	findall(MAILMSGHDRFOLD *,cchar **) ;

static int	nextpiece(int,const char *,int,int *) ;
static int	nextbreak(int,int,const char *,int,int *) ;
static int	isskip(int) ;
static int	isend(int) ;

#if	CF_STRNBREAK
static const char	*strnbreak(const char *,int,int) ;
#endif


/* local variables */

static const char	*blank = " " ;
static const char	*breaks = ";:@.%!=" ;


/* exported subroutines */


int mailmsghdrfold_start(MAILMSGHDRFOLD *op,int mcols,int ln,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (mcols < 1) return SR_INVALID ;
	if (ln < 0) return SR_INVALID ;

	if (sl < 0) sl = strlen(sp) ;

	memset(op,0,sizeof(MAILMSGHDRFOLD)) ;
	op->mcols = mcols ;
	op->ln = ln ;

	while (sl && isskip(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	while (sl && isskip(sp[sl-1])) {
	    sl -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_start: sl=%d sp=>%t<\n",
	    sl,sp,strlinelen(sp,sl,40)) ;
#endif

	op->sp = sp ;
	op->sl = sl ;

	op->magic = MAILMSGHDRFOLD_MAGIC ;

	return rs ;
}
/* end subroutine (mailmsghdrfold_start) */


int mailmsghdrfold_finish(MAILMSGHDRFOLD *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGHDRFOLD_MAGIC) return SR_NOTOPEN ;

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (mailmsghdrfold_finish) */


/* get the resulting lines from the folding operation */
int mailmsghdrfold_get(MAILMSGHDRFOLD *op,int ncol,cchar **rpp)
{
	const int	ntab = NTABCOLS ;
	int		ll = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGHDRFOLD_MAGIC) return SR_NOTOPEN ;

	if (ncol < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold_get: ent mcols=%u ccol=%u\n",
	    op->mcols,ncol) ;
#endif

	if (op->sl > 0) {
	    const int	ncols = ncolstr(ntab,ncol,op->sp,op->sl) ;
#if	CF_DEBUGS
	    debugprintf("mailmsghdrfold_get: ncols=%u\n",ncols) ;
#endif
	    if ((ncol + ncols) > op->mcols) {
	        if ((ll = findpieces(op,ncol,rpp)) == 0) {
	            if ((ll = findbreaks(op,ncol,rpp)) == 0) {
	                if ((ll = findslices(op,ncol,rpp)) == 0) {
	                    ll = findall(op,rpp) ;
	                }
	            }
	        }
	    } else {
	        ll = findall(op,rpp) ;
	    }
	    op->ln += 1 ;
	} else {
	    *rpp = op->sp ;
	} /* end if */

#if	CF_DEBUGS
	{
	    const char	*lp = *rpp ;
	    debugprintf("mailmsghdrfold_get: ret str=>%t<\n",
	        lp,strlinelen(lp,ll,50)) ;
	    debugprintf("mailmsghdrfold_get: ret ll=%u\n",ll) ;
	}
#endif

	return ll ;
}
/* end subroutine (mailmsghdrfold_get) */


/* private subroutines */


static int findpieces(MAILMSGHDRFOLD *op,int ncol,const char **rpp)
{
	const int	mcols = op->mcols ;
	int		sl = op->sl ;
	int		ll = 0 ;
	const char	*sp = op->sp ;

#if	CF_DEBUGS
	debugprintf("findpieces: ent ncol=%u\n",ncol) ;
#endif

	if (sl > 0) {
	    int		nc = ncol ;
	    int		pl ;
	    int		ncs ;

	    while (sl && isskip(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    *rpp = sp ;
	    while ((pl = nextpiece(nc,sp,sl,&ncs)) > 0) {

#if	CF_DEBUGS
	        debugprintf("findpieces: nc=%u ncs=%u\n",nc,ncs) ;
	        debugprintf("findpieces: pl=%d piece=>%t<\n",
	            pl,sp,strlinelen(sp,pl,40)) ;
#endif

	        if ((nc + ncs) > mcols) {
	            if (ll == 0) {
	                if (op->ln == 0) {
	                    if (rpp != NULL) *rpp = blank ;
	                    ll = 1 ;
	                }
	            }
	            break ;
	        }

	        ll += pl ;
	        nc += ncs ;
	        sp += pl ;
	        sl -= pl ;

	    } /* end while */

#if	CF_DEBUGS
	    {
	        const char	*lp = *rpp ;
	        debugprintf("findpieces: str=>%t<\n",
	            lp,strlinelen(lp,ll,40)) ;
	    }
#endif

	    if (ll > 0) {
	        while (sl && isskip(sp[0])) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	        op->sp = sp ;
	        op->sl = sl ;
	    }

	} else {
	    *rpp = sp ;
	} /* end if (string length) */

#if	CF_DEBUGS
	{
	    const char	*lp = *rpp ;
	    debugprintf("findpieces: str=>%t<\n",
	        lp,strlinelen(lp,ll,50)) ;
	    debugprintf("findpieces: ret ll=%u\n",ll) ;
	}
#endif

	return ll ;
}
/* end subroutine (findpieces) */


static int findbreaks(MAILMSGHDRFOLD *op,int ncol,const char **rpp)
{
	int		i ;
	int		ll = 0 ;

	for (i = 0 ; (ll == 0) && breaks[i] ; i += 1) {
	    ll = findbreakers(op,breaks[i],ncol,rpp) ;
	} /* end for */

	return ll ;
}
/* end subroutine (findbreaks) */


static int findbreakers(MAILMSGHDRFOLD *op,int bch,int ncol,const char **rpp)
{
	const int	mcols = op->mcols ;
	int		sl = op->sl ;
	int		ll = 0 ;
	const char	*sp = op->sp ;

#if	CF_DEBUGS
	debugprintf("findbreakers: ent bch=»%c« ncol=%u\n",bch,ncol) ;
	debugprintf("findbreakers: mcols=%d sl=%d\n",mcols,sl) ;
#endif

	if (sl > 0) {
	    int		nc = ncol ;
	    int		pl ;
	    int		ncs ;

#if	CF_DEBUGS
	    debugprintf("findbreakers: going\n") ;
#endif
	    while (sl && isskip(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

#if	CF_DEBUGS
	    debugprintf("findbreakers: while-before\n") ;
#endif

	    *rpp = sp ;
	    while ((pl = nextbreak(nc,bch,sp,sl,&ncs)) > 0) {

#if	CF_DEBUGS
	        debugprintf("findbreakers: nc=%u ncs=%u\n",nc,ncs) ;
	        debugprintf("findbreakers: pl=%d piece=>%t<\n",
	            pl,sp,strlinelen(sp,pl,40)) ;
#endif

	        if ((nc + ncs) > mcols) {
	            break ;
	        }

	        ll += pl ;
	        nc += ncs ;
	        sp += pl ;
	        sl -= pl ;

	    } /* end while */

#if	CF_DEBUGS
	    {
	        const char	*lp = *rpp ;
	        debugprintf("findbreakers: line=>%t<\n",
	            lp,strlinelen(lp,ll,40)) ;
	    }
#endif

	    if (ll > 0) {
	        while (sl && isskip(sp[0])) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	        op->sp = sp ;
	        op->sl = sl ;
	    }

	} else {
	    *rpp = sp ;
	} /* end if (string length) */

#if	CF_DEBUGS
	{
	    const char	*lp = *rpp ;
	    debugprintf("findbreakers: str=>%t<\n",
	        lp,strlinelen(lp,ll,50)) ;
	    debugprintf("findbreakers: ret ll=%u\n",ll) ;
	}
#endif

	return ll ;
}
/* end subroutine (findbreakers) */


static int findslices(MAILMSGHDRFOLD *op,int ncol,const char **rpp)
{
	const int	ntab = NTABCOLS ;
	const int	mcols = op->mcols ;
	int		sl = op->sl ;
	int		ll = 0 ;
	const char	*sp = op->sp ;

#if	CF_DEBUGS
	debugprintf("findslices: ent ncol=%u\n",ncol) ;
#endif

	if (sl > 0) {
	    int		si ;
	    int		nc = ncol ;
	    int		n ;

	    while (sl && isskip(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    *rpp = sp ;
	    while (sl && sp[0] && (nc < mcols)) {
	        if (sp[0] == CH_DQUOTE) {
	            nc += 1 ;
	            sp += 1 ;
	            sl -= 1 ;
	            ll += 1 ;
	            n = 0 ;
	            if ((si = sidquote(sp,sl)) > 0) {
	                n = ncolstr(ntab,ncol,sp,sl) ;
	            }
	        } else {
	            si = 1 ;
	            n = ncolchar(ntab,nc,sp[0]) ;
	        } /* end if */
	        nc += n ;
	        sp += si ;
	        sl -= si ;
	        ll += si ;
	    } /* end while */

	    while (sl && isskip(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	    if (ll > 0) {
	        op->sp = sp ;
	        op->sl = sl ;
	    }

	} else {
	    *rpp = sp ;
	} /* end if (string length) */

#if	CF_DEBUGS
	{
	    const char	*lp = *rpp ;
	    debugprintf("findslices: str=>%t<\n",
	        lp,strlinelen(lp,ll,50)) ;
	    debugprintf("findslices: ret ll=%u\n",ll) ;
	}
#endif

	return ll ;
}
/* end subroutine (findslices) */


static int findall(MAILMSGHDRFOLD *op,cchar **rpp)
{
	int		cl ;
	const char	*cp ;
	if ((cl = sfshrink(op->sp,op->sl,&cp)) >= 0) {
	    *rpp = cp ;
	}
	op->sp += op->sl ;
	op->sl = 0 ;
	return cl ;
}
/* end subroutine (findall) */


static int nextpiece(int ncol,cchar *sp,int sl,int *ncp)
{
	const int	ntab = NTABCOLS ;
	int		ncs = 0 ;
	int		cl = sl ;
	int		si ;
	int		n ;
	int		pl = 0 ;
	const char	*cp = sp ;

/* skip over whitespace */

	while (cl && CHAR_ISWHITE(cp[0])) {
	    n = ncolchar(ntab,ncol,cp[0]) ;
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* skip over the non-whitespace */

	while (cl && cp[0] && (! CHAR_ISWHITE(cp[0]))) {
	    if (cp[0] == CH_DQUOTE) {
	        cp += 1 ;
	        cl -= 1 ;
	        ncs += 1 ;
	        ncol += 1 ;
	        n = 0 ;
	        if ((si = sidquote(cp,cl)) > 0) {
	            n = ncolstr(ntab,ncol,cp,cl) ;
	        }
	    } else {
	        si = 1 ;
	        n = ncolchar(ntab,ncol,cp[0]) ;
	    } /* end if */
	    cp += si ;
	    cl -= si ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* done */

	*ncp = ncs ;
	pl = (cp - sp) ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold/nextpiece: ret pl=%u ncs=%u\n",pl,ncs) ;
#endif

	return pl ;
}
/* end subroutine (nextpiece) */


static int nextbreak(int ncol,int bch,cchar *sp,int sl,int *ncp)
{
	const int	ntab = NTABCOLS ;
	int		ncs = 0 ;
	int		cl = sl ;
	int		n ;
	int		pl = 0 ;
	const char	*tp ;
	const char	*cp = sp ;

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold/nextbreak: ent bch=»%c« sl=%d\n",
	    bch,sl) ;
#endif

	*ncp = 0 ;

/* skip over whitespace */

	while (cl && CHAR_ISWHITE(cp[0])) {
	    n = ncolchar(ntab,ncol,cp[0]) ;
#if	CF_DEBUGS
	    debugprintf("mailmsghdrfold/nextbreak: ncolchar() n=%d\n",n) ;
#endif
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* find a possible break */

#if	CF_STRNBREAK
	if ((tp = strnbreak(cp,cl,bch)) != NULL) {
	    n = ncolstr(ntab,ncol,cp,((tp+1)-cp)) ;
	    ncs += n ;
	    pl = ((tp+1) - sp) ;
	    *ncp = ncs ;
	} /* end if */
#else
	if ((tp = strnchr(cp,cl,bch)) != NULL) {
	    n = ncolstr(ntab,ncol,cp,((tp+1)-cp)) ;
	    ncs += n ;
	    pl = ((tp+1) - sp) ;
	    *ncp = ncs ;
	} /* end if */
#endif /* CF_STRNBREAK */

	*ncp = ncs ;

/* done */

#if	CF_DEBUGS
	debugprintf("mailmsghdrfold/nextbreak: ret pl=%u ncs=%u\n",pl,ncs) ;
#endif

	return pl ;
}
/* end subroutine (nextbreak) */


static int isskip(int ch)
{
	return (CHAR_ISWHITE(ch) || isend(ch)) ;
}
/* end subrouine (isskip) */


static int isend(int ch)
{
	return ((ch == '\r') || (ch == '\n')) ;
}
/* end subroutine (isend) */


#if	CF_STRNBREAK
const char *strnbreak(const char *sp,int sl,int bch)
{
	int		ch ;
	int		si ;
	int		f = FALSE ;
	bch &= UCHAR_MAX ;
	while (sl && *sp) {
	    ch = MKCHAR(sp[0]) ;
	    if (ch == CH_DQUOTE) {
	        sp += 1 ;
	        sl -= 1 ;
	        si = sidquote(sp,sl) ;
	    } else {
	        si = 1 ;
	        f = (ch == bch) ;
	        if (f) break ;
	    }
	    sp += si ;
	    sl -= si ;
	} /* end while */
	return (f) ? sp : NULL ;
}
/* end subroutine (strnbreak) */
#endif /* CF_STRNBREAK */


