/* fftell */

/* knock off of 'ftell(3c)' */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'ftell(3c)' subroutine, only
	sensible!

	Synopsis:

	int fftell(fp,offp)
	FFILE		*fp ;
	off_t		*offp ;

	Arguments:

	fp		pointer to a FILE object
	offp		pointer to variable to hold file offset

	Returns:

	<0		an error occurred
	>=0		OK


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdio.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */

#define	TO_AGAIN	(5*60)


/* exported subroutines */


int fftell(fp,offp)
FFILE		*fp ;
off_t		*offp ;
{
	FILE	*sfp ;

	off_t	soff ;

	int	rs = SR_OK ;
	int	to = TO_AGAIN ;

	if (fp == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

again:
	soff = ftello(sfp) ;
	if (soff == -1) rs = (- errno) ;

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

	if (offp != NULL) *offp = soff ;

	if (rs >= 0) rs = (soff & LONG_MAX) ;

	return rs ;
}
/* end subroutine (fftell) */



