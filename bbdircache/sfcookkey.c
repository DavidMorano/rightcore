/* sfcookkey */

/* string-find a cookie key */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a cookie key in a string.  

	Synopsis:

	int sfcookkey(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp	supplied string to test
	sl	length of supplied string to test
	rpp	pointer to store result "thing" pointer

	Returns:

	>=0	length of resulting key-name
	<0	no key found

	Notes:

	A zero-length key-name can be returned.  This is an error
	but it should be processed so that the consequences of the
	error can be made manifest.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#undef	CH_COOK
#define	CH_COOK		'%'


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnsub(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	getkey(const char *,int,const char *,const char **) ;


/* local variables */


/* exported subroutines */


int sfcookkey(cchar *sp,int sl,cchar **rpp)
{
	const int	sch = CH_COOK ;
	int		cl = -1 ;
	const char	*cp = NULL ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("sfcookkey: sl=%d s=>%t<\n",
		sl,sp,strlinelen(sp,sl,50));
#endif

	if (sl >= 1) {
	    const char	*ss = "{}" ;
	    const char	*tp ;
	    while ((tp = strnchr(sp,sl,sch)) != NULL) {

	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if (sl > 0) {
	            if (sp[0] != sch) {
	                cl = getkey(sp,sl,ss,&cp) ;
	                if (cl >= 0) break ;
	            }
	            sp += 1 ;
	            sl -= 1 ;
	        }

	    } /* end while */
	} /* end if */

	if (rpp != NULL)
	    *rpp = (cl >= 0) ? cp : NULL ;

#if	CF_DEBUGS
	debugprintf("sfcookkey: ret rs=%d\n",cl) ;
#endif

	return cl ;
}
/* end subroutine (sfcookkey) */


/* local subroutines */


static int getkey(cchar *sp,int sl,cchar *ss,cchar **rpp)
{
	int		cl = -1 ;
	const char	*cp = NULL ;

	if (sl > 0) {
	    if (sp[0] == ss[0]) {
	        const char	*tp ;
	        sp += 1 ;
	        sl -= 1 ;
	        if ((tp = strnchr(sp,sl,ss[1])) != NULL) {
	            cp = sp ;
	            cl = (tp-sp) ;
	        }
	    } else {
	        cp = sp ;
	        cl = 1 ;
	    }
	} /* end if */

	if (rpp != NULL) {
	    *rpp = (cl >= 0) ? cp : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("sfcookkey/getkey: ret cl=%d\n",cl) ;
#endif

	return cl ;
}
/* end subroutine (getkey) */


