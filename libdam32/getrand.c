/* getrand */

/* get random data from the UNIX® kernel */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int getrand(char *rbuf,int rlen)

	Arguments:

	rbuf		result buffer
	rlen		length of supplied result buffer

	Returns:

	<0		error
	>0		returned number of bytes


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getrand(void *rbuf,int rlen)
{
	return uc_getrandom(rbuf,rlen,0) ;
}
/* end subroutine (getrand) */


