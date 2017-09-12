/* cmpc */

/* routine to compare two character strings for equallity */
/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


/* exported subroutines */


int cmpc(len,src,dst)
int		len ;
const char	*src, *dst ;
{
	int		i ;
	int		rc = 0 ;

	for (i = 0 ; i < len ; i += 1) {
	    rc = (*src++ - *dst++) ;
	    if (rc != 0) break ;
	}

	return rc ;
}
/* end subroutine (cmpc) */


