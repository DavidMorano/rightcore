/* bwritefile */

/* copy a file to another file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine copies the contents of a given file to the current file.

	Synospsis:

	int bwritefile(bfile *ofp,cchar *fname)

	Arguments:

	ofp		output file pointer to copy to
	fname		the file (which will be read) to copy to the current

	Returns:

	>=0		length of data copied
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


int bwritefile(bfile *ofp,cchar *fname)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		tlen = 0 ;

#if	CF_DEBUGS
	debugprintf("bwritefile: ent fname=%s\n",fname) ;
#endif

	if (ofp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (ofp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (! ofp->f.nullfile) {
	    bfile		ifile, *ifp = &ifile ;
	    const mode_t	om = 0665 ;
	    if ((rs = bopen(ifp,fname,"r",om)) >= 0) {
		{
		    rs = bwriteblock(ofp,ifp,-1) ;
		    tlen = rs ;
		}
		rs1 = bclose(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (bfile) */
	} /* end if (not nullfile) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (bwritefile) */


