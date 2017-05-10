/* brewind */

/* "Basic I/O" package similiar to "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We rewind the BFILE stream.

*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* externals within the library */


/* exported subroutines */


int brewind(fp)
bfile		*fp ;
{
	int	rs ;


	rs = bseek(fp,0L,SEEK_SET) ;

	return rs ;
}
/* end subroutine (brewind) */



