/* linefold */

/* manage folding of a line */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module takes a line of text as input and breaks it up into
	pieces that are folded so as to fit in a specified number of cols.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>

#include	"linefold.h"


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

extern int	charcols(int,int,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

struct params {
	int		cols ;
	int		ind ;
	int		nline ;
} ;

struct liner {
	const char	*lp ;
	int		ll ;
} ;


/* forward references */

static int	linefold_process(LINEFOLD *,int,int,const char *,int) ;

static int	params_load(PARAMS *,int,int) ;
static int	params_nextline(PARAMS *,const char *,int,const char **) ;
static int	params_nline(PARAMS *) ;

static int	nextpiece(int,const char *,int,int *) ;
static int	isend(int) ;


/* local variables */


/* exported subroutines */


int linefold_start(LINEFOLD *op,int cols,int ind,cchar lbuf[],int llen)
{
	int		rs = SR_OK ;
	int		nlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lbuf == NULL) return SR_FAULT ;

	if (llen < 0) llen = strlen(lbuf) ;

#if	CF_DEBUGS
	debugprintf("linefold_start: ent cols=%u ind=%u llen=%d\n",
		cols,ind,llen) ;
#endif

	if (cols <= 0) {
	    int		v ;
	    const char	*cp = getenv(VARCOLUMNS) ;
	    if ((cp != NULL) && (cfdeci(cp,-1,&v) >= 0)) {
	        cols = v ;
	    }
	    if (cols <= 0)
	        cols = COLUMNS ;
	} /* end if (default cols) */

	memset(op,0,sizeof(LINEFOLD)) ;

	if (rs >= 0) {
	    int	size = sizeof(LINER) ;
	    int	opts = (VECOBJ_OCOMPACT) ;
	    if ((rs = vecobj_start(&op->lines,size,10,opts)) >= 0) {
	        if (llen < 0) llen = strlen(lbuf) ;
	        while ((llen > 0) && isend(lbuf[llen-1])) llen -= 1 ;
	        if ((rs = linefold_process(op,cols,ind,lbuf,llen)) >= 0) {
	            nlines = rs ;
	            op->magic = LINEFOLD_MAGIC ;
	        }
	        if (rs < 0)
		    vecobj_finish(&op->lines) ;
	    } /* end if (vecobj_start) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("linefold_start: ret rs=%d nlines=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (linefold_start) */


int linefold_finish(LINEFOLD *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEFOLD_MAGIC) return SR_NOTOPEN ;

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("linefold_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (linefold_finish) */


int linefold_get(LINEFOLD *op,int li,cchar **rpp)
{
	LINER		*lep ;
	int		rs ;
	int		ll = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEFOLD_MAGIC) return SR_NOTOPEN ;

	if (li < 0) return SR_INVALID ;

	if ((rs = vecobj_get(&op->lines,li,&lep)) >= 0) {
	    ll = lep->ll ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? lep->lp : NULL ;
	}

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (linefold_get) */


int linefold_getline(LINEFOLD *op,int li,cchar **rpp)
{
	LINER		*lep ;
	int		rs ;
	int		ll = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEFOLD_MAGIC) return SR_NOTOPEN ;

	if (li < 0) return SR_INVALID ;

	if ((rs = vecobj_get(&op->lines,li,&lep)) >= 0) {
	    ll = lep->ll ;
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

	if (rpp != NULL) {
	    *rpp = ((rs >= 0) && (ll > 0)) ? lep->lp : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("linefold_getline: ret rs=%d ll=%u\n",rs,ll) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (linefold_getline) */


/* private subroutines */


static int linefold_process(LINEFOLD *op,int cols,int ind,cchar lbuf[],int llen)
{
	LINER		le ;
	PARAMS		p ;
	int		rs = SR_OK ;
	int		sl, ll ;
	int		nline = 0 ;
	const char	*sp ;
	const char	*lp ;

	params_load(&p,cols,ind) ;

	if (llen < 0)
	    llen = strlen(lbuf) ;

	sp = lbuf ;
	sl = llen ;
	while ((ll = params_nextline(&p,sp,sl,&lp)) > 0) {

#if	CF_DEBUGS
	debugprintf("linefold_process: params_nextline() ll=%u\n",ll) ;
#endif

	    le.lp = (const char *) lp ;
	    le.ll = ll ;
	    rs = vecobj_add(&op->lines,&le) ;

	    sl -= ((lp + ll) - sp) ;
	    sp = (lp + ll) ;

	    if (rs < 0) break ;
	} /* end while */

	if (rs >= 0) {
	    nline = params_nline(&p) ;
	}

	return (rs >= 0) ? nline : rs ;
}
/* end subroutine (linefold_process) */


static int params_load(PARAMS *pp,int cols,int ind)
{

	memset(pp,0,sizeof(PARAMS)) ;
	pp->cols = cols ;
	pp->ind = ind ;
	return SR_OK ;
}
/* end subroutine (params_load) */


static int params_nextline(PARAMS *pp,cchar *sp,int sl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		ncol ;
	int		ncs ;
	int		pl ;
	int		ll = 0 ;

/* move up to the first non-whitespace character */

	if (pp->nline > 0) {
	    while (sl && CHAR_ISWHITE(sp[0])) {
	        sp += 1 ;
	        sl -= 1 ;
	    }
	}

	*rpp = sp ;
	ncol = (pp->nline > 0) ? pp->ind : 0 ;

/* continue */

	while ((pl = nextpiece(ncol,sp,sl,&ncs)) > 0) {

	    if ((ncol + ncs) > pp->cols) {
		if (ll == 0) ll = pl ;
	        break ;
	    }

	    ll += pl ;
	    ncol += ncs ;

	    sp += pl ;
	    sl -= pl ;

	} /* end while */

	if ((rs >= 0) && (ll > 0))
	    pp->nline += 1 ;

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (params_nextline) */


static int params_nline(PARAMS *pp)
{

	return pp->nline ;
}
/* end subroutine (params_nline) */


static int nextpiece(int ncol,cchar *sp,int sl,int *ncp)
{
	int		ncs = 0 ;
	int		cl = sl ;
	int		n ;
	int		pl = 0 ;
	const char	*cp = sp ;

/* skip over whitespace */

	while (cl && CHAR_ISWHITE(cp[0])) {
	    n = charcols(NTABCOLS,ncol,cp[0]) ;
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* skip over the non-whitespace */

	while (cl && cp[0] && (! CHAR_ISWHITE(cp[0]))) {
	    n = charcols(NTABCOLS,ncol,cp[0]) ;
	    cp += 1 ;
	    cl -= 1 ;
	    ncs += n ;
	    ncol += n ;
	} /* end while */

/* done */

	*ncp = ncs ;
	pl = (cp - sp) ;
	return pl ;
}
/* end subroutine (nextpiece) */


static int isend(int ch)
{
	return ((ch == '\n') || (ch == '\r')) ;
}
/* end subroutine (isend) */


