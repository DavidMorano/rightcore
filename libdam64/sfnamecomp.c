/* sfnamecomp */

/* find something inside a string */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We find some sort of sub-string within the given string. We remove any
        slashes from the beginning of the string numbering more than two, and we
        remove all trailing slash characters.

	Synopsis:

	int sfnamecomp(cchar *sp,int sl,cchar **spp)

	Arguments:

	sp		pointer to start of user supplied buffer
	sl		length of user supplied buffer
	spp		pointer to pointer of the found field

	Returns:

	>0		length of found field
	==0		no field found or a NL character was encountered


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* local defines */


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int sfnamecomp(cchar *sp,int sl,cchar **rpp)
{
	if (sl < 0) sl = strlen(sp) ;
	if (sl > 1) {
	    while ((sl > 1) && (sp[0] == '/') && (sp[1] == '/')) {
	        sp += 1 ;
	        sl -= 1 ;
	    } /* end while */
	    while ((sl > 1) && (sp[sl-1] == '/')) {
	        sl -= 1 ;
	    }
	} /* end if */
	if (rpp != NULL) *rpp = sp ;
	return sl ;
}
/* end subroutine (sfnamecomp) */


