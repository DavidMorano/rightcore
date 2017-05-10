/* ematch */


/* revision history:

	= 1995-04-01, David A­D­ Morano

	This is part of our cleanup-compatibility effort.


*/


/***************************************************************************

	Is the initial substring of 'ts' the specified 'hs' string?
	Return 0 if there is no match, else we return the character
	position of the header value string.  The match is case
	independent.

	Arguments:

	string		possible envelope
	len		len

	Returns:

	>0		the string contained an envelope
	==0		the string did not contain an evelope


****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>
#include	<ctype.h>

#include	<char.h>

#include	"localmisc.h"




int ematch(s,len)
char	s[] ;
int	len ;
{
	int	f_len, f_esc ;

	char	*cp = s ;


	if ((! (f_esc = (strcmp(s,">From ") == 0))) &&
		(strcmp(s,"From ") != 0)) return 0 ;

	f_len = (len >= 0) ;
	cp += 5 ;
	len -= 5 ;
	if (f_esc) {
		cp += 1 ;
		len -= 1 ;
	}

	while (((! f_len) || (len > 0)) && CHAR_ISWHITE(*cp)) {
		cp += 1 ;
		len -= 1 ;
	}
	
	if (isalpha(*cp)) 
		return (cp - s) ;

	return 0 ;
}
/* end subroutine (ematch) */



