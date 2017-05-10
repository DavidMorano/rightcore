/* process */

/* process a log table file */


#define	CF_DEBUG	1		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

        The subroutine was adapted from other programs that do similar things.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine handles the processing for one lockfile.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

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


/* global variables */


/* local forward references */


/* local variables */


/* exported subroutines */


int process(pip,ofp,lockfname,timeout,to_remove)
struct proginfo	*pip ;
bfile		*ofp ;
char		lockfname[] ;
int		timeout, to_remove ;
{
	struct ustat	sb ;
	int		rs ;
	cchar		*cp ;
	char		timebuf[TIMEBUFLEN + 1] ;
	char		timebuf1[TIMEBUFLEN + 1] ;
	char		timebuf2[TIMEBUFLEN + 1] ;

	if ((lockfname == NULL) || (lockfname[0] == '\0'))
	    return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: lockfname=\"%s\"\n",
	        lockfname) ;
#endif


/* check if the lock file is too old */

	if (u_stat(lockfname,&sb) >= 0) {

	    if (! S_ISDIR(sb.st_mode)) {

	        if ((to_remove > 0) && 
	            ((pip->daytime - sb.st_mtime) >= to_remove)) {

	            u_unlink(lockfname) ;

	            logfile_printf(&pip->lh,
	                "%s removed old lock mtime=%s\n",
	                timestr_log(pip->daytime,timebuf1),
	                timestr_log(sb.st_mtime,timebuf2)) ;

	            if (pip->debuglevel > 0) {

	                bprintf(pip->efp,
	                    "%s: removed old lock\n",
	                    pip->progname) ;

	                bprintf(pip->efp,"%s: lkfile=%s mtime=%s\n",
	                    pip->progname,
	                    lockfname,
	                    timestr_log(sb.st_mtime,timebuf2)) ;

	            }

	        } /* end if (possible remove of lock) */

	    } else {

	        logfile_printf(&pip->lh,"item is not a file\n") ;

	        goto badlock ;

	    }

	} /* end if (stat for possible removal) */


/* try to capture the file */

	rs = SR_OK ;
	while (timeout > 0) {

	    rs = u_open(lockfname,O_RDWR | O_CREAT | O_EXCL,0664) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("process: opened w/ (rs %d)\n", rs) ;
#endif

	    if (rs >= 0)
	        break ;

	    sleep(1) ;

	    timeout -= 1 ;

	} /* end while */

	u_close(rs) ;

	if (rs < 0) {

	    u_time(&pip->daytime) ;

	    if (pip->debuglevel > 0) {

	        bprintf(pip->efp,
	            "%s: failed to capture lock (rs %d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: lkfile=%s mtime=%s\n",
	            pip->progname,
	            timestr_log(sb.st_mtime,timebuf)) ;

	    }

	    logfile_printf(&pip->lh,
	        "%s could not capture lock (rs %d)\n",
	        timestr_log(pip->daytime,timebuf),
	        rs) ;

	    logfile_printf(&pip->lh,"lkfile=%s mtime=%s\n",
	        lockfname,
	        timestr_log(sb.st_mtime,timebuf)) ;

	    if (pip->verboselevel > 0)
	        bprintf(ofp, "failed to capture lock (rs %d)\n",rs) ;

	    rs = SR_WOULDBLOCK ;

	} else {

	    cp = "captured lock file\n" ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s",
	            pip->progname,cp) ;

	    if (pip->verboselevel > 0)
	        bprintf(ofp,cp) ;

	    rs = SR_OK ;

	} /* end if (bad or good) */


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: exiting rs=%d\n",rs) ;
#endif

	return rs ;

badlock:
	rs = SR_EXIST ;
	return rs ;
}
/* end subroutine (process) */


