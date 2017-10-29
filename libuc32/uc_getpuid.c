/* uc_getpuid */

/* interface component for UNIX® library-3c */
/* get the UID of a given process by its PID */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-15, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves from the system the UID of a given process
	(if that process exists).

	Synopsis:

	int uc_getpuid(pid_t pid)

	Arguments:

	pid		PID of process to retrieve UID for

	Returns:

	>=0		UID of process (as an integer)
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<storebuf.h>


/* local defines */

#define	PROCDNAME	"/proc"


/* external subroutines */


/* local variables */


/* forward references */

static int	mkpidfname(char *,cchar *,pid_t) ;


/* exported subroutines */


int uc_getpuid(pid_t pid)
{
	uid_t		uid = 0 ;
	int		rs = SR_OK ;
	int		r ;

	if (pid < 0)
	    return SR_INVALID ;

	if (pid > 0) {
	    const char	*pd = PROCDNAME ;
	    char	pidfname[MAXPATHLEN + 1] ;

	    if ((rs = mkpidfname(pidfname,pd,pid)) >= 0) {
	        struct ustat	sb ;
		const int	nrs = SR_NOENT ;
	        if ((rs = u_stat(pidfname,&sb)) == nrs) {
		    if ((rs = u_stat(pd,&sb)) >= 0) {
		        rs = SR_SRCH ;
		    } else if (rs == nrs) {
		    	rs = SR_NOSYS ;
		    }
		} else {
	            uid = sb.st_uid ;
		}
	    } /* end if (mkpidfname) */

	} else {
	    uid = getuid() ;
	}

	r = uid ;
	r &= INT_MAX ;
	return (rs >= 0) ? r : rs ;
}
/* end subroutine (uc_getpuid) */


/* local subroutines */


static int mkpidfname(char *fname,cchar *dev,pid_t pid)
{
	const int	flen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(fname,flen,i,dev,-1) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (fname[i-1] != '/')) {
	    rs = storebuf_char(fname,flen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    const int	v = pid ;
	    rs = storebuf_deci(fname,flen,i,v) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkpidfname) */


