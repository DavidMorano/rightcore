/* ffseek */

/* read a block of (binary) data from the STDIO stream */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fseek(3c)' subroutine, only
	sensible!

	Synopsis:

	int ffseek(fp,off,whence)
	FFILE		*fp ;
	off_t		off ;
	int		whence ;

	Arguments:

	fp		pointer to a FFILE object
	off		file offset
	whence		position indicator

	Returns:


	<0		an error occurred
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


int ffseek(fp,off,whence)
FFILE		*fp ;
off_t		off ;
int		whence ;
{
	FILE	*sfp ;

	int	rs ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

again:
	rs = fseeko(sfp,off,whence) ;
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
/* end subroutine (ffseek) */



