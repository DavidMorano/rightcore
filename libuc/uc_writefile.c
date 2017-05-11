/* uc_writefile */

/* interface component for UNIX® library-3c */
/* copy from one file descriptor to another */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-01-10, David A­D­ Morano
	The subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine copies data from the specified file to the output file
        descriptor.

	Synopsis:

	int uc_writefile(int ofd,cchar *fname)

	Arguments:

	ofd		source file descriptor
	fname		file to write to output file-descriptor

	Returns:

	<0		error
	>=0		length copied


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* exported subroutines */


int uc_writefile(int ofd,cchar *fname)
{
	const mode_t	om = 0666 ;
	const int	of = O_RDONLY ;
	int		rs ;
	int		len = 0 ;
	if (fname == NULL) return SR_FAULT ;
	if ((rs = uc_open(fname,of,om)) >= 0) {
	    const int	ifd = rs ;
	    {
		rs = uc_writedesc(ofd,ifd,-1) ;
		len = rs ;
 	    }
	    u_close(ifd) ;
	} /* end if (open-file) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_writefile) */


