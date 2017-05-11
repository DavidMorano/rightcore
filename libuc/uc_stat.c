/* uc_stat */

/* interface component for UNIX® library-3c */
/* get status on a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine provides an enchanced |stat(2)| function. It also
        removes trailing slash ('/') character from file-names, but it will
        preserve a lone pair of characters consisting of '//'.

	Some systems do not ignore trailing slash characters on file-names,
	so we take care of it for them (at least here we do).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	FSTYPE_FLOAT	"float"

#ifdef	_ST_FSTYPSZ
#define	FSTYPESZ	_ST_FSTYPSZ
#else
#define	FSTYPESZ	16
#endif


/* external subroutines */

extern int	mkuserpath(char *,const char *,const char *,int) ;
extern int	mkcdpath(char *,const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	hasvarpathprefix(const char *,int) ;
extern int	hasnonpath(const char *,int) ;


/* forward references */


/* exported subroutines */


int uc_stat(cchar *fname,struct ustat *sbp)
{
	int		rs = SR_OK ;
	int		fl ;
	int		flen = 0 ;
	cchar		*fbuf = NULL ;

	if (fname == NULL) return SR_FAULT ;
	if (sbp == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	fl = strlen(fname) ;

	if ((fl > 1) && (fname[0] != '/')) {
	    if (fname[fl-1] == '/') {
	        fl -= 1 ;
	        if ((rs = uc_libmallocstrw(fname,fl,&fbuf)) >= 0) {
		    fname = fbuf ;
		}
	    }
	}

	if (rs >= 0) {

	    if (hasnonpath(fname,fl)) {
	        memset(sbp,0,sizeof(struct ustat)) ;
	        sbp->st_mode = S_IFNAM ;
	        sbp->st_uid = UID_NOBODY ;
	        sbp->st_gid = GID_NOBODY ;
	        strncpy(sbp->st_fstype,FSTYPE_FLOAT,FSTYPESZ) ;
	    } else {
	        const int	elen = MAXPATHLEN ;
	        char		*ebuf ;
	        if ((rs = uc_libmalloc((elen+1),&ebuf)) >= 0) {
	            if ((rs = mkuserpath(ebuf,NULL,fname,fl)) > 0) {
	                fname = ebuf ;
	            } else if (rs == 0) {
	                if ((rs = mkcdpath(ebuf,fname,fl)) > 0) {
	                    fname = ebuf ;
	                } else if (rs == 0) {
	                    if ((rs = mkvarpath(ebuf,fname,fl)) > 0) {
	                        fname = ebuf ;
		            }
		        }
	            }
	            if (rs >= 0) {
	                if ((rs = u_stat(fname,sbp)) >= 0) {
		            flen = (sbp->st_size & INT_MAX) ;
			}
	            }
		    uc_libfree(ebuf) ;
	        } /* end if (memory-allocation) */
	    } /* end if (nonpath) */

	    if (fbuf != NULL) {
		uc_libfree(fbuf) ;
	    }

	} /* end if (ok) */

	return (rs >= 0) ? flen : rs ;
}
/* end subroutine (uc_stat) */


