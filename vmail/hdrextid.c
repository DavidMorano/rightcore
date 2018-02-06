/* hdrextid */

/* header-extract-id */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Here we extract an ID from a header value. The ID is only looked for in
        a "route-address" part of an address specification.

	Synopsis:

	int hdrextid(rbuf,rlen,vp,vl)
	char		rbuf[] ;
	int		rlen ;
	const char	vp[] ;
	int		vl ;

	Arguments:

	rbuf		supplied result buffer
	rlen		length of supplied result buffer
	vp		value pointer
	vl		value length

	Returns:

	<0		error
	>=0		length of result

	Notes: IDs are derived from the "route-address" in an
	email-addresses (EMAs).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ema.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkbestaddr(char *,int,cchar *,int) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsubstance(const char *,int,const char **) ;
extern int	isprintlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwset(char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hdrextid(char *rbuf,int rlen,cchar *abuf,int alen)
{
	EMA		a ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if ((rs = ema_start(&a)) >= 0) {
	    if ((rs = ema_parse(&a,abuf,alen)) >= 0) {
	        EMA_ENT	*ep ;
	        int	i ;
	        for (i = 0 ; ema_get(&a,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                int	sl = 0 ;
	                cchar	*sp = NULL ;
	                if ((ep->rp != NULL) && (ep->rl > 0)) {
			    sp = ep->rp ;
			    sl = ep->rl ;
			}
	                if (sl > 0) {
			    int		al ;
			    cchar	*ap ;
			    if ((al = sfshrink(sp,sl,&ap)) > 0) {
	                        rs = snwcpy(rbuf,rlen,ap,al) ;
	                        len = rs ;
			    }
	                }
	            } /* end if (non-null) */
		    if (len > 0) break ;
		    if (rs < 0) break ;
	        } /* end for */
	    } /* end if (parse) */
	    rs1 = ema_finish(&a) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ema) */

#if	CF_DEBUGS
	debugprintf("mkbestaddr: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (hdrextid) */


