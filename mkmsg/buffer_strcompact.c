/* buffer_strcompact */

/* buffer up a compacted string */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch but based on previous versions
        of the 'mkmsg' program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Store a source string (which is not compacted) into the "buffer"
        while compacting it.

	Synopsis:

	int buffer_strcompact(bufp,sp,sl)
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

extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int buffer_strcompact(BUFFER *bufp,const char *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		len = 0 ;
	int		c = 0 ;
	const char	*cp ;

	if (sl < 0) sl = strlen(sp) ;

	while ((cl = nextfield(sp,sl,&cp)) > 0) {
	    if (c++ > 0) {
	        rs = buffer_char(bufp,CH_SP) ;
	        len += rs ;
	    }
	    if (rs >= 0) {
	        rs = buffer_strw(bufp,cp,cl) ;
	        len += rs ;
	    }
	    sl -= ((cp+cl)-sp) ;
	    sp = (cp+cl) ;
	    if (rs < 0) break ;
	} /* end while */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (buffer_strcompact) */


