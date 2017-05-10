/* uc_openpt */

/* interface component for UNIX® library-3c */
/* open a PTS-style pseudo-terminal */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was adapted from a previously related subroutine.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine opens a PTS-type pseudo-terminal for use on System V
        Release 3 (SVR3) UNIX® OS type systems.

	Synopsis:

	int uc_openpt(oflags)
	int		oflags ;

	Arguments:

	oflags		open-flags

	Returns:

	<0		errror
	>=0		file-descriptor


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mkdev.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/ptms.h>
#include <unistd.h>
#include <fcntl.h>

#include	<vsystem.h>


/* local defines */

#ifndef	PTMXFNAME
#define	PTMXFNAME	"/dev/ptmx"
#endif


/* forward references */


/* exported subroutines */


int uc_openpt(int oflags)
{
	int		rs ;

	oflags &= (~ O_ACCMODE) ;
	oflags |= O_RDWR ;

#if	defined(SYSHAS_OPENPT) && (SYSHAS_OPENPT > 0)

	errno = 0 ;
	if ((rs = posix_openpt(oflags)) < 0) rs = (- errno) ;

#elif	defined(SYSHAS_PTMX) && (SYSHAS_PTMX > 0)

	rs = u_open(PTMXFNAME,oflags,0662) ;

#else

	rs = SR_NOSYS ;

#endif /* SYSHAS_OPENPT */

	return rs ;
}
/* end subroutine (uc_openpt) */


