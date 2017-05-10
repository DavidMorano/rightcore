/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Jim Kutsch                                      *
 *									*
 ***********************************************************************/
/* usystem same as system except runs with real user and group id
* rather than effective uid and gid
* J. A. Kutsch   Auguest 1981  */
#include <signal.h>

usystem(s)
char *s;
{
	int status, pid, w;
	register void (*istat)(), (*qstat)();

	if((pid = fork()) == 0) {
	setuid(getuid());
	setgid(getgid());
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != -1)
		;
	if(w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}
