/* snwcpywidehdr */

/* counted-string copy while compacting white-space from the source */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-01-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Similar to 'snwcpycompact(3dam)' except that the source consists of 
	wide-characters.

	Synopsis:

	int snwcpywidehdr(dbuf,dlen,sp,sl)
	char		dbuf[] ;
	int		dlen ;
	const wchar_t	*sp ;
	int		sl ;

	Arguments:

	dbuf		result buffer
	dlen		length of supplied result buffer
	sp		source string
	sl		source string length

	Returns:

	<0		error
	>=0		resulting string length


	Implemetation note:
	We could have used either 'sbuf(3dam)' or 'storebuf(3dam)' or what a
	few other subroutines of this ilk have done, but this subroutine was
	written at a time before resort to those interfaces was automatic.
	It's a little messy, but it works just fine as it is!


	See-also:

	snwcpy(3dam),
	snwcpylatin(3dam), 
	snwcpyopaque(3dam), 
	snwcpywidehdr(3dam), 
	snwcpyclean(3dam), 
	snwcpyhyphen(3dam), 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stddef.h>		/* presumably for 'wchar_t' type */

#include	<vsystem.h>
#include	<ascii.h>
#include	<strmgr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	wsfnext(const wchar_t *,int,const wchar_t **) ;
extern int	wsinul(const wchar_t *) ;
extern int	isprintbad(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*straltwchar(uint) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int snwcpywidehdr(char *dbuf,int dlen,const wchar_t *wsp,int wsl)
{
	STRMGR		m ;
	int		rs ;
	int		rs1 ;
	int		dl = 0 ;

	if (dlen < 0) dlen = INT_MAX ;
	if (wsl < 0) wsl = wsinul(wsp) ;

	if ((rs = strmgr_start(&m,dbuf,dlen)) >= 0) {
	    int			wl ;
	    const wchar_t	*wp ;
	    while ((wl = wsfnext(wsp,wsl,&wp)) > 0) {
	        if (dl > 0) {
	            rs = strmgr_char(&m,' ') ;
	            if (rs >= 0) dl += 1 ;
	        }
	        if (rs >= 0) {
	            uint	wch ;
	            int		i ;
	            for (i = 0 ; (rs >= 0) && (i < wl) && wp[i] ; i += 1) {
	                if ((wch = (uint) wp[i]) > UCHAR_MAX) {
			    cchar	*ss = straltwchar(wch) ;
			    if (ss != NULL) {
	                        rs = strmgr_str(&m,ss,-1) ;
			    } else {
			        wch = '¿' ;
	                        rs = strmgr_char(&m,wch) ;
			    }
			} else {
			    if (isprintbad(wch)) wch = '¿' ;
	                    rs = strmgr_char(&m,wch) ;
			}
	            } /* end for */
	            if (rs >= 0) dl += i ;
	        }
	        wsl -= ((wp+wl) - wsp) ;
	        wsp = (wp+wl) ;
	        if (rs < 0) break ;
	    } /* end while (looping through string pieces) */
	    rs1 = strmgr_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (strmgr) */

	dbuf[dl] = '\0' ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (snwcpywidehdr) */


