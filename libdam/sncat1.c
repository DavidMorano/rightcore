/* sncat1 */

/* concatenate a NUL-terminated string(s) to a fixed sized destination */


/* revision history:

	= 1999-11-01, David A­D­ Morano
	This was originally written.

	= 2001-12-03, David A­D­ Morano
        This was updated to use 'strlcpy(3c)' when it was rumored to be coming.
        We used our own version of 'strlcpy(3c)' before it was provided by
        vendors.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from a single specificed
	string.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>


/* exported subroutines */


int sncat1(ds,dslen,s1)
char		ds[] ;
int		dslen ;
const char	s1[] ;
{
	int		rlen ;
	int		ml ;
	char		*bp = ds ;

	if (dslen < 0)
	    dslen = (INT_MAX - 1) ;

	rlen = dslen + 1 ;

/* one */

	ml = strlcat(bp,s1,rlen) ;

	if (ml >= rlen)
	    return SR_OVERFLOW ;

	bp += ml ;
	return (bp - ds) ;
}
/* end subroutine (sncat1) */


