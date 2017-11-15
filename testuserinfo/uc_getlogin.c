/* uc_getlogin */

/* interface component for UNIX® library-3c */
/* get the username of this login session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is just the wrapper for the system's own 'getlogin(3c)', but it's
        reentrant where available.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<limits.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	BUFLEN		_POSIX_LOGIN_NAME_MAX


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;


/* exported subroutines */


int uc_getlogin(char *rbuf,int rlen)
{
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = BUFLEN ;

	if ((rlen >= 0) && (rlen < BUFLEN))
	    return SR_OVERFLOW ;

#ifdef	_POSIX_PTHREAD_SEMANTICS

#if	CF_DEBUGS
	debugprintf("uc_getlogin: POSIX buflen=%d\n",buflen) ;
#endif

	    if ((rs = getlogin_r(rbuf,rlen)) != 0) rs = (- errno) ;
	    if (rs >= 0) rs = strlen(rbuf) ;

#else /* _POSIX_PTHREAD_SEMANTICS */
	{
	    cchar	*rp ;

#if	CF_DEBUGS
	debugprintf("uc_getlogin: not POSIX\n") ;
#endif

	    rp = getlogin() ;
	    rs = (rp != NULL) ? 0 : (- errno) ;
	    if (rs >= 0) rs = sncpy1(rbuf,rlen,rp) ;
	}
#endif /* _POSIX_PTHREAD_SEMANTICS */

	return rs ;
}
/* end subroutine (uc_getlogin) */


