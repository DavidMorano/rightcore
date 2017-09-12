/* strdcpyopaque */

/* special (excellent) string-copy type of subroutine! */


/* revision history:

	= 1998-07-08, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is essentially the same as the 'strdcpy1(3dam)' subroutine except
        that white-space characters not copied over to the result.

	Synopsis:

	char *strdcpyopaque(dp,dl,sp,sl)
	char		*dp ;
	int		dl ;
	const char	*sp ;
	int		sl ;

	Arguments:

	dp		destination string buffer
	dl		destination string buffer length
	sp		source string
	sl		source string length

	Returns:

	-		pointer to end of destination string

	See-also:

	strdcpy1(3dam),
	strdcpycompact(3dam),
	strdcpyclean(3dam),


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* exported subroutines */


char *strdcpyopaque(char *dp,int dl,const char *sp,int sl)
{
	int		ch ;

	while (sl && *sp)  {
	    ch = MKCHAR(*sp) ;
	    if (! CHAR_ISWHITE(ch)) {
	        if (dl-- == 0) break ;
	        *dp++ = (char) ch ;
	    } /* end if */
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strdcpyopaque) */


