/* uc_opensys */

/* interface component for UNIX® library-3c */
/* open some system related resource */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine attempts to open a special system-wide resource of some
	kind.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<opensysfs.h>
#include	<ipasswd.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	opensys_banner(const char *,int,int) ;

extern char	*strnrchr(const char *,int,int) ;


/* local structures */


/* forward references */

static int isRealName(const char *,int) ;


/* local variables */

static const char	*sysnames[] = {
	"userhomes",
	"usernames",
	"groupnames",
	"projectnames",
	"users",
	"groups",
	"projects",
	"passwd",
	"group",
	"project",
	"realname",
	OPENSYSFS_FSHELLS,
	OPENSYSFS_FSHADOW,
	"userattr",
	"banner",
	"bandate",
	NULL
} ;

enum sysnames {
	sysname_userhomes,
	sysname_usernames,
	sysname_groupnames,
	sysname_projectnames,
	sysname_users,
	sysname_groups,
	sysname_projects,
	sysname_passwd,
	sysname_group,
	sysname_project,
	sysname_realname,
	sysname_shells,
	sysname_shadow,
	sysname_userattr,
	sysname_banner,
	sysname_bandate,
	sysname_overlast
} ;

static const int	whiches[] = {
	OPENSYSFS_WUSERHOMES,
	OPENSYSFS_WUSERNAMES,
	OPENSYSFS_WGROUPNAMES,
	OPENSYSFS_WPROJECTNAMES,
	OPENSYSFS_WUSERNAMES,
	OPENSYSFS_WGROUPNAMES,
	OPENSYSFS_WPROJECTNAMES,
	OPENSYSFS_WPASSWD,
	OPENSYSFS_WGROUP,
	OPENSYSFS_WPROJECT,
	OPENSYSFS_WREALNAME,
	OPENSYSFS_WSHELLS,
	OPENSYSFS_WSHADOW,
	OPENSYSFS_WUSERATTR,
	-1
} ;


/* exported subroutines */


/* ARGSUSED */
int uc_opensys(cchar *fname,int of,mode_t om,cchar **envv,int to,int opts)
{
	int		rs = SR_OK ;
	int		fl = -1 ;
	int		fi ;
	const char	*tp ;

#if	CF_DEBUGS
	debugprintf("uc_opensys: fname=%s\n",fname) ;
#endif

/* take off any leading slashes */

	while (fname[0] == '/') fname += 1 ;

/* take off the '/sys' componenent */

	if ((tp = strpbrk(fname,"/­")) != NULL) {
	    fl = (tp-fname) ;
	}

	{
	    int	len ;
	    if ((len = isRealName(fname,fl)) > 0) {
		fl = len ;
	    }
	}

	if ((fi = matstr(sysnames,fname,fl)) >= 0) {
	    switch (fi) {
	    case sysname_userhomes:
	    case sysname_usernames:
	    case sysname_groupnames:
	    case sysname_projectnames:
	    case sysname_users:
	    case sysname_groups:
	    case sysname_projects:
	    case sysname_passwd:
	    case sysname_group:
	    case sysname_project:
	    case sysname_realname:
	    case sysname_shells:
	    case sysname_shadow:
	    case sysname_userattr:
		{
	            int	w = whiches[fi] ;
	            rs = opensysfs(w,of,-1) ;
		}
		break ;
	    case sysname_banner:
	    case sysname_bandate:
		rs = opensys_banner(fname,of,om) ;
		break ;
	    default:
		rs = SR_NOENT ;
		break ;
	    } /* end switch */
	} else
	    rs = SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("uc_opensys: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_opensys) */


/* local subroutines */


static int isRealName(const char *fname,int fl)
{
	int		len = 0 ;
	int		cl ;
	const char	*cp ;

	if ((cl = sfbasename(fname,fl,&cp)) > 0) {
	    cchar	*suf = IPASSWD_SUF ;
	    cchar	*tp ;
	    if ((tp = strnrchr(cp,cl,'.')) != NULL) {
		const int	w = sysname_realname ;
		const int	suflen = strlen(suf) ;
		if (strncmp((tp+1),suf,suflen) == 0) {
		    cchar	*rn = sysnames[w] ;
		    if (strwcmp(rn,cp,(tp-cp)) == 0) {
			len = (tp-fname) ;
		    }
		}
	    }
	}

	return len ;
}
/* end subroutine (isRealName) */


