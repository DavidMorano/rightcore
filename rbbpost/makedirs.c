/* makedirs */

/* make directories (within a single path) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a directory and all of its parent directories
	if they do not exist, and if it is possible to create them.

	Synopsis:

	int makedirs(dirp,dirl,mode)
	const char	dirp[] ;
	int		dirl ;
	mode_t		mode ;

	Arguments:

	dirp		path to the directory to create
	dirl		length of directory path string
	mode		the file permission mode to create the directories

	Returns:

	<0		system error
	0		directory already existed
	>0		directory was created


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* forward references */

static int	procdir(IDS *,const char *,mode_t) ;


/* exported subroutines */


int makedirs(cchar *dirp,int dirl,mode_t mode)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (dirp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("makedirs: ent dirl=%d dirp=%t\n",dirl,dirp,dirl) ;
#endif

	if ((rs = ids_load(&id)) >= 0) {
	    const char	*dp ;
	    char	dirbuf[MAXPATHLEN + 1] ;
	    char	*bp ;

	    if (dirl < 0) {

	        rs = procdir(&id,dirp,mode) ;
	        c += rs ;
	        if (rs == SR_NOENT)
	            dirl = strdcpy1w(dirbuf,MAXPATHLEN,dirp,-1) - dirbuf ;

	    } else {

	        strdcpy1w(dirbuf,MAXPATHLEN,dirp,dirl) ;
	        rs = procdir(&id,dirbuf,mode) ;
	        c += rs ;

	    } /* end if */

	    if (rs == SR_NOENT) {
	        rs = SR_OK ;

	        dp = dirbuf ;
	        while ((bp = strchr(dp,'/')) != NULL) {

	            *bp = '\0' ;
	            if (((bp - dp) > 0) && (strcmp(dp,".") != 0)) {
	                rs = procdir(&id,dirbuf,mode) ;
	                c += rs ;
	            } /* end if */

	            *bp = '/' ;
	            dp = bp + 1 ;

	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (*dp != '\0')) {
	            rs = procdir(&id,dirbuf,mode) ;
	            c += rs ;
	        }

	    } /* end if (needed some creations) */

	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

#if	CF_DEBUGS
	debugprintf("makedirs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (makedirs) */


/* local subroutines */


static int procdir(IDS *idp,cchar *dirbuf,mode_t mode)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(dirbuf,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        rs = sperm(idp,&sb,X_OK) ;
	        if (rs > 0) rs = 0 ;
	    } else {
	        rs = SR_NOTDIR ;
	    }
	} else if (rs == SR_NOENT) {
	    rs = u_mkdir(dirbuf,mode) ;
	    if (rs >= 0) rs = 1 ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("makedirs/procdir: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdir) */


