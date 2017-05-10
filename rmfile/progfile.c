/* progfile */

/* process a file (to be deleted) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-05-01, David A­D­ Morano

	This program was originally written.


	= 1996-06-01, David A­D­ Morano

	This program was enhanced to have a "verbose" mode where the
	user can see what is going on.	Also, the program has been
	made recursive so that bad directories can be removed also
	(with their contents).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ rmfile [-d[=delay]] [-?DVdn] file | directory

	Arguments:
	-?		print brief help
	-D		debug mode
	-V		print version and exit
	-n		no-delete mode
	-r		recursive
	-f		take the next argument as a file or directory to
			be deleted
	-d		request a nominal delay or a specific delay


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	removes(const char *) ;
extern int	unlinkd(const char *,int) ;


/* external variables */


/* global variables */


/* forward references */


/* local variables */


/* exported subroutines */


int progfile(pip,delay,fname)
struct proginfo	*pip ;
int		delay ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile: delay=%d fname=%s\n",delay,fname) ;
#endif

	if (u_stat(fname,&sb) < 0) goto ret0 ;

	if (S_ISDIR(sb.st_mode)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: directory\n") ;
#endif

	    if (! pip->f.nodelete) {
	        if (delay > 0) {
	            rs = unlinkd(fname,delay) ;
	        } else {
		    if (pip->f.recursive) {
	                rs = removes(fname) ;
		    } else
	                rs = u_rmdir(fname) ;
		}
	    } /* end if */

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: regular file\n") ;
#endif

	    if (delay > 0) {
	        rs = unlinkd(fname,delay) ;
	    } else {
	        rs = u_unlink(fname) ;
	    } /* end if */

	} /* end if */

ret0:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: file=%s (%d)\n",
	        pip->progname,fname,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progfile) */



