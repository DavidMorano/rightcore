/* bcopyblock */

/* copy a file to another file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This subroutine copies the remainder of the input file
	to the output file.

	Synospsis:

	int bcopyblock(ifp,ofp,ulen)
	bfile		*ifp, *ofp ;
	int		ulen ;

	Arguments:

	ifp		input file pointer to copy from
	ofp		output file pointer to copy to
	ulen		length of supplied buffer

	Returns:

	>=0		length of data copied or error return
	<0		error


*************************************************************************/


#define	BFILE_MASTER	0


#undef	bcopy

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#undef	bcopy

#define	BFILE_NPAGES	4


/* external subroutines */


/* exported subroutines */


int bcopyblock(ifp,ofp,ulen)
bfile		*ifp, *ofp ;
int		ulen ;
{
	return bwriteblock(ofp,ifp,ulen) ;
}
/* end subroutine (bcopyblock) */



