/* hasdots */

/* does the given string lead with two dot characters? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-07-13, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if the given string has two leading dot
	characters.

	Synopsis:

	int hasdots(const char *sp,int sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp	pointer to given string
	sl	length of given string

	Returns:

	==0	string does not have two leading dots
	>0	string has the two leading dots


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int hasdots(cchar *np,int nl)
{
	int		f = FALSE ;

	if (np[0] == '.') {
	    if (nl < 0) nl = strlen(np) ;
	    f = f || (nl == 1) ;
	    f = f || ((nl == 2) && (np[1] == '.')) ;
	}

	return f ;
}
/* end subroutine (hasdots) */


