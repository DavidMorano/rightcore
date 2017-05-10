/* uc_forkdetached */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forks the current process but it does so with the CHILD
        signal ignored first.

        Note: This really accomplishes pretty much nothing, and therefore is not
        even compiled into anything (libraries included).

	Synopsis:

	int uc_forkdetached()

	Arguments:

	pid		PID of process to retrieve UID for

	Returns:

	>=0		UID of process (as an integer)
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* local defines */


/* external subroutines */


/* local variables */


/* forward references */


/* exported subroutines */


int uc_forkdetached()
{
	struct sigaction	osh, nsh ;

	sigset_t	sm ;

	pid_t		pid ;

	int	rs ;


	uc_sigemptyset(&sm) ;

	memset(&nsh,0,sizeof(struct sigaction)) ;
	nsh.sa_handler = SIG_IGN ;
	nsh.sa_mask = sm ;
	nsh.sa_flags = (SA_NOCLDWAIT | SA_NOCLDSTOP) ;

	if ((rs = u_sigaction(SIGCLD,&nsh,&osh)) >= 0) {

	    rs = uc_fork() ;
	    pid = rs ;

	    u_sigaction(SIGCLD,&osh,NULL) ;
	} /* end if (sigaction) */

	return (rs >= 0) ? ((int) pid) : rs ;
}
/* end subroutine (uc_forkdetached) */


