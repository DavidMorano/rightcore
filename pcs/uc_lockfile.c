/* uc_lockfile(3uc) */

/* UNIX® System V file-record locking */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-17, David A­D­ Morano
	This subroutine was written as a generalization of the locking found in
	many previous programs.  Why don't the OS suppliers write this
	subroutine in addition to 'lockf(3c)'?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is similar to something like 'lockf(2)' but has
	different arguments for different options.  A time-out can be supplied
	with this subroutine.  A negative time-out is taken to be a reasonable
        default but may eventually time-out none-the-less. Read locks can also
        be manipulated with this routine unlike with 'lockf(3c)'. 

	If the 'start' argument is negative, then the function behaves
	essentially the same as 'lockf(3c)' and all locks are relative with
	respect to the current file offset position.  Also, a zero-sized lock
	is equivalent to locking from the specified starting position (possibly
	the current position) to (and beyond) the end of the file.

	A negative 'time-out' is equivalent to a default time-out.  A 'time-out'
	of zero is zero!

	Synopsis:

	int uc_lockfile(fd,cmd,start,size,to)
	int		fd ;
	int		cmd ;
	offset_t	start ;
	offset_t	size ;
	int		to ;

	Arguments:

	fd	file descriptor of file to lock
	cmd	command, one of:

			F_ULOCK

			F_WTEST (F_TEST)
			F_WLOCK (F_LOCK)
			F_TWLOCK (F_TLOCK)

			F_RTEST
			F_RLOCK
			F_TRLOCK

	start		starting offset of region to lock (or test)
	size		size of region to lock in the file (0="whole file")
	to		time-out in seconds to wait for the lock

	Returns:

	>=0	succeeded
	<0	failed w/ system error code returned

	Notes:

        If the starting offset is given as negative then the region to be locked
        is taken as relative to the current file position.

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
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>


/* local defines */

#define	DEFTIMEOUT	1000

#ifndef	F_LOCK
#define	F_ULOCK	0
#define	F_LOCK	1
#define	F_TLOCK	2
#define	F_TEST	3
#endif


/* local structures */

#if	CF_DEBUGS
struct lockcmd {
	int	cmd ;
	char	*s ;
} ;
#endif


/* forward references */

#if	CF_DEBUGS
static int	debugprintstat(const char *,int) ;
#endif


/* local variables */

#if	CF_DEBUGS
static const struct lockcmd	cmdlu[] = {
	{ F_ULOCK, "unlock" },
	{ F_LOCK, "lock" },
	{ F_TLOCK, "tlock" },
	{ F_TEST, "test" },
	{ F_RLOCK, "rlock" },
	{ F_WLOCK, "wlock" },
	{ F_TRLOCK, "trlock" },
	{ F_TWLOCK, "twlock" },
	{ F_RTEST, "rtest" },
	{ F_WTEST, "wtest" },
	{ 0, NULL }
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int uc_lockfile(int fd,int cmd,offset_t start,offset_t size,int to)
{
	struct flock	fl ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("lockfile: fd=%d cmd=%d start=%lld size=%u to=%u\n",
	    fd,cmd,start,size,to) ;
	for (i = 0 ; cmdlu[i].s != NULL ; i += 1) {
	    if (cmd == cmdlu[i].cmd) break ;
	}
	if (cmdlu[i].s != NULL)
	    debugprintf("lockfile: cmd=%s(%d)\n",cmdlu[i].s,i) ;
	debugprintstat("lockfile: ",fd) ;
#endif /* CF_DEBUGS */

	memset(&fl,0,sizeof(struct flock)) ;
	fl.l_whence = (start >= 0) ? SEEK_SET : SEEK_CUR ;
	fl.l_start = (start >= 0) ? start : 0 ;
	fl.l_len = size ;

	if (to < 0)
	    to = DEFTIMEOUT ;

	switch (cmd) {
	case F_RTEST:
	    fl.l_type = F_RDLCK ;
	    if ((rs = u_fcntl(fd,F_GETLK,&fl)) >= 0) {
	        rs = SR_AGAIN ;
	        if (fl.l_type == F_UNLCK) rs = SR_OK ;
	    }
	    break ;
	case F_WTEST:
	    fl.l_type = F_WRLCK ;
	    if ((rs = u_fcntl(fd,F_GETLK,&fl)) >= 0) {
	        rs = SR_AGAIN ;
	        if (fl.l_type == F_UNLCK) rs = SR_OK ;
	    }
	    break ;
	case F_RLOCK:
	case F_WLOCK:
	case F_TRLOCK:
	case F_TWLOCK:
	    switch (cmd) {
	    case F_TRLOCK:
	        to = 0 ;
/* FALLTHROUGH */
	    case F_RLOCK:
	        fl.l_type = F_RDLCK ;
	        break ;
	    case F_TWLOCK:
	        to = 0 ;
/* FALLTHROUGH */
	    case F_WLOCK:
	        fl.l_type = F_WRLCK ;
/* FALLTHROUGH */
	    } /* end switch */
	    for (i = 0 ; i < (to + 1) ; i += 1) {
	        rs = u_fcntl(fd,F_SETLK,&fl) ;
#if	CF_DEBUGS
	        debugprintf("lockfile: u_fcntl() rs=%d fd=%d\n",rs,fd) ;
#endif
	        if ((rs != SR_AGAIN) && (rs != SR_ACCES)) break ;
	        if (i < to) uc_safesleep(1) ;
	    } /* end for */
	    break ;
	case F_ULOCK:
	    fl.l_type = F_UNLCK ;
	    rs = u_fcntl(fd,F_SETLK,&fl) ;
	    break ;
	default:
	    rs = SR_INVAL ;
	    break ;
	} /* end switch */

/* handle old UNIX compatibility problems! */

	if (rs == SR_ACCES)
	    rs = SR_AGAIN ;

#if	CF_DEBUGS
	debugprintf("lockfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_lockfile) */


/* local subroutines */


#if	CF_DEBUGS
static int debugprintstat(cchar *s,int fd)
{
	struct ustat	sb ;
	int		rs ;
	int		sl = 0 ;
	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    sl = debugprintf("%s fd=%d rs=%d size=%ld perm=%04o\n",
	        s,fd,rs,sb.st_size,sb.st_mode) ;
	}
	return (rs >= 0) ? sl : rs ;
}
#endif /* CF_DEBUGS */


