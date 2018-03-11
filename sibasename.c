/* sibasename */

/* get the base file name out of a file-path */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine returns the index within the given string to the start of
	the base-name portion of the given path string.

	Synopsis:

	int sibasename(sp,sl)
	const char	sp[] ;
	int		sl ;

	Arguments:

	sp	given path string
	sl	length of given path string (can be -1)

	Returns:

	+	index of found string


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sibasename(const char *sp,int sl)
{
	int		si ;

	if (sl < 0) 
	    sl = strlen(sp) ;

/* remove trailing slash characters */

	while ((sl > 0) && (sp[sl - 1] == '/'))  {
	    sl -= 1 ;
	}

/* find the next previous slash character (if there is one) */

	for (si = sl ; si > 0 ; si -= 1) {
	    if (sp[si - 1] == '/') break ;
	}

	return si ;
}
/* end subroutine (sibasename) */


