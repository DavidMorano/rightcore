/* bcopyblock */

/* copy a part of a given file to the current file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies a part of a given file to the current file.

	Synospsis:

	int bcopyblock(ifp,ofp,ulen)
	bfile		*ofp ;
	bfile		*ifp ;
	int		ulen ;

	Arguments:

	ifp		input file pointer to copy from
	ofp		output file pointer to copy to
	ulen		length of supplied buffer

	Returns:

	>=0		length of data copied or error return
	<0		error


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* exported subroutines */


int bcopyblock(bfile *ifp,bfile *ofp,int ulen)
{
	return bwriteblock(ofp,ifp,ulen) ;
}
/* end subroutine (bcopyblock) */


