/* progtmpdir */

/* make the program-specific temporary directory */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks/makes the program-specific temporary directory.


*******************************************************************************/


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
#include	<grp.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	IPCDIRMODE
#define	IPCDIRMODE	0777
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getgid_group(cchar *,int) ;
extern int	isNotPresent(int) ;


/* forward references */

static int	procdircheck(PROGINFO *,const char *,mode_t) ;
static int	procdirgroup(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int progtmpdir(PROGINFO *pip,char *jobdname)
{
	const mode_t	cmode = (IPCDIRMODE | S_ISVTX) ;
	int		rs ;
	int		len = 0 ;
	char		buf[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progtmpdir: ent\n") ;
#endif

	if (jobdname == NULL)
	    jobdname = buf ;

/* this first one should be open permissions! */

	rs = mkpath2(tmpfname,pip->tmpdname,pip->rootname) ;
	if (rs >= 0)
	    rs = procdircheck(pip,tmpfname,cmode) ;

	if (rs < 0)
	    goto ret0 ;

/* this second one can be open (permissions-wise) or not */

	rs = mkpath2(jobdname,tmpfname,pip->searchname) ;
	len = rs ;
	if (rs >= 0)
	    rs = procdircheck(pip,jobdname,cmode) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progtmpdir: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progtmpdir) */


/* local subroutines */


/* check if a directory exists and has the correct permissions */
static int procdircheck(PROGINFO *pip,cchar *dname,mode_t operms)
{
	struct ustat	sb ;
	int		rs ;

	rs = u_stat(dname,&sb) ;

	if ((rs < 0) || ((sb.st_mode & operms) != operms)) {

	    if (rs < 0)
	        rs = u_mkdir(dname,operms) ;

	    if (rs >= 0) {
	        u_chmod(dname,operms) ;
		procdirgroup(pip) ;
		if (pip->gid_rootname > 0)
		    u_chown(dname,-1,pip->gid_rootname) ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (procdircheck) */


static int procdirgroup(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->gid_rootname < 0) {
	    if (pip->rootname != NULL) {
		if ((rs = getgid_group(pip->rootname,-1)) >= 0) {
	            pip->gid_rootname = rs ;
		} else if (isNotPresent(rs)) {
	            pip->gid_rootname = 0 ;
		    rs = SR_OK ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (procdirgroup) */


