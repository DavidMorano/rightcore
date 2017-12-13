/* mktmpfile */

/* make a temporary file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Make a temporary file.

	Synopsis:

	int mktmpfile(rbuf,mode,template)
	char		rbuf[] ;
	mode_t		mode ;
	const char	template[] ;

	Arguments:

	- output buffer to hold resultant file name
	- file creation mode
	- input file name template string

	Returns:

	>=0	success and length of created file name
	<0	failure w/ error number


	Notes:
	Q. What is with |uc_forklock(3uc)|?
	A. We try to minimize child processes getting an extra (unknown to it)
	   file-descriptor.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	opentmpfile(const char *,int,mode_t,char *) ;


/* external variables */


/* forward reference */


/* local variables */


/* exported subroutines */


int mktmpfile(char *rbuf,mode_t om,cchar *inname)
{
	const int	of = (O_WRONLY|O_CLOEXEC) ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = uc_forklockbegin(-1)) >= 0) {
	    if ((rs = opentmpfile(inname,of,om,rbuf)) >= 0) {
	        u_close(rs) ;
	        len = strlen(rbuf) ;
	    } /* end if (opentmpfile) */
	    rs1 = uc_forklockend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (uc_forklock) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mktmpfile) */


