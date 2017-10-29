/* username */

/* get the current user name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Get user information from PASSWD database.

	Synopsis:

	int username(username)
	char	username[] ;

	Arguments:

	- address of buffer to receive the username

	Returns:

	>=0		length of returned string
	<0		error


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int username(username)
char	username[] ;
{
	int	rs ;


	if (username == NULL)
	    return SR_FAULT ;

	rs = getusername(username,USERNAMELEN,-1) ;

	return rs ;
}
/* end subroutine (username) */


