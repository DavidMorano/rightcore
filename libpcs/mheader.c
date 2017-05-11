/* mheader */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Is the initial substring of 'hs' the specified 'ts' string? Return 0 if
        there is no match, else we return the character position of the header
        value string. The match is case independent.

	ts	- (test string) this is the unknown header string
	hs	- (header string) standard header string to test against


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* exported subroutines */


int mheader(hs,ts)
char	*ts, *hs ;
{
	int	l = strlen(hs) ;

	char	*cp ;


	if (strncasecmp(ts,hs,l) != 0) 
		return 0 ;

	cp = ts + l ;
	while (CHAR_ISWHITE(*cp)) 
		cp += 1 ;

	if (*cp != ':') 
		return 0 ;

	return (cp - ts + 1) ;
}
/* end subroutine (mheader) */



