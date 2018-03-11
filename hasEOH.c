/* hasEOH */

/* check for an End-Of-Header (EOH) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Determine if the given string consists of an End-Of-Header (EOH)
        sequence.

	An EOH is a leadering blank like of two sorts:

	<NL>
	<CR><NL>

	Synopsis:

	int hasEOH(cchar *sp,int sl)

	Arguments:

	sp		string to test
	sl		length of strin to test

	Returns:

	FALSE		assertion fails
	TRUE		assertion succeeds


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasEOH(cchar *sp,int sl)
{
	int		f = FALSE ;
	if (sl >  0) {
	    f = (sp[0] == '\n') ;
	    f = f || ((sl > 1) && (sp[0] == '\r') && (sp[1] == '\n')) ;
	}
	return f ;
}
/* end subroutine (hasEOH) */


