/* mkaddrname */

/* find the best address-name, if one exists */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_MASSAGE	0		/* allow for massaging */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This code piece was part of the 'pcsmailcheck(3pcs)' subroutine and I
	pulled it out to make a subroutine that can be used in multiple
	places.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine extracts the "best" address out of an EMA-type of
        address specification (given in raw string form).

	Synopsis:

	int mkaddrname(fbuf,flen,sp,sl)
	char		fbuf[] ;
	int		flen ;
	const char	*sp ;
	int		sl ;

	Arguments:

	fbuf		result buffer
	flen		result buffer length
	sp		source string
	sl		source string length

	Returns:

	<0		error
	>=0		length of resulting string


	Notes:

	+ Massaging the result:
        In the old days, before header fields could be encoded in wacko ways,
        the result here was the final result. It could therefore be massaged to
        get rid of some cruft that certain mailers (who will remain nameless --
        for now) would add to the field string value. But now-a-days, the result
        here could still be wackily encoded, so massaging will at best do
        nothing, and at worst break the encoded format.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ema.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MSGLINEBUFLEN
#define	MSGLINEBUFLEN	(LINEBUFLEN * 5)
#endif


/* external subroutines */

extern int	sfsubstance(const char *,int,const char **) ;
extern int	mkdisphdr(char *,int,const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */

enum atypes {
	atype_comment,
	atype_address,
	atype_route,
	atype_overlast
} ;


/* forward references */

static int	emaentry_addrname(EMA_ENT *,char *,int) ;
static int	isBadAddr(int) ;


/* local variables */

static int	rsbadaddr[] = {
	SR_INVALID,
	SR_DOM,
	0
} ;


/* exported subroutines */


int mkaddrname(char *fbuf,int flen,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (fbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("mkaddrname: ent s=>%t<\n",
		sp,strlinelen(sp,sl,40)) ;
#endif

	fbuf[0] = '\0' ;
	if (sl > 0)  {
	    EMA		a ;
	    EMA_ENT	*ep ;
	    if ((rs = ema_start(&a)) >= 0) {
	        if ((rs = ema_parse(&a,sp,sl)) >= 0) {
		    const int	rsn = SR_NOTFOUND ;
		    int		i ;
		    for (i = 0 ; (rs1 = ema_get(&a,i,&ep)) >= 0 ; i += 1) {
		        rs = emaentry_addrname(ep,fbuf,flen) ;
	                len = rs ;
			if (rs != 0) break ;
		    } /* end for (ema_get) */
		    if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;
		} else if (isBadAddr(rs)) {
		    rs = SR_OK ;
	        } /* end if (ema_parse) */
	        rs1 = ema_finish(&a) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (ema) */
	} /* end if (non-zero source) */

#if	CF_DEBUGS
	debugprintf("mkaddrname: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkaddrname) */


/* local subroutines */


static int emaentry_addrname(EMA_ENT *ep,char *fbuf,int flen)
{
	int		rs = SR_OK ;
	int		nl = 0 ;
	int		atype = -1 ;
	int		len = 0 ;
	const char	*np = NULL ;

	if ((np == NULL) || (nl == 0)) {
	    if (ep->cp != NULL) {
	        atype = atype_comment ;
	        nl = sfshrink(ep->cp,ep->cl,&np) ;
	    }
	}

	if ((np == NULL) || (nl == 0)) {
	    if (ep->ap != NULL) {
	        atype = atype_address ;
	        nl = sfshrink(ep->ap,ep->al,&np) ;
	    }
	}

	if ((np == NULL) || (nl == 0)) {
	    if (ep->rp != NULL) {
	        atype = atype_route ;
	        nl = sfshrink(ep->rp,ep->rl,&np) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mkaddrname/emaentry_addrname: mid rs=%d atype=%d\n",
		rs,atype) ;
	if ((np != NULL) && (nl > 0)) {
	    debugprintf("mkaddrname/emaentry_addrname: f=>%t<\n",
		np,strlinelen(np,nl,40)) ;
	}
#endif /* CF_DEBUGS */

#if	CF_MASSAGE
	if ((np != NULL) && (nl > 0)) {
	    int		cl ;
	    cchar	*cp ;
	    switch (atype) {
	    case atype_comment:
	        if ((cl = sfsubstance(np,nl,&cp)) > 0) {
	            rs = snwcpy(fbuf,flen,cp,cl) ;
	            len = rs ;
	        }
	        break ;
	    case atype_address:
	        rs = mkdisphdr(fbuf,flen,np,nl) ;
	        len = rs ;
	        break ;
	    case atype_route:
	        rs = snwcpy(fbuf,flen,np,nl) ;
	        len = rs ;
		break ;
	    } /* end switch */
	} /* end if (positive) */
#else /* CF_MASSAGE */
	if ((np != NULL) && (nl > 0)) {
	    switch (atype) {
	    case atype_comment:
	    case atype_address:
	    case atype_route:
	        rs = snwcpy(fbuf,flen,np,nl) ;
	        len = rs ;
		break ;
	    } /* end switch */
	} /* end if (positive) */
#endif /* CF_MASSAGE */

#if	CF_DEBUGS
	debugprintf("mkaddrname/emaentry_addrname: ret rs=%d len=%u\n",
		rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (emaentry_addrname) */


static int isBadAddr(int rs)
{
	return isOneOf(rsbadaddr,rs) ;
}
/* end subroutine (emaentry_addrname) */


