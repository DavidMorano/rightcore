/* mkfmtphone */

/* similar to 'snwcpy(3dam)' but formatting a phone number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-03-14, David A­D­ Morano
        This subroutine was originally rwritten.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We format a raw phone number to make it look more pretty.

	See-also:

	snwcpy(3dam),
	snwcpylatin(3dam), 
	snwcpyopaque(3dam), 
	snwcpycompact(3dam), 
	snwcpyclean(3dam), 
	snwcpyhyphen(3dam),


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpyopaque(char *,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkfmtphone(char *dbuf,int dlen,cchar *pp,int pl)
{
	int		dl = 0 ;
	int		rs ;
	int		rs1 ;
	int		sl ;
	cchar		*sp ;

	if ((sl = sfshrink(pp,pl,&sp)) > 0) {
	    if (strnpbrk(sp,sl,"-()") == NULL) {
	        const int	tlen = sl ;
	        char		*tbuf ;
	        if ((rs = uc_malloc((tlen+1),&tbuf)) >= 0) {
	            if ((rs = snwcpyopaque(tbuf,tlen,sp,sl)) >= 0) {
			SBUF		b ;
		        const int	tl = rs ;
			if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
			    if (tl > 10) {
				rs = sbuf_strw(&b,tbuf,(tl-10)) ;
			    }
		            if ((rs >= 0) && (tl >= 10)) {
				cchar	*tp = (tbuf+tl-10) ;
				sbuf_char(&b,CH_LPAREN) ;
				sbuf_strw(&b,tp,3) ;
				sbuf_char(&b,CH_RPAREN) ;
			    }
			    if ((rs >= 0) && (tl >= 7)) {
				cchar	*tp = (tbuf+tl-7) ;
				sbuf_strw(&b,tp,3) ;
				sbuf_char(&b,CH_MINUS) ;
			    }
			    if (rs >= 0) {
 			        if (tl >= 4) {
				    cchar	*tp = (tbuf+tl-4) ;
				    sbuf_strw(&b,tp,4) ;
			        } else {
				    sbuf_strw(&b,tbuf,tl) ;
				}
			    }
			    dl = sbuf_finish(&b) ;
			    if (rs >= 0) rs = dl ;
			} /* end if (sbuf) */
	            } /* end if (sncpyopaque) */
	            rs1 = uc_free(tbuf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (m-a-f) */
	    } else {
		rs = snwcpy(dbuf,dlen,sp,sl) ;
		dl = rs ;
	    }
	} /* end if (sfshrink) */

	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (mkfmtphone) */


