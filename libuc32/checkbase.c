/* basallbase */

/* test whether a string is composed of all characters of a given base */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        This subroutine is mostly used as a utility function to determine
        (verify) that all of the characters in a counted string are consistent
        to be be used as digits within the supplied numeric base.

	Synopsis:

	int checkbase(sp,sl,b)
	const char	*sp ;
	int		sl ;
	int		b ;

	Arguments:

	sp		string to test
	sl		length of strin to test
	b		base to check against

	Returns:

	>=0		OK
	<0		bad


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	hasallbase(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int checkbase(cchar *sp,int sl,int b)
{
	return (hasallbase(sp,sl,b)) ? SR_OK : SR_DOM ;
}
/* end subroutine (checkbase) */


