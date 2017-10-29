/* sfbasename */

/* get the base file name out of a path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine returns the length of the base name portion of the given
	path string.

	Synopsis:

	int sfbasename(sp,sl,rpp)
	const char	sp[] ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp	given path string
	sl	length of given path string (can be -1)
	rpp	result pointer of beginning of found string

	Returns:

	>0	length of found string
	==0	not found (no base-name)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* exported subroutines */


int sfbasename(cchar *sp,int sl,cchar **rpp)
{
	int		si ;

	if (sl < 0) sl = strlen(sp) ;

/* remove trailing slash characters */

	while ((sl > 1) && (sp[sl - 1] == '/')) {
	    sl -= 1 ;
	}

/* find the next previous slash character (if there is one) */

	for (si = sl ; si > 0 ; si -= 1) {
	    if (sp[si - 1] == '/') break ;
	}

	if ((sl == 1) && (si == 1) && (sp[0] == '/')) {
	    si -= 1 ;
	}

	if (rpp != NULL) {
	    *rpp = (sp + si) ;
	}

	return (sl - si) ;
}
/* end subroutine (sfbasename) */


