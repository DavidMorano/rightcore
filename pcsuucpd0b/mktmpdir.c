/* mktmpdir */

/* make the program-specific temporary directory */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1991-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

        This subroutine checks/makes the program-specific temporary directory.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	IPCDIRMODE	0777

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;


/* forward references */

static int	checkdir(const char *,int) ;


/* exported subroutines */


/* make the private TMP area */
int mktmpdir(pip,jobdname)
struct proginfo	*pip ;
char		jobdname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	cmode = (IPCDIRMODE | S_ISVTX) ;
	int	len = 0 ;

	char	buf[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mktmpdir: ent\n") ;
#endif

	if (jobdname == NULL)
	    jobdname = buf ;

/* this first one should be open permissions! */

	mkpath2(tmpfname,pip->tmpdname,pip->rootname) ;

	rs = checkdir(tmpfname,cmode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mktmpdir: checkdir() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* this second one can be open (permissions-wise) or not */

	len = mkpath2(jobdname,tmpfname,pip->searchname) ;

#if	defined(P_FINGERS) && (P_FINGERS == 1)
	rs = checkdir(jobdname,cmode) ;
#else
	rs = u_stat(jobdname,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mktmpdir: checkdir() or u_stat() rs=%d\n",rs) ;
#endif

	if ((rs < 0) || ((sb.st_mode & S_ISVTX) != S_ISVTX)) {

	    if (rs < 0) {

	        rs = u_mkdir(jobdname,cmode) ;

	        if (rs >= 0)
	            u_stat(jobdname,&sb) ;

	    }

	    if (rs >= 0)
	        u_chmod(jobdname,(sb.st_mode | S_ISVTX)) ;

	} /* end if */

#endif /* defined(P_FINGERS) && (P_FINGERS == 1) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mktmpdir: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mktmpdir) */


/* local subroutines */


/* check if a directory exists and has the correct permissions */
static int checkdir(dname,mode)
const char	dname[] ;
int		mode ;
{
	struct ustat	sb ;

	int	rs ;


	rs = u_stat(dname,&sb) ;

	if ((rs < 0) || ((sb.st_mode & mode) != mode)) {

	    if (rs < 0)
	        rs = u_mkdir(dname,mode) ;

	    u_chmod(dname,mode) ;

	} /* end if */

	return rs ;
}
/* end subroutine (checkdir) */



