/* strlinelen */

/* special hack -- mostly for debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int strlinelen(s,slen,mlen)
	const char	s[] ;
	int		slen ;
	int		mlen ;

	Arguments:

	s	string
	slen	length of given string
	mlen	maximum length desired to be returned

	Returns:

	-	maximum length of the string up to its first NL character


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;


/* exported subroutines */


int strlinelen(cchar *sp,int sl,int mlen)
{
	int		len = 0 ;

	if (mlen < 0) mlen = 0 ;

	if ((sp != NULL) && (sl != 0) && (mlen > 0)) {
	    cchar	*tp ;
	    if (sl < 0) sl = strlen(sp) ;
	    len = MIN(sl,mlen) ;
	    if ((tp = strnchr(sp,len,'\n')) != NULL) {
	        len = (tp - sp) ;
  	        while ((len > 0) && (sp[len-1] == '\r')) {
		    len -= 1 ;
		}
	    }
	} /* end if (have stuff) */

	return len ;
}
/* end subroutine (strlinelen) */


