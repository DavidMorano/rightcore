/* uc_readlink */

/* interface component for UNIX® library-3c */
/* read a symbolic link */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
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


int uc_readlink(cchar *fname,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		fl ;

	if (fname == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	fl = strlen(fname) ;

	if (hasnonpath(fname,fl)) {
	    rs = SR_INVALID ;
	} else {
	    char	efname[MAXPATHLEN + 1] ;

	    if ((rs = mkuserpath(efname,NULL,fname,fl)) > 0) {
	        fname = efname ;
	    } else if (rs == 0) {
	        if ((rs = mkcdpath(efname,fname,fl)) > 0) {
	            fname = efname ;
	        } else if (rs == 0) {
	            rs = mkvarpath(efname,fname,fl) ;
	            if (rs > 0) fname = efname ;
		}
	    }

	    if (rs >= 0) {
	        rs = u_readlink(fname,rbuf,rlen) ;
	    }

	} /* end if (nonpath or other) */

	return rs ;
}
/* end subroutine (uc_readlink) */


