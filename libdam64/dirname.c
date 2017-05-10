/* dirname */

/* get the directory part out of a file name path */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***************************************************************************

	This routine returns the directory portion of a file name
	path.


	Synopsis:

	char *dirname(s)
	char	*s ;


	Arguments:

	+	string buffer address


	Returns:

		pointer to directory part of modified string buffer



***************************************************************************/



char *dirname(s)
char	*s ;
{
	int	si ;
	int	sl = strlen(s) ;


/* remove trailing slash characters */

	while ((sl > 0) && (s[sl - 1] == '/')) 
		sl -= 1 ;

/* find the next previous slash character */

	for (si = sl ; si > 0 ; si -= 1) {

	    if (s[si - 1] == '/') 
		break ;

	}

/* nuke it here */

	s[si - 1] = '\0' ;
	return s ;
}
/* end subroutine (dirname) */



