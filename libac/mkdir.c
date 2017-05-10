/* mkdir */

/* make a directory */


/* revistion history:

	= 1987-07-10, David A­D­ Morano

	This was originally written.


*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a library version of the 'mkdir(1)' program.
	They ought to get this sort of subroutine as a system call
	one of these days.  This is outragious !

	Synopsis:

	int mkdir(dir,mode)
	const char	*dir ;
	mode_t		mode ;

	Arguments:

	dir		string of directory name
	mode		file permission bits (modified by UMASK)

	Returns:

	0	OK


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<localmisc.h>


/* exported subroutines */


int mkdir(dir,mode)
const char	*dir ;
mode_t		mode ;
{
	int		rs ;
	pid_t		pid ;
	const char	*dn = "/dev/null" ;

	int	child_stat ;

	if ((rs = fork()) == 0) {
	    u_close(1) ;		/* close standard output */
	    u_open(dn,O_WRONLY,0664) ;
	    u_close(2) ;		/* close standard error */
	    u_open(dn,O_WRONLY,0664) ;
	    execl("/bin/mkdir","mkdir",dir,0) ;
	    uc_exit(127) ;
	}

	if (rs > 0) rs = u_waitpid(pid,&child_stat,0) ;

	return rs ;
}
/* end subroutine (mkdir) */


