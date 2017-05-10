/* calent */

/* calendar entry object (for CALYEARS) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage an individual calendar entry.

	We do not actually hold the entry (proper).  Rather we hold
	a reference to the entry.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"calent.h"


/* local defines */


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int calent_start(CALENT *ep,CALENT_Q *qp,uint loff,int llen)
{
	CALENT_LINE	*elp ;
	const int	ne = CALENT_NLE ;
	int		rs ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	memset(ep,0,sizeof(CALENT)) ;
	ep->cidx = -1 ;
	ep->q = *qp ;
	ep->voff = loff ;
	ep->vlen = llen ;

	size = ne * sizeof(CALENT_LINE) ;
	if ((rs = uc_malloc(size,&elp)) >= 0) {
	    ep->lines = elp ;
	    ep->e = ne ;
	    ep->i += 1 ;
	    ep->magic = 0x99000001 ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	}

	return rs ;
}
/* end subroutine (calent_start) */


int calent_finish(CALENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->lines != NULL) {
	    rs1 = uc_free(ep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->lines = NULL ;
	}

	ep->i = 0 ;
	ep->e = 0 ;
	return rs ;
}
/* end subroutine (calent_finish) */


int calent_setidx(CALENT *ep,int cidx)
{

	if (ep == NULL) return SR_FAULT ;

	ep->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (calent_setidx) */


int calent_add(CALENT *ep,uint loff,int llen)
{
	CALENT_LINE	*elp ;
	int		rs = SR_OK ;
	int		ne ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = (ep->e * 2) + CALENT_NLE ;
	    size = ne * sizeof(CALENT_LINE) ;
	    if ((rs = uc_realloc(ep->lines,size,&elp)) >= 0) {
	        ep->e = ne ;
	        ep->lines = elp ;
	    }
	}

	if (rs >= 0) {
	    ep->vlen = ((loff + llen) - ep->voff) ;
	    elp = (ep->lines + ep->i) ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (calent_add) */


int calent_samecite(CALENT *ep,CALENT *oep)
{
	int		f = TRUE ;
	f = f && (ep->q.y == oep->q.y) ;
	f = f && (ep->q.m == oep->q.m) ;
	f = f && (ep->q.d == oep->q.d) ;
	return f ;
}
/* end subroutine (calent_samecite) */


int calent_mkhash(CALENT *ep,cchar *md)
{
	CALENT_LINE	*elp = ep->lines ;
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if (ep->lines == NULL) return SR_NOTOPEN ;

	if (! ep->f.hash) {
	    uint	hash = 0 ;
	    int		i ;
	    int		sl, cl ;
	    const char	*sp, *cp ;
	        for (i = 0 ; i < ep->i ; i += 1) {
	            sp = (md + elp[i].loff) ;
	            sl = elp[i].llen ;
	            while ((cl = nextfield(sp,sl,&cp)) > 0) {
	                hash += hashelf(cp,cl) ;
	                sl -= ((cp + cl) - sp) ;
	                sp = (cp + cl) ;
	            } /* end while */
	        } /* end for */
	        ep->hash = hash ;
	        ep->f.hash = TRUE ;
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (calent_mkhash) */


int calent_sethash(CALENT *ep,uint hash)
{
	ep->hash = hash ;
	ep->f.hash = TRUE ;
	return SR_OK ;
}
/* end subroutine (calent_sethash) */


int calent_gethash(CALENT *ep,uint *rp)
{
	int		rs = SR_OK ;
	int		f = ep->f.hash ;
	if (rp != NULL) {
	    *rp = (f) ? ep->hash : 0 ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calent_gethash) */


int calent_loadbuf(CALENT *ep,char rbuf[],int rlen,cchar *mp)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    CALENT_LINE	*lines = ep->lines ;
	    int		nlines = ep->i ; /* number of line elements */
	    int		i ;
	    int		ll ;
	    cchar	*lp ;

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0) sbuf_char(&b,' ') ;

	        lp = (mp + lines[i].loff) ;
	        ll = lines[i].llen ;

#if	CF_DEBUGS
	        debugprintf("calent_loadbuf: i=%u loff=%u llen=%u\n",
	            i,lines[i].loff,lines[i].llen) ;
#endif

	        rs = sbuf_strw(&b,lp,ll) ;

	        if (rs < 0) break ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (calent_loadbuf) */


int calent_getci(CALENT *ep)
{
	int		rs ;
	rs = ep->cidx ;
	return rs ;
}
/* end subroutine (calent_getci) */


