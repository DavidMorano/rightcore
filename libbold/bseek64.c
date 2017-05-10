/* bseek64 */

/* "Basic I/O" package (BIO) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Note on the 'whence' argument:
	SEEK_SET	0 = from beginning of file
	SEEK_CUR	1 = from current pointer of file
	SEEK_END	2 = from end of file


******************************************************************************/


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

extern int	bfile_flush(bfile *) ;


/* external variables */


/* exported subroutines */


int bseek64(fp,o64,whence)
bfile		*fp ;
off64_t		o64 ;
int		whence ;
{
	offset_t	oo = (offset_t) o64 ;

	return bseek(fp,oo,whence) ;
}
/* end subroutine (bseek64) */



