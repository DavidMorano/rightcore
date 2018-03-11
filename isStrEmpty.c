/* isStrEmpty */

/* determine if the given string is empty */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine determines if a given string is empty or not.

	Synopsis:

	int isStrEmpty(cchar *sp,int sl)

	Arguments:

	sp		string pointer
	sl		string length

	Returns:

	1		TRUE (empty)
	0		FALSE (not empty)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	hasallwhite(cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isStrEmpty(cchar *sp,int sl)
{
	int		f = TRUE ;
	if (sp != NULL) {
	    f = hasallwhite(sp,sl) ;
	}
	return f ;
}
/* end subroutine (isStrEmpty) */


