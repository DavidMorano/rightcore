/* getloghost */

/* get user information from UTMPX database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get the host that a user logged in from (there is almost always one
        now-a-days!).

	Synopsis:

	int getloghost(rbuf,rlen,sid)
	char		rbuf[] ;
	int		rlen ;
	pid_t		sid ;

	Arguments:

	rbuf		buffer to hold resulting logname
	rlen		length of user supplied buffer
	sid		session ID to look up

	Returns:

	>=0		length of user logname
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getutmpent.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;


/* external variables */


/* forward reference */


/* exported subroutines */


int getloghost(char *rbuf,int rlen,pid_t sid)
{
	GETUTMPENT	e ;
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if ((rs = getutmpent(&e,sid)) >= 0) {
	    rs = sncpy1(rbuf,rlen,e.host) ;
	}

	return rs ;
}
/* end subroutine (getloghost) */


