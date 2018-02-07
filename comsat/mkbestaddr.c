/* mkbestaddr */

/* get the "best" address out of an EMA type address specification */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 1999-02-01, David A­D­ Morano
        This code was part of another subroutine and I pulled it out to make a
        subroutine that can be used in multiple places.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine extracts the "best" address out of an EMA-type of
        address specification (given in raw string form).

	Synopsis:

	int mkbestaddr(rbuf,rlen,sp,sl)
	char		rbuf[] ;
	int		rlen ;
	const char	*sp ;
	int		sl ;

	Arguments:

	rbuf		result buffer
	rlen		result buffer length
	sp		source string
	sl		source string length

	Returns:

	<0		error
	>=0		length of resulting string


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ema.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local subroutines */


/* exported subroutines */


int mkbestaddr(char *rbuf,int rlen,cchar *abuf,int alen)
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
	                int		sl = 0 ;
	                const char	*sp = NULL ;

			if (sl == 0) {
	                    if ((ep->rp != NULL) && (ep->rl > 0)) {
				sp = ep->rp ;
				sl = ep->rl ;
			    }
			}
			if (sl == 0) {
	                    if ((ep->ap != NULL) && (ep->al > 0)) {
				sp = ep->ap ;
				sl = ep->al ;
	                    }
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
/* end subroutine (mkbestaddr) */


