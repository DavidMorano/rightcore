/* chmods */

/* change permissions on all components of a file path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A­D­ Morano
        This subroutine was originally written. This subroutines is standard on
        some UNIXes but not on others so it is now provided.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will try to change the permissions mode on all of the
        directories in the specified directory path.

	Synopsis:

	int chmods(dname,mode)
	const char	dname[] ;
	mode_t		mode ;

	Arguments:

	- dname		direcrtory path to a new directory to create
	- mode		newly created directories are created with this

	Returns:

	>=0		operation completed successfully
	<0		represents a system error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	CHECKINFO	struct xcheckinfo


/* external variables */

extern int	mkpath1(char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct xcheckinfo {
	uid_t		uid ;
	mode_t		dm ;
} ;


/* forward references */

static int	checkinfo_start(CHECKINFO *,mode_t) ;
static int	checkinfo_dir(CHECKINFO *,cchar *) ;
static int	checkinfo_finish(CHECKINFO *) ;


/* local variables */


/* exported subroutines */


int chmods(cchar *dname,mode_t dm)
{
	CHECKINFO	ci ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("chmods: ent %s\n",dir) ;
#endif

	if ((rs = checkinfo_start(&ci,dm)) >= 0) {
	    char	dirbuf[MAXPATHLEN + 1] ;
	   if ((rs = mkpath1(dirbuf,dname)) >= 0) {
		char	*cp, *cp2 ;
	        cp = dirbuf ;
	        while ((cp2 = strchr(cp,'/')) != NULL) {
	            *cp2 = '\0' ;
	            if (((cp2 - cp) > 0) && (strcmp(cp,".") != 0)) {
	                rs = checkinfo_dir(&ci,dirbuf) ;
	            } /* end if */
	            *cp2 = '/' ;
	            cp = (cp2 + 1) ;
	            if (rs < 0) break ;
	        } /* end while */
	        if ((rs >= 0) && (*cp != '\0')) {
	            rs = checkinfo_dir(&ci,dirbuf) ;
	        }
	    } /* end if (mkpath) */
	    rs = checkinfo_finish(&ci) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (checkinfo) */

	return rs ;
}
/* end subroutine (chmods) */


/* local subroutines */


static int checkinfo_start(CHECKINFO *cip,mode_t dm)
{
	cip->dm = dm ;
	cip->uid = getuid() ;
	return SR_OK ;
}
/* end subroutine (checkinfo_start) */


static int checkinfo_dir(CHECKINFO *cip,cchar *dirbuf)
{
	struct ustat	sb ;
	int		rs ;

	if (((rs = u_stat(dirbuf,&sb)) >= 0) && (sb.st_uid == cip->uid)) {
	    if ((sb.st_mode & cip->dm) != cip->dm) {
		const mode_t	nm = (sb.st_mode | cip->dm) ;
	        rs = u_chmod(dirbuf,nm) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (checkinfo_dir) */


static int checkinfo_finish(CHECKINFO *cip)
{
	return SR_OK ;
}
/* end subroutine (checkinfo_finish) */


