/* checklink */

/* check a directory entry for a dangling link */


#define	CF_DEBUG 	0


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Synopsis:

	int checklink(name,sbp,pip)
	char		name[] ;
	struct ustat	*sbp ;
	struct proginfo	*pip ;

	Arguments:

	name		directory entry
	sbp		'stat' block pointer

	Returns:

	>0		skip this directory entry in directory walk
	0		continue with directory walk as usual
	<0		exit directory walk altogether


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<dirent.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif


/* external subroutines */


/* external variables */


/* exported subroutines */


int checklink(name,sbp,pip)
char		name[] ;
struct ustat	*sbp ;
struct proginfo	*pip ;
{
	struct ustat	sb2 ;

	int	rs1 ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checklink: entered %s\n",name) ;
#endif

#ifdef	COMMENT
	if (! S_ISLNK(sbp->st_mode))
	    return 0 ;
#endif /* COMMENT */

	if ((rs1 = u_stat(name,&sb2)) >= 0)
	    return 0 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checklink: stat rs=%d\n",
	        rs1) ;
#endif

	if (pip->verboselevel > 0)
	    bprintf(pip->ofp,"badlink=%s (%d)\n",
	        name,rs1) ;

	if ((rs1 != SR_NOENT) && (rs1 != SR_ACCESS)) {

	    if (pip->verboselevel > 0)
	        bprintf(pip->ofp,"file not removed (%d)\n", rs1) ;

	    return 1 ;
	}

	if ((pip->debuglevel <= 1) && (! pip->f.no)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("checklink: unlinked=%s\n",
	            name) ;
	    }
#endif

	    rs1 = u_unlink(name) ;
	if (rs1 >= 0)
		pip->c_removed += 1 ;

	} /* end if (removed the file) */

	if (pip->f.print)
	    bprintf(pip->ofp,"%s\n", name) ;

	return 1 ;
}
/* end subroutine (checklink) */



