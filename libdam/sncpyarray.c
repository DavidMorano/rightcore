/* sncpyarray */

/* string-copy-array-of-strings */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies the successive elements of an array of strings
	to the given destination (result) buffer.

	Synopsis:
	int sncpyarray(rbuf,rlen,a,n)
	char		rbuf[] ;
	int		rlen ;
	const char	*a[] ;
	uint		n ;

	Arguments:
	rbuf		result buffer
	rlen		size of supplied result buffer
	a		array of strings
	v		number of strings in array

	Returns:
	>=0		resulting length
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* external subroutines */


/* exported subroutines */


int sncpyarray(char *dbuf,int dlen,cchar *a[],int n)
{
	int		rs = SR_OK ;
	int		i ;
	int		si = 0 ;

	if (n < 0) n = INT_MAX ;

	for (i = 0 ; (rs >= 0) && (i < n) && a[i] ; i += 1) {
	    if (a[i][0] != '\0') {
	        rs = storebuf_strw(dbuf,dlen,si,a[i],-1) ;
		si += rs ;
	    }
	}

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (sncpyarray) */


