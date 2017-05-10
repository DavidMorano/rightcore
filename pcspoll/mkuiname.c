/* mkuiname */

/* try to divine the best real name from a USERINFO object */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We try to divine the best name that we can for the current user.  We do
	this simply by looking through some options contained in the USERINFO
	object.

	Synopsis:

	int mkuiname(rbuf,rlen,uip)
	char		rbuf[] ;
	int		rlen ;
	USERINFO	*uip ;

	Arguments:

	uip		pointer to USERINFO object
	rbuf		supplied result buffer
	rlen		supplied result buffer length

	Returns:

	<0		error (really just whether overflow)
	>=0		length of resulting name


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<userinfo.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;


/* local structures */


/* exported subroutines */


int mkuiname(char rbuf[],int rlen,USERINFO *uip)
{
	int		rs = SR_OK ;
	const char	*np = NULL ;

	if (rbuf == NULL) return SR_FAULT ;
	if (uip == NULL) return SR_FAULT ;

	if (rlen < 0)
	    rlen = NODENAMELEN ;

	rbuf[0] = '\0' ;
	if ((uip->fullname != NULL) && (uip->fullname[0] != '\0')) {
	    np = uip->fullname ;
	} else if ((uip->name != NULL) && (uip->name[0] != '\0')) {
	    np = uip->name ;
	} else if ((uip->mailname != NULL) && (uip->mailname[0] != '\0')) {
	    np = uip->mailname ;
	} else if ((uip->username != NULL) && (uip->username[0] != '\0')) {
	    np = uip->username ;
	}

	if (np != NULL) {
	    rs = sncpy1(rbuf,rlen,np) ;
	}

	return rs ;
}
/* end subroutine (mkuiname) */


