/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz                                    *
 *									*
 ***********************************************************************/
/* ssystem same as system except that it runs with real user and group id
* rather than effective uid and gid and it invokes the named program directly
* without going via a shell. So the command line that is given to this command
* must be directly be usable by the command that is invoked, i.e. no I/O
* redirection, etc.
* J. Mukerji 9/4/84 */
#include <signal.h>
#include <string.h>

ssystem(s)
char *s;
{
	int status, pid, w;
	register void (*istat)(), (*qstat)();
	char *argv[20];
	register int i;
	char *command;

	if((pid = fork()) == 0) {
		setuid(getuid());
		setgid(getgid());
		command = argv[0] = strtok(s," ");
		if( command == 0 ) 
		  command = argv[0] = "";
		i = 1;
		while( (argv[i++] = strtok(0," ")) != 0 );
		i=execvp( command, argv );
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
