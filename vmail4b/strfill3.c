/* strfill3 */

/* make an output string from three input strings */



/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

	= 2001-12-03, David A­D­ Morano
        This was updated to use 'strlcpy(3c)' when it was rumored to be coming.
        We used our own version of 'strlcpy(3c)' before it was provided by
        vendors.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a single string from two specificed
	strings.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>


/* exported subroutines */


char *strfill3(s,slen,s1,s2,s3)
char		s[] ;
int		slen ;
const char	s1[], s2[], s3[] ;
{
	int	rlen ;
	int	ml ;

	char	*bp ;


	bp = s ;
	if (slen < 0)
	    slen = (INT_MAX - 1) ;

	rlen = slen + 1 ;

/* one */

	ml = strlcpy(bp,s1,rlen) ;

	if (ml >= rlen)
	    return (s + slen) ;

	bp += ml ;
	rlen -= ml ;

/* two */

	ml = strlcpy(bp,s2,rlen) ;

	if (ml >= rlen)
	    return (s + slen) ;

	bp += ml ;
	rlen -= ml ;

/* three */

	ml = strlcpy(bp,s3,rlen) ;

	if (ml >= rlen)
	    return (s + slen) ;

	bp += ml ;
	return bp ;
}
/* end subroutine (strfill3) */



