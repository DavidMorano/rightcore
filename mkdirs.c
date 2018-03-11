/* mkdirs */

/* make all directories in a directory path */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10, David A­D­ Morano
	This subroutine was originally written.  This subroutines (or something
	similar to it) is standard on some UNIXes but not on others, so it is
	now provided.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will create all of the directories in the specified
	directory path if they do not exist already.

	Synopsis:

	int mkdirs(dname,dm)
	const char	dname[] ;
	mode_t		dm ;

	Arguments:

	- dname		direcrtory path to a new directory to create
	- dm		newly created directories are created with this
			this permissions mode (subject to UMASK restrictions)

	Returns:

	>0		number of directories that were created
	==0		all directories existed
	<0		represents a system error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;


/* external variables */


/* forward references */

static int	procdir(IDS *,const char *,mode_t) ;


/* local variables */


/* external subroutines */


int mkdirs(cchar *dname,mode_t dm)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (dname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkdirs: ent dname=%s\n",dname) ;
#endif

	if ((rs = ids_load(&id)) >= 0) {

	    if ((rs = procdir(&id,dname,dm)) >= 0) {
	        c += rs ;
	    } else if (rs == SR_NOENT) {
		char	dirbuf[MAXPATHLEN + 1] ;
	        if ((rs = mkpath1(dirbuf,dname)) >= 0) {
		    const char	*dp = dirbuf ;
		    char	*bp ;

	            while ((bp = strchr(dp,'/')) != NULL) {

	                *bp = '\0' ;
	                if (((bp - dp) > 0) && (strcmp(dp,".") != 0)) {
	                    rs = procdir(&id,dirbuf,dm) ;
	                    c += rs ;
	                } /* end if */
	                *bp = '/' ;
	                dp = (bp + 1) ;

	                if (rs < 0) break ;
	            } /* end while */

	            if ((rs >= 0) && (*dp != '\0')) {
	                rs = procdir(&id,dirbuf,dm) ;
	                c += rs ;
	            } /* end if */

	        } /* end if (mkpath1) */
	    } /* end if (needed some creations) */

	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirs) */


/* local subroutines */


static int procdir(IDS *idp,cchar *dirbuf,mode_t dm)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = uc_stat(dirbuf,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        rs = sperm(idp,&sb,X_OK) ;
	        if (rs > 0) rs = 0 ;
	    } else {
	        rs = SR_NOTDIR ;
	    }
	} else if (rs == SR_NOENT) {
	    rs = uc_mkdir(dirbuf,dm) ;
	    if (rs >= 0) rs = 1 ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mkdirs/procdir: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdir) */


