/* uc_lockf */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revisions history:

	= 1998-01-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int uc_lockf(int fd,int cmd,offset_t size)

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

	size		size of region to lock in the file (0="whole file")

	Returns:

	<0		error
	>=0		OK


	NOTE:

        Note that the stupid developers of the File Locking mechanism un UNIX®
        System V did not distinguish a real deadlock from a temporary lack of
        system resources. We attempt to make up for this screw ball bug in UNIX®
        with our retries on DEADLOCK.

        Also, watch out for stupid Solaris bugs. They do not want a remote file
        to be memory mapped and locked at the same time. They think that they
        are protecting us against ourselves but in reality they are just inept
        developers who are making our lives more miserable with Solaris than it
        alreay is!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<upt.h>
#include	<localmisc.h>


/* local defines */

#define	TO_DEADLOCK	50		/* time-out in seconds */

#define	NDF		"/home/ncmp/lockf.deb"


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif


/* exported subroutines */


int uc_lockf(int fd,int cmd,offset_t size)
{
	int		rs ;
	int		to = (TO_DEADLOCK*1000) ;
	int		f_exit = FALSE ;

#if	CF_DEBUGN
	const int	pid = getpid() ;
	const pthread_t	tid = uptself(NULL) ;
#endif

	if (fd < 0) return SR_NOTOPEN ;
	if ((cmd < 0) || (cmd > F_TEST)) return SR_INVALID ;

#if	CF_DEBUGN
	    nprintf(NDF,"uc_lockf: ent pid=%u:%u cmd=%u size=%lld\n",
		pid,tid,cmd,size) ;
#endif

	repeat {
	    errno = 0 ;
	    if ((rs = lockf(fd,cmd,size)) < 0) rs = (- errno) ;
	    if (rs < 0) {
#if	CF_DEBUGN
		{
		nprintf(NDF,"uc_lockf: err pid=%u:%u rs=%d\n",pid,tid,rs) ;
		}
#endif
	        switch (rs) {
	        case SR_DEADLK:
	            if (to-- > 0) {
	                msleep(1) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

/* handle old mess-ups */

#if	CF_DEBUGS
	debugprintf("uc_lockf: fin rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"uc_lockf: fin pid=%u:%u rs=%d\n",pid,tid,rs) ;
#endif

	if (rs == SR_ACCES) rs = SR_AGAIN ;

#if	CF_DEBUGS
	debugprintf("uc_lockf: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_lockf) */


