/* hmatch */



/*******************************************************************************

	Is the initial substring of 'hs' the specified 'ts' string?  
	Return 0 if there is no match, else we return the
	character position of the header value string.
	The match is case independent.

	ts	- string that is in the message header field
	hs	- standard header string to test against


*******************************************************************************/



#include	<sys/types.h.h>
#include	<stdlib.h>
#include	<string.h>






int hmatch(hs,ts)
char	hs[], ts[] ;
{
	char	*tp = ts , *hp = hs ;


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


