/* onckeyalready */

/* is the ONC private key already with the Key Server? */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was written to deal with NIS key authorization issues
	when using Solaris 2.x.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if the KEYSERV daemon already has our secret ONC
	private key.

	We use the (formerly) secret undocumented subroutine
	'key_secretkey_is_set()' to do the job for us.

	Synopsis:

	int onckeyalready(netname)
	const char	netname[] ;

	Arguments:

	netname		a user supplied buffer with an ONC "netname"

	Returns:

	>=0	OK, 0=NO, 1=YES
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"stubrpc.h"


/* local defines */


/* exported subroutines */


/* exported subroutines */


int onckeyalready(cchar *netname)
{
	int		rs ;
	if (netname == NULL) return SR_FAULT ;
	if ((rs = key_secretkey_is_set()) < 0) rs = SR_NOPKG ;
	return rs ;
}
/* end subroutine (onckeyalready) */


