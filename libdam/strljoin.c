/* strljoin */

/* make a file name from two parts */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This was made specifically for the HDB UUCP modified code.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from two specificed strings.

	Synopsis:

	int strljoin(ofname,rlen,p1,p2)
	char		ofname[] ;
	int		rlen ;
	const char	p1[], p2[] ;

	Arguments:

	ofname		suppled output buffer
	rlen		length of supplied output buffer
	p1		string 1
	p2		string 2

	Returns:

	>=0		length of resulting string
	<0		overflow error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int strljoin(ofname,rlen,p1,p2)
char		ofname[] ;
int		rlen ;
const char	p1[], p2[] ;
{
	int		ml ;
	char		*bp ;

	bp = ofname ;
	if (rlen < 0)
	    rlen = MAXPATHLEN ;

/* one */

	ml = strlcpy(bp,p1,rlen) ;

	if (ml >= rlen)
		return SR_OVERFLOW ;

	bp += ml ;
	rlen -= ml ;

/* two */

	ml = strlcpy(bp,p2,rlen) ;

	if (ml >= rlen)
		return SR_OVERFLOW ;

	bp += ml ;
	return (bp - ofname) ;
}
/* end subroutine (strljoin) */


