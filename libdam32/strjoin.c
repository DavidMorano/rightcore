/* strjoin */

/* join two strings together */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This was made specifically for the HDB UUCP modified code.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from two specificed strings.


*******************************************************************************/


#include	<envstanadrds.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>


/* exported subroutines */


char *strjoin(ofname,p1,p2)
char		ofname[] ;
const char	p1[], p2[] ;
{
	int		rlen ;
	int		ml ;
	char		*bp ;

	bp = ofname ;
	rlen = MAXPATHLEN ;

/* one */

	ml = strlcpy(bp,p1,rlen) ;

	if (ml >= rlen)
	    return (ofname + (MAXPATHLEN - 1)) ;

	bp += ml ;
	rlen -= ml ;

/* two */

	ml = strlcpy(bp,p2,rlen) ;

	if (ml >= rlen)
	    return (ofname + (MAXPATHLEN - 1)) ;

	bp += ml ;
	return bp ;
}
/* end subroutine (strjoin) */


