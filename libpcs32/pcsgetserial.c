/* pcsgetserial */

/* PCS Get-Serial number */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a unique serial number from a specified
        file. These numbes are used for sequencing and other purposes in general
        code. An attempt is made to lock the SERIAL file and if the lock fails,
        the subroutine returns an error (negative number).

        Locking may indeed fail due to the very poorly written file locking code
        on the old SunOS 4.xxx version of the UNIX system. Remote file locking
        over NFS on the old SunOS 4.xxx systems **never** worked correctly!
        Other errors, like "couldn't create the file" are reported as such.

	Synopsis:

	int pcsgetserial(pr)
	const char	pr[] ;

	Arguments:

	pr		PCS program-root

	Returns:

	>0		the serial number
	==0		file was just created
	<0		could not get it!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	SERIALFNAME
#define	SERIALFNAME	"var/serial"
#endif

#define	NDF		"pcsgetserial.deb"


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	getserial(const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;


/* local variables */


/* exported subroutines */


int pcsgetserial(cchar *pr)
{
	int		rs ;
	int		s = 0 ;
	char		sfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("pcsgetserial: ent pr=%s\n",pr) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (pr[0] == '\0') return SR_INVALID ;

	if ((rs = mkpath2(sfname,pr,SERIALFNAME)) >= 0) {
	    if ((rs = getserial(sfname)) >= 0) {
		s = rs ;
	    } else if (isNotPresent(rs)) {
		const mode_t	m = 0666 ;
		s = 0 ;
		uc_unlink(sfname) ;
		if ((rs = uc_create(sfname,m)) >= 0) {
		    const int	fd = rs ;
		    if ((rs = uc_fminmod(fd,m)) >= 0) {
	    	        const int	n = _PC_CHOWN_RESTRICTED ;
	    	        if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
		            USTAT	sb ;
		            if ((rs = u_fstat(fd,&sb)) >= 0) {
			        rs = u_fchown(fd,sb.st_uid,sb.st_gid) ;
			    }
			} /* end if (u_pathconf) */
		    } /* end if (uc_minmod) */
		    u_close(fd) ;
		} /* end if (uc_createfile) */
	    }
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("pcsgetserial: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (pcsgetserial) */


