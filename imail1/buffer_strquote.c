/* buffer_strquote */

/* take a string and insert it into the buffer in quoted form */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch but was based on code that was 
	in the 'mkmsg' program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This method (for object BUFFER) takes the given string and inserts it
	into the "buffer" after it has been shell-quoted.

	Synopsis:

	int buffer_strquote(bufp,sp,sl)
	BUFFER		*bufp ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	bufp		pointer to BUFFER object
	sp		pointer to string
	sl		length of string

	Returns

	<0		error
	>=0		length of string buffered


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpycompact(char *,int,const char *,int) ;
extern int	haswhite(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int buffer_strquote(BUFFER *bufp,const char *sp,int sl)
{
	const int	qch = CH_DQUOTE ;
	int		rs ;

	if (sl < 0) sl = strlen(sp) ;

	if (strnpbrk(sp,sl," \t\r\n\v\f\b\"\\") != NULL) {
	    const int	size = ((2*sl)+3) ;
	    const char	*tp ;
	    char	*ap ;
	    if ((rs = uc_malloc(size,&ap)) >= 0) {
		char	*bp = ap ;
		*bp++ = qch ;
		while ((tp = strnpbrk(sp,sl,"\"\\")) != NULL) {
		    bp = strwcpy(bp,sp,(tp-sp)) ;
		    *bp++ = CH_BSLASH ;
		    *bp++ = *tp ;
		    sl -= ((tp+1)-sp) ;
		    sp = (tp+1) ;
		} /* end while */
		if (sl > 0) {
		    bp = strwcpy(bp,sp,sl) ;
		}
		*bp++ = qch ;
		rs = buffer_strw(bufp,ap,(bp-ap)) ;
		uc_free(ap) ;
	    } /* end if (memory-allocation) */
	} else {
	    rs = buffer_strw(bufp,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (buffer_strquote) */


