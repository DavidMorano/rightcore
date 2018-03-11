/* passfd */

/* subroutine to pass a file-descriptor to a file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1999-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	= Pass a file-descritpr to a file in the file-system.

        This subroutine passes the given file-descriptor to a file (specified
	by its file-path).

	Synopsis:

	int passfd(cchar *fname,int fd)

	Arguments:

	fname		path to UNIX® file (pipe-FIFO) to pass FD to
	fd		file-descriptor to pass

	Returns:

	<0		error in dialing
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int passfd(cchar *fname,int fd)
{
	const int	of = (O_WRONLY) ;
	int		rs ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;
	if (fd < 0) return SR_BADFD ;

#if	CF_DEBUGS
	debugprintf("passfd: fname=%s\n",fname) ;
#endif

	if ((rs = uc_open(fname,of,0666)) >= 0) {
	    const int	pfd = rs ;
	    rs = uc_fpassfd(pfd,fd) ;
	    u_close(pfd) ;
	}

	return rs ;
}
/* end subroutine (passfd) */


