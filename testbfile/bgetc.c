/* bgetc */

/* "Basic I/O" package similiar to "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutine to read a single character (within the BFILE facility.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* exported subroutines */


int bgetc(fp)
bfile	*fp ;
{
	int	rs ;
	int	ch ;

	char	buf[2] ;


	rs = bread(fp,buf,1) ;
	if (rs == 0) rs = SR_EOF ;

	ch = (buf[0] & 0xff) ;
	return (rs > 0) ? ch : rs ;
}
/* end subroutine (bgetc) */



