/* lockfile(3dam) */

/* UNIX® System V file-record locking */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-17, David A­D­ Morano
        This subroutine was written as a generalization of the locking found in
        many previous programs. Why don't the OS suppliers write this subroutine
        in addition to 'lockf(3c)'?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to something like 'lockf(2)' but has
	different arguments for different options.  A timeout can be supplied
	with this subroutine.  A negative timeout is taken to be a reasonable
	default but may eventually timeout none-the-less.  Read locks can also
	be manipulated with this routine unlike with 'lockf(3c)'.  Like with
	'lockf(3c)' all lock regions are relative to the current file position
	if the starting offset is given as negative (anything).

	If the 'start' argument is negative, then the function behaves
	essentially the same as 'lockf(3c)' and all locks are relative with
	respect to the current file offset position.  Also, a zero-sized lock
	is equivalent to locking from the specified starting position (possibly
	the current position) to (and beyond) the end of the file.

	A negative 'timeout' is equivalent to a default timeout.  A 'timeout'
	of zero is zero!

	Synopsis:

	int lockfile(fd,cmd,start,size,timeout)
	int		fd ;
	int		cmd ;
	offset_t	start ;
	offset_t	size ;
	int		timeout ;

	Arguments:

	fd	file descriptor of file to lock
	cmd	command, one of:

			F_ULOCK

			F_WLOCK (F_LOCK)
			F_TWLOCK (F_TLOCK)
			F_TWTEST (F_TEST)

			F_RLOCK
			F_TRLOCK
			F_RTEST

	start		starting offset of region to lock (or test)
	size		size of region to lock in the file (0="whole file")
	timeout		timeout in seconds to wait for the lock

	Returns:

	>=0	succeeded
	<0	failed w/ system error code returned

	Notes:

	If the starting offset is given as negative then the region to be
	locked is taken as relative to the current file position.

	Some Solaris bugs to be aware of:

	If a file is locked to the exact size of the file and then a memory map
	of the whole file is attempted (at the size of the file), then memory
	mapping fails with an ERRNO of EAGAIN.  This bug does NOT show up when
	files are locked in their entirety and beyond the end of them.  So
	there is an inconsistency in the way the kernel handles the
	interactions of file locking and file mapping.  Who can figure?  Of
	course, I (and others) just wish that the system would allow both file
	locking AND memory mapping simultaneously without vomitting up all over
	the place!  We understand that file-locking does not "prevent" reading
	memory (from a process that has the same file memory-mapped), but
	neither does file-locking prevent another process from reading the same
	file using 'read(2)'!!   Do you get it solaris boys?


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>


/* local defines */


/* local structures */


/* forward references */

#if	CF_DEBUGS
static int	debugprintstat(const char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int lockfile(int fd,int cmd,offset_t start,offset_t size,int to)
{
	return uc_lockfile(fd,cmd,start,size,to) ;
}
/* end subroutine (lockfile) */


