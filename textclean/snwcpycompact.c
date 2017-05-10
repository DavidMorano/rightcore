/* snwcpycompact */

/* counted-string copy while compacting white-space from the source */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Similar to 'snwcpy(3dam)' except that we copy the source to the
	destination while removing extra white-space from the source.

	Synopsis:

	int snwcpycompact(dbuf,dlen,sp,sl)
	char		dbuf[] ;
	int		dlen ;
	cchar		*sp ;
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
        We could have used either 'sbuf(3dam)' or 'storebuf(3dam)' or some other
        subroutines of this ilk, but this subroutine was written at a time
        before resort to those interfaces was automatic. It's a little messy,
        but it works just fine as it is!


	See-also:

	snwcpy(3dam),
	snwcpylatin(3dam), 
	snwcpyopaque(3dam), 
	snwcpycompact(3dam), 
	snwcpyclean(3dam), 
	snwcpyhyphen(3dam), 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<strmgr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	sncpyuc(char *,int,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	nextfield(cchar *,int,cchar **) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,cchar *) ;
extern char	*strdcpy2(char *,int,cchar *,cchar *) ;
extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*strwcpyblanks(char *,int) ;	/* NUL-terminaed */
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminated */
extern char	*strwset(char *,int,int) ;
extern char	*strnset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int snwcpycompact(char *dbuf,int dlen,cchar *sp,int sl)
{
	STRMGR		m ;
	int		rs ;
	int		rs1 ;
	int		dl = 0 ;

	if (dlen < 0) dlen = INT_MAX ;
	if (sl < 0) sl = strlen(sp) ;

	if ((rs = strmgr_start(&m,dbuf,dlen)) >= 0) {
	    int		cl ;
	    cchar	*cp ;
	    while ((cl = nextfield(sp,sl,&cp)) > 0) {
	        if (dl > 0) {
	            rs = strmgr_char(&m,' ') ;
	            if (rs >= 0) dl += 1 ;
	        }
	        if (rs >= 0) {
	            rs = strmgr_str(&m,cp,cl) ;
	            if (rs >= 0) dl += cl ;
	        }
	        sl -= ((cp+cl) - sp) ;
	        sp = (cp+cl) ;
	        if (rs < 0) break ;
	    } /* end while (looping through string pieces) */
	    rs1 = strmgr_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (strmgr) */

	dbuf[dl] = '\0' ;
	return (rs >= 0) ? dl : rs ;
}
/* end subroutine (snwcpycompact) */


