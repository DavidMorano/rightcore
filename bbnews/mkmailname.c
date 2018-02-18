/* mkmailname */

/* fix a name to make it like a mail-name */


/* revision history:

	= 1998-08-12, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

	This subroutine takes the GECOS version of a user name (this is from
	the fifth field of the system password file) and creates the associated
	'mailname' from that.

	Synopsis:

	int mkmailname(rbuf,rlen,gn,gl)
	char		rbuf[] ;
	int		rlen ;
	const char	gn[] ;
	int		gl ;

	Arguments:

	rbuf		supplied buffer to receive result
	rlen		length of supplied buffer
	gn		source text to work from (should be GECOS-name)
	gl		source text length

	Returns:

	>=0		length of extracted mailname string
	<0		error

*	SUBROUTINES CALLED:						
*		Only system routines are called.
*
*	GLOBAL VARIABLES USED:						
*		None!!  AS IT SHOULD BE!!
*
*
*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<realname.h>
#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* forward references */


/* exported subroutines */


int mkmailname(char *rbuf,int rlen,cchar *gp,int gl)
{
	REALNAME	rn ;
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;
	if (gp == NULL) return SR_FAULT ;

	if ((rs = realname_start(&rn,gp,gl)) >= 0) {

	    rs = realname_mailname(&rn,rbuf,rlen) ;

	    realname_finish(&rn) ;
	} /* end if (readlname) */

	return rs ;
}
/* end subroutine (mkmailname) */


