/* hmatch */


#define	CF_DEBUGS	0


/* revision history:

	= David A.D. Morano, 94/01/15
	This subroutine was adapted from a previous similar one
	in the PCS package.

*/


/***************************************************************************

	Is the initial substring of 'ts' the specified 'hs' string ?  
	Return 0 if there is no match, else we return the
	character position of the header value string.
	The match is case independent.

	Arguments:
	hs		header string to search for
	ts		string to test for above header

	Returns:
	>0		got a match
	==0		did not get a match


****************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<string.h>
#include	<errno.h>

#include	"localmisc.h"




int hmatch(hs,ts)
char	hs[], ts[] ;
{
	char	*tp = ts, *hp = hs ;


	while (*hp && (*hp != ':')) {

	    if (LOWER(*tp) != LOWER(*hp)) return 0 ;

	    tp += 1 ;
	    hp += 1 ;
	}

	while (ISWHITE(*tp)) tp += 1 ;

	if (*tp != ':') return 0 ;

	return (tp + 1 - ts) ;
}
/* end subroutine (hmatch) */



