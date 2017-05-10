/* bfile_strbasename */

/* get the base file name out of a path */


#define	CF_DEBUGS	0


/* revision history:

	= 83/07/19, David A­D­ Morano

	Originally written for PPI formware.


*/


/*************************************************************************

	This routine returns the poiner in the given string
	of the start of the basename portion.


**************************************************************************/






char *bfile_strbasename(s)
char	*s ;
{
	int	si ;
	int	sl = strlen(s) ;


/* remove trailing slash characters */

	while ((sl > 1) && (s[sl - 1] == '/')) 
		sl -= 1 ;

	s[sl] = '\0' ;

/* find the next previous slash character */

	for (si = sl ; si > 0 ; si -= 1) {

	    if (s[si - 1] == '/') 
		break ;

	}

#ifdef	COMMENT
	if ((si > 0) && (s[si] == '\0')) 
		return (s + si - 1) ;
#else
	if (s[1] == '\0') 
		return s ;
#endif

	return (s + si) ;
}
/* end subroutine (bfile_strbasename) */



