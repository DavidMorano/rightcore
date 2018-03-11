/* hasINET4AddrStr */

/* test whether the given string contains an INET4 address string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test whether the given string consists of an INET4 address string.

	Synopsis:

	int hasINET4AddrStr(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp		string to test
	sl		length of strin to test

	Returns:

	FALSE		assertion fails
	TRUE		assertion succeeds


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	hasalldig(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	hasINET4Num(cchar *,int) ;


/* local variables */


/* exported subroutines */


int hasINET4AddrStr(cchar *sp,int sl)
{
	int		f = TRUE ;
	int		c = 0 ;
	cchar		*tp ;
	if (sl < 0) sl = strlen(sp) ;
	while ((tp = strnchr(sp,sl,'.')) != NULL) {
	    f = hasINET4Num(sp,(tp-sp)) ;
	    if (! f) break ;
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    c += 1 ;
	} /* end while */
	if (f && (sl > 0)) {
	    c += 1 ;
	    f = hasINET4Num(sp,sl) ;
	} /* end if */
	if (f && (c != 4)) {
	    f = FALSE ;
	}
#if	CF_DEBUGS
	debugprintf("hasINET4AddrStr: ret f=%u\n",f) ;
#endif
	return f ;
}
/* end subroutine (hasINET4AddrStr) */


/* local subroutines */


static int hasINET4Num(cchar *sp,int sl)
{
	int		f = FALSE ;
#if	CF_DEBUGS
	   debugprintf("hasINET4AddrStr: sl=%d s=>%t<\n",sl,sp,sl) ;
#endif
	if (hasalldig(sp,sl)) {
	    f = (sl <= 3) ;
	}
	return f ;
}
/* end subroutine (hasINET4Num) */


