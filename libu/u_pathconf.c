/* u_pathconf */

/* query a FS directory path for configuration */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the sane version of 'pathconf(2)'.  Note that the
	returned value can (and will be '-1') if the resource queried is
	infinite.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_IO		60
#define	TO_DQUOT	(5 * 60)
#define	TO_NOSPC	(5 * 60)


/* exported subroutines */


int u_pathconf(cchar *fname,int name,long *rp)
{
	long		lw ;
	int		rs = SR_OK ;

	errno = 0 ;
	lw = pathconf(fname,name) ;
	if ((lw < 0) && (errno != 0)) rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_pathconf: rs=%d lw=%ld\n",rs,lw) ;
#endif

#ifdef	_PC_CHOWN_RESTRICTED /* can you Solaris® guys fix this brain-damage? */
	if ((rs >= 0) && (name == _PC_CHOWN_RESTRICTED) && (lw == -1)) {
	   lw = 0 ;
	}
#endif /* _PC_CHOWN_RESTRICTED */

	if (rp != NULL) {
	    *rp = (rs >= 0) ? lw : 0 ;
	}

	if (rs >= 0) rs = (lw & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("u_pathconf: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_pathconf) */


