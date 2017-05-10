/* cfhexs */

/* convert from a HEX string */


/* revision history:

	= 1998-10-01, David A­D­ Morano

	This subroutine was written adapted from assembly.
	The original assembly goes wa...ay bad!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine converts a string of HEX digits into a character
	string.  Every two hexadecimal digits are converted into one
	character.

; routine to convert a left justified HEX string to its binary value

;	Arguments:

;	- address of string to be converted
	- len of source address to convert
;	- address of buffer to store result

;	Outputs :

;	- return status is zero for OK else not zero


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* exported subroutines */


int cfhexs(sbuf,slen,rp)
char		sbuf[] ;
int		slen ;
unsigned char	*rp ;
{
	int	v, i ;
	int	ch ;


	if (slen < 0)
	    slen = strlen(sbuf) ;

	while (slen && sbuf[0] && isspace(sbuf[0])) {
	    sbuf += 1 ;
	    slen -= 1 ;
	}

	for (i = 0 ; (i < slen) && (sbuf[i] != 0) ; i += 1) {

	    ch = (sbuf[i] & 0xff) ;
	    if (isspace(ch))
		break ;

	    if (! isxdigit(ch))
	        return SR_INVALID ;

	    if (isalpha(ch)) {
	        v = 10 + tolower(ch) - 'a' ;

	    } else 
	        v = ch - '0' ;

	    if (i & 1) {
	        rp[(i - 2) >> 1] |= (v & 15) ;

	    } else
	        rp[(i - 2) >> 1] = ((v & 15) << 4) ;

	} /* end for */

	return (i & 1) ? SR_INVALID : i ;
}
/* end subroutine (cfhexs) */



