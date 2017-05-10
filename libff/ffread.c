/* ffread */

/* read a block of (binary) data from the STDIO stream */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fread(3c)' subroutine, only sensible!

	Synopsis:

	int ffread(fp,buf,len)
	FFILE		*fp ;
	char		buf[] ;
	int		len ;

	Arguments:

	fp		pointer to a FFILE object
	buf		user supplied buffer
	len		size of user supplied buffer

	Returns:

	<0		an error occurred
	==0		no data was returned
	>0		size of data returned


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


int ffread(fp,buf,len)
FFILE		*fp ;
void		*buf ;
int		len ;
{
	FILE	*sfp ;

	int	rs = SR_OK ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (len < 0) return SR_INVALID ;

	sfp = fp->sfp ;

again:
	if (len > 0) {
	    rs = fread(buf,1,len,sfp) ;
	    if (rs == 0) {
		if (ferror(sfp)) {
		    rs = (- errno) ;
		} else if (! feof(sfp)) {
		    rs = SR_NOANODE ;
		}
	        clearerr(sfp) ;
	    }
	} /* end if (non-zero length) */

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
/* end subroutine (ffread) */



