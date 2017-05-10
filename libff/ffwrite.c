/* ffwrite */

/* write binary data to a STDIO stream */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fwrite(3c)' subroutine, only
	sensible!


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */

#define	TO_AGAIN	(5*60)


/* exported subroutines */


int ffwrite(fp,buf,len)
FFILE		*fp ;
const void	*buf ;
int		len ;
{
	FILE	*sfp ;

	int	rs ;
	int	to = TO_AGAIN ;

	if (fp == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (len < 0) len = strlen(buf) ;

	if (len == 0) return 0 ;

	sfp = fp->sfp ;

again:
	rs = fwrite(buf,1,len,sfp) ;
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

	return rs ;
}
/* end subroutine (ffwrite) */



