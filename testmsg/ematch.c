/* ematch */


#define	CF_DEBUGS	0


/* revision history:

	= David A.D. Morano


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***************************************************************************

	Is the initial substring of 'ts' the specified 'hs' string ?  
	Return 0 if there is no match, else we return the
	character position of the header value string.
	The match is case independent.

	Synopsis:

	int ematch(s,len)
	char	s[] ;
	int	len ;


	Arguments:

	string		possible envelope
	len		len


	Returns:

	>0		the string contained an envelope
	==0		the string did not contain an evelope



****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
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

	char	*cp ;


#if	CF_DEBUGS
	debugprintf("ematch: entered> %s\n",s) ;
#endif

#ifndef	CF_DEBUGS
	if ((! (f_esc = (strncmp(s,">From ",6) == 0))) &&
		(strncmp(s,"From ",5) != 0)) 
		return 0 ;
#else
	f_esc = (strncmp(s,">From ",6) == 0) ;

	debugprintf("ematch: f_esc=%d regular=%d\n",
		f_esc,
		(strncmp(s,"From ",5) != 0)) ;

	if ((! f_esc) && (strncmp(s,"From ",5) != 0)) 
		return 0 ;
#endif

#if	CF_DEBUGS
	debugprintf("ematch: match so far\n") ;
#endif

	f_len = (len >= 0) ;
	cp = s + 5 ;
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



