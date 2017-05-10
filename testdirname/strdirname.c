/* strdirname */

/* get the directory part out of a file name path */


/* revision history:

	= 1998-08-27, David A­D­ Morano
        This is a replacement for some systems (UNIX yes, but not all others --
        hence this code up) that do not have a 'dirname(3c)' type of subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine returns the directory portion of a file name path.

	Synopsis:

	char *strdirname(s)
	char	s[] ;

	Arguments:

	s	string buffer address

	Returns:

	-	pointer to directory part of modified string buffer


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>


/* local variables */

static const char	*const strdirname_dot = "." ;


/* exported subroutines */


char *strdirname(char *s)
{
	int		si ;
	int		sl = strlen(s) ;

/* remove trailing slash characters */

	while ((sl > 1) && (s[sl - 1] == '/'))
	    sl -= 1 ;

/* find the next previous slash character (if there is one) */

	for (si = sl ; si > 0 ; si -= 1) {
	    if (s[si - 1] == '/') break ;
	}

/* nuke it here */

	if (si > 1) {
	    s[si - 1] = '\0' ;
	} else if (si == 1) {
	    s[si] = '\0' ;
	} else if (si == 0) {
	    s = (char *) strdirname_dot ;
	}

	return s ;
}
/* end subroutine (strdirname) */


