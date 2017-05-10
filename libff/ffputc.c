/* ffputc */

/* write binary data to a FFILE stream */


#define	CF_FPUTC	0	/* faster or not? */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fputc(3c)' subroutine, only
	sensible!


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


int ffputc(fp,c)
FFILE	*fp ;
int	c ;
{
	FILE	*sfp ;

	int	rs ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;

	sfp = fp->sfp ;

again:
	rs = fputc(c,sfp) ;
	if (rs == EOF) {
	    rs = (- errno) ;
	    clearerr(sfp) ;
	}

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

	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (ffputc) */



