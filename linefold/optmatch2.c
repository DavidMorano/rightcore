/* option match 2 */

/* save as 'optmatch' but MUST match at least 2 characters */


#define	F_DEBUGS	0


/* revision history :

	= 94/04/10, David A­D­ Morano

	This subroutine was originally written.


*/


/****************************************************************************** 

	Check that the given string matches the LEADING string of some
	string in the given array of strings.  If we get a match, we
	return the array index.  If we do not match, we return
	"less-than-zero".


******************************************************************************/



#include	<string.h>

#include	"misc.h"



/* local defines */

#define	MINMATCH	2






int optmatch2(os,s,len)
char	*os[] ;
char	*s ;
int	len ;
{
	int	i ;
	int	sl ;


	if (len < 0)
	    len = strlen(s) ;

	for (i = 0 ; os[i] != NULL ; i += 1) {

	    if (strncmp(os[i],s,len) == 0) {

	        if (len >= MINMATCH)
	            return i ;

	        if (strlen(os[i]) < MINMATCH)
	            return i ;

	    } /* end if */

	} /* end for */

	return -1 ;
}
/* end subroutine (optmatch2) */



