/* strnlead */

/* check if string 's2' is a leading substring of string 's1' */



/* revision history:

	= 82/11/01, David A­D­ Morano

	Originally written for Audix Database Processor work.


*/


/******************************************************************************

	This subroutine returns TRUE if 's2' is an initial substring of
	's1'.  But only up to the maximum number of characters are
	checked.


******************************************************************************/


#include	<sys/types.h>
#include	<string.h>

#include	"localmisc.h"





int strnlead(s1,s2,lr)
const char	*s1, *s2 ;
int		lr ;
{


	if (lr < 0)
	    lr = strlen(s2) ;

	while (lr-- > 0) {

	    if (*s2++ != *s1++)
	        return FALSE ;

	} /* end while */

	return TRUE ;
}
/* end subroutine (strnlead) */



