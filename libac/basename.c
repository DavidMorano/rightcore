/* basename */

/* get the base file name out of a path */



/******************************************************************************

	This routine returns the pointer in the given string of the
	start of the basename portion.


	Synopsis:

	char *basename(s)
	char	*s ;




******************************************************************************/



char *basename(s)
char	*s ;
{
	int	si ;
	int	sl = strlen(s) ;


/* remove trailing slash characters */

	while ((sl > 0) && (s[sl - 1] == '/')) 
		sl -= 1 ;

	s[sl] = '\0' ;

/* find the next previous slash character */

	for (si = sl ; si > 0 ; si -= 1) {

	    if (s[si - 1] == '/') 
		break ;

	}

	return (s + si) ;
}
/* end subroutine (basename) */



