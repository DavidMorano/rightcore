

#include	"misc.h"



int opt_nmatch(os,s,l)
char	*os[] ;
char	*s ;
int	l ;
{
	int	i = 0 ;


	if (l < 0) l = strlen(s) ;

	while ((os[i] != NULL) && (os[i][0] != '\0')) {

	    if (strnlead(os[i],s,l)) return i ;

	    i += 1 ;
	}

	return -1 ;
}
/* end subroutine (opt_nmatch) */



