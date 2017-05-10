/* strncasecmp */


#include	<envstandards.h>
#include	<char.h>



int strncasecmp(s1,s2,n)
char	*s1, *s2 ;
int	n ;
{
	int	i = n ;

	while ((n > 0) && *s1 && *s2) {

	    if (CHAR_TOLC(*s1) != CHAR_TOLC(*s2)) 
		return 1 ;

	    s1 += 1 ;
	    s2 += 1 ;
	    n -= 1 ;

	} /* end while */

	return (*s1 == *s2) ? 0 : n ;
}
/* end subroutine (strncasecmp) */


