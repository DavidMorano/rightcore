/* btell64 */

/* "Basic I/O" package (BIO) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* exported subroutines */


int btell64(fp,rp)
bfile		*fp ;
off64_t		*rp ;
{
	offset_t	oo ;

	int	rs ;


	rs = btell(fp,&oo) ;

	if (rp != NULL)
	    *rp = (rs >= 0) ? ((off64_t) oo) : 0 ;

	return rs ;
}
/* end subroutine (btell64) */



