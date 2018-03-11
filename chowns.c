/* chowns */

/* change ownership on all components of a file path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A­D­ Morano
        This subroutine was originally written. This subroutines is standard on
        some UNIXes but not on others so it is now provided.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will try to change the wonership (UID and GID) on all of
        the directories in the specified directory path.

	Synopsis:

	int chowns(dir,uid,gid)
	const char	dir[] ;
	uid_t		uid ;
	gid_t		gid ;

	Arguments:

	- dir		direcrtory path to a new directory to create
	- uid
	- gid

	Returns:

	>=0		operation completed successfully
	<0		represents a system error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	CHECKINFO	struct checkinfo


/* external variables */

extern int	mkpath1(char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct checkinfo {
	uid_t		uid ;
	gid_t		gid ;
	mode_t		mode ; /* not used here */
} ;


/* forward references */

static int	checkdir(struct checkinfo *,const char *) ;


/* local variables */


/* exported subroutines */


int chowns(cchar *dname,uid_t uid,gid_t gid)
{
	CHECKINFO	ci ;
	int		rs ;
	char		dirbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("chowns: ent dir=%s\n",dir) ;
#endif

	memset(&ci,0,sizeof(struct checkinfo)) ;
	ci.uid = uid ;
	ci.gid = gid ;

	if ((rs = mkpath1(dirbuf,dname)) >= 0) {
	    cchar	*cp = dirbuf ;
	    char	*bp ;
	    while ((bp = strchr(cp,'/')) != NULL) {
	        *bp = '\0' ;
	        if (((bp - cp) > 0) && (strcmp(cp,".") != 0)) {
	            rs = checkdir(&ci,dirbuf) ;
	        } /* end if */
	        *bp = '/' ;
	        cp = bp + 1 ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (*cp != '\0')) {
	        rs = checkdir(&ci,dirbuf) ;
	    }
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (chowns) */


/* local subroutines */


static int checkdir(CHECKINFO *cip,cchar *dirbuf)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(dirbuf,&sb)) >= 0) {
	    int	f = FALSE ;
	    f = f || ((cip->uid >= 0) && (sb.st_uid != cip->uid)) ;
	    f = f || ((cip->gid >= 0) && (sb.st_gid != cip->gid)) ;
	    if (f) {
		rs = u_chown(dirbuf,cip->uid,cip->gid) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (checkdir) */


