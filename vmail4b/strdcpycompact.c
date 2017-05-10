/* strdcpycompact */

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

	int strdcpycompact(dbuf,dlen,sp,sl)
	char		dbuf[] ;
	int		dlen ;
	const char	*sp ;
	int		sl ;

	Arguments:

	dbuf		result buffer
	dlen		length of supplied result buffer
	sp		source string
	sl		source string length

	Returns:

	-		pointer to character at end of result string


	See also:

	strdcpy1(3dam),
	strdcpyopaque(3dam),
	strdcpyclean(3dam),

	Notes:

        + The semantic here is to fill up the destination buffer as much as
        possible. There is no overflow indication returned to the caller. But
        since we are looping on chunks of source string separated by
        white-space, what if we get a chunk that is too big to fix into the
        remaining destriantion? We take an extra step to detect this situation
        and compensate.

        + We could have used either 'sbuf(3dam)' or 'storebuf(3dam)' or a few
        other subroutines of this ilk, but this subroutine was written at a time
        before resort to those interfaces was automatic. It's a little messy,
        but it works just fine as it is!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<ascii.h>
#include	<strmgr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnrpbrk(const char *,int,const char *) ;
extern char	*strwcpyblanks(char *,int) ;	/* NUL-terminaed */
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminated */
extern char	*strwset(char *,int,int) ;
extern char	*strnset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


char *strdcpycompact(char *dbuf,int dlen,cchar *sp,int sl)
{
	STRMGR		m ;
	int		rs ;
	int		dl = 0 ;

	if (dbuf == NULL) return NULL ;

	if (dlen < 0) dlen = INT_MAX ;
	if (sl < 0) sl = strlen(sp) ;

	if ((rs = strmgr_start(&m,dbuf,dlen)) >= 0) {
	    int		cl ;
	    const char	*cp ;
	    while ((cl = nextfield(sp,sl,&cp)) > 0) {
		if (dl > 0) {
	            if ((rs = strmgr_char(&m,CH_SP)) >= 0) {
		        dl += 1 ;
		    }
		}
		if (rs >= 0) {
	            if ((rs = strmgr_str(&m,cp,cl)) >= 0) {
		        dl += cl ;
		    } else if (rs == SR_OVERFLOW) {
			if ((rs = strmgr_rem(&m)) > 0) {
			    const int	ml = MIN(rs,cl) ;
	            	    if ((rs = strmgr_str(&m,cp,ml)) >= 0) {
				dl += ml ;
			    }
			}
		    }
	        }
	        sl -= ((cp+cl) - sp) ;
	        sp = (cp+cl) ;
	        if (rs < 0) break ;
	    } /* end while (looping through string pieces) */
	    strmgr_finish(&m) ;
	} /* end if (strmgr) */

	dbuf[dl] = '\0' ;
	return (dbuf+dl) ;
}
/* end subroutine (strdcpycompact) */


