/* uc_moveup */

/* interface component for UNIX® library-3c */
/* dup an FD to above the reserved FD region, and CLOSE the original! */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-11, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine *moves* file descriptors up to some minimum number that
        is specified. This comes in most handy when forking child processes with
        pipes open and which need to be duplicated to some place above FD=2 (to
        a minumum FD=3).

	Synopsis:

	int uc_moveup(fd,minfd)
	int	fd ;
	int	minfd ;

	Arguments:

	fd		file descriptor to move up (possibly gets closed)
	minfd		minimum number where new FD will move to

	Returns:

	>=0		the new FD
	<0		error (original is *not* closed in this case)


	Implementation note:

        This subroutine needs to be able to execute after a 'fork(2)' but before
        any corresponding 'exit(2)', so we need it to be fork-safe (as much as
        possible). So we do not use 'malloc(3c)' for any memory allocation. The
        reason for this is because 'malloc(3c)' is *not* fork-safe on many
        (most) UNIX® platforms. One exception is that 'malloc(3c)' does appear
        to be fork-safe on the Solaris® platform since they (AT&T and Sun
        Microsystems) seemed to be much more concerned with thread and fork
        safety than most of the other UNIX®i® systems out there. But we cannot
        rely on always executing on the Solaris® system, so we need to
        accommodate poorer UNIX®i® cousins.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */

static int	movethem(int,int,int) ;


/* local variables */


/* exported subroutines */


int uc_moveup(int fd,int minfd)
{
	int		rs = SR_OK ;

	if (fd < 0)
	    return SR_INVALID ;

	if (fd < minfd) {
	    const int	nfds = getdtablesize() ;
	    if (minfd < nfds) {
	        rs = movethem(nfds,fd,minfd) ;
	        fd = rs ;
	    } else {
	        rs = SR_MFILE ;
	    }
	} /* end if (move needed) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (uc_moveup) */


/* local subroutines */


int movethem(int nfds,int fd,int minfd)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	int		j ;
	int		fds[nfds+1] ;	/* dynamic stack memory-allocation */

	while ((rs >= 0) && (fd < minfd) && (i < nfds)) {
	    fds[i++] = fd ;
	    rs = u_dup(fd) ;
	    fd = rs ;
	} /* end while */

/* close all the FDs that we made including the the original one given us */

	for (j = (((rs < 0) || (i == nfds)) ? 1 : 0) ; j < i ; j += 1) {
	    u_close(fds[j]) ;
	}

	if ((rs >= 0) && (i == nfds)) {
	    rs = SR_MFILE ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (movethem) */


