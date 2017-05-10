/* process */

/* process a lock file */


#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	CF_MAPFILE	0		/* try to map the file */
#define	CF_FILESIZE	0		/* lock using file size */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from other programs that do similar
	things.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine tests to see if a lock is present on a file or not.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int process(pip,ofp,lockfname,timeout,to_remove)
struct proginfo	*pip ;
bfile		*ofp ;
const char	lockfname[] ;
int		timeout, to_remove ;
{
	struct ustat	sb ;
	time_t		daytime ;
	int		rs ;
	int		i, fd ;
	int		lockcmd ;
	int		f_captured = FALSE ;
	cchar		*cp ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (lockfname == NULL) return SR_FAULT ;

	if (lockfname[0] == '\0') return SR_NOENT ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: lockfname=%s\n",
	        lockfname) ;
#endif

/* open the file */

	rs = u_open(lockfname,O_RDWR,0664) ;
	fd = rs ;
	if (rs >= 0) {

/* try to capture the file */

	    lockcmd = (pip->f.readlock) ? F_RLOCK : F_WLOCK ;

	    rs = u_fstat(fd,&sb) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: u_fstat() rs=%d\n",rs) ;
#endif

#if	CF_FILESIZE
	    rs = lockfile(fd,lockcmd,0L,(int) sb.st_size,timeout) ;
#else
	    rs = lockfile(fd,lockcmd,0L,0,timeout) ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: lockfile() rs=%d\n",rs) ;
#endif

	    f_captured = (rs >= 0) ;

	if (f_captured) {

	    rs = SR_OK ;
	    cp = "captured lock file\n" ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s",
	            pip->progname,cp) ;

	    if (pip->verboselevel > 0) {

	        bprintf(ofp,cp) ;

	    }

	} /* end if (bad or good) */

	    if (pip->holdtime > 0)
	        sleep(pip->holdtime) ;

#if	CF_MAPFILE
	    if (f_captured) {

	        int	prot, flags ;
	        int	mapsize = getpagesize() ;

	        char	*mapfile ;


	        prot = PROT_READ | PROT_WRITE ;
	        flags = MAP_SHARED ;
	        rs = u_mmap(((caddr_t) 0),(size_t) mapsize,prot,flags,
	            fd,0L,(caddr_t *) &mapfile) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: u_mmap() rs=%d\n",rs) ;
#endif

	    }
#endif /* CF_MAPFILE */

	    u_close(fd) ;

	} /* end if (opened the file) */

	if ((rs < 0) || (! f_captured)) {

	    if (pip->debuglevel > 0) {

	        bprintf(pip->efp,"%s: failed to capture lock (%d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: lkfile=%s\n",
	            pip->progname,lockfname) ;

	    }

	    logfile_printf(&pip->lh,"could not capture lock (%d)\n",
	        rs) ;

	    logfile_printf(&pip->lh,"lkfile=%s\n",
	        lockfname) ;

	    if (pip->verboselevel > 0)
	        bprintf(ofp,"failed to capture lock (%d)\n",rs) ;

	    rs = SR_WOULDBLOCK ;

	} 

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_captured : rs ;
}
/* end subroutine (process) */


