/* usystem */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	= 2008-09-03, David A­D­ Morano

        Is this garbage this needed someplace? How about we try an experiment
        and delete this stuff and see if anyone misses it!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>





int usystem(s)
char *s;
{
	int	status, pid, w;

	void	(*istat)(), (*qstat)();


	if ((pid = fork()) == 0) {

	setuid(getuid());

	setgid(getgid());

		execl("/bin/sh", "sh", "-c", s, 0);

		_exit(127);
	}

	istat = signal(SIGINT, SIG_IGN);

	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid && w != -1) ;

	if (w == -1) status = -1 ;

	signal(SIGINT, istat);

	signal(SIGQUIT, qstat);

	return status ;
}
/* end subroutine (usystem) */


