/* ffclose */

/* close a FFILE stream */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fclose(3c)' subroutine, only
	sensible!

	Synopsis:

	int ffclose(fp)
	FFILE		*fp ;

	Arguments:

	fp		pointer to a FFILE object

	Returns:

	<0		error
	>=0		OK


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */

#define	TO_AGAIN	(5*60)


/* exported subroutines */


int ffclose(fp)
FFILE		*fp ;
{
	FILE	*sfp ;

	int	rs ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

again:
	rs = fclose(sfp) ;
	if (rs == EOF) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_INTR:
		goto again ;
	    case SR_AGAIN:
	        if (to-- > 0) {
		    sleep(1) ;
		    goto again ;
		}
		break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ffclose) */



