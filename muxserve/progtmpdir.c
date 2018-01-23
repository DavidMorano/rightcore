/* progjobdir */

/* make the program-specific job directory */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

	= 2018-01-20, David A­D­ Morano
	I changed the name of this subroutine (and the file it is in) from
	|progtmpdir()| to |progjobdir()|.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks/makes the program-specific job directory.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	IPCDIRMODE
#define	IPCDIRMODE	0777
#endif


/* external subroutines */

extern int	mkdirs(cchar *,mode_t) ;
extern int	getgid_group(cchar *,int) ;
extern int	isNotPresent(int) ;


/* forward references */

static int	procdircheck(PROGINFO *,const char *,mode_t) ;
static int	procdirmk(PROGINFO *,cchar *,mode_t) ;
static int	procdirgroup(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int progjobdir(PROGINFO *pip,char *jobdname)
{
	int		rs ;
	int		len = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progjobdir: ent\n") ;
#endif

	if (jobdname == NULL) return SR_FAULT ;

/* this first one should be open permissions! */

	if (pip->jobdname == NULL) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(tbuf,pip->tmpdname,pip->rootname)) >= 0) {
	        const mode_t	dm = (IPCDIRMODE | S_ISVTX) ;
	        if ((rs = procdircheck(pip,tbuf,dm)) >= 0) {
		    cchar	*sn = pip->searchname ;
		    if ((rs = mkpath2(jobdname,tbuf,sn)) >= 0) {
		        len = rs ;
	    	        if ((rs = procdircheck(pip,jobdname,dm)) >= 0) {
			    cchar	**vpp = &pip->jobdname ;
			    rs = proginfo_setentry(pip,vpp,jobdname,len) ;
			}
		    }
	        }
	    }
	} else {
	    rs = mkpath1(jobdname,pip->jobdname) ;
	    len = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progjobdir: ret rs=%d len=%d\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progjobdir) */


/* local subroutines */


/* check if a directory exists and has the correct permissions */
static int procdircheck(PROGINFO *pip,cchar *dname,mode_t dm)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(dname,&sb)) >= 0) {
	    if ((sb.st_mode & dm) != dm) {
	        rs = procdirmk(pip,dname,dm) ;
	    }
	} else if (rs == SR_NOENT) {
	    rs = procdirmk(pip,dname,dm) ;
	}

	return rs ;
}
/* end subroutine (procdircheck) */


static int procdirmk(PROGINFO *pip,cchar *dname,mode_t dm)
{
	int		rs ;
	if ((rs = mkdirs(dname,dm)) >= 0) {
	    if ((rs = u_chmod(dname,dm)) >= 0) {
	        if ((rs = procdirgroup(pip)) >= 0) {
		    if (pip->gid_rootname > 0) {
		        u_chown(dname,-1,pip->gid_rootname) ;
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (procdirmk) */


static int procdirgroup(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->gid_rootname < 0) {
	    if (pip->rootname != NULL) {
		if ((rs = getgid_group(pip->rootname,-1)) >= 0) {
	            pip->gid_rootname = rs ;
		} else if (rs == SR_NOTFOUND) {
	            pip->gid_rootname = 0 ;
		    rs = SR_OK ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (procdirgroup) */


