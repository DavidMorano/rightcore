/* sntid */

/* create a string representation of a Pthread Thread-ID */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We convert a Pthread Thread-ID into a string representation.

	Synopsis:

	int sntid(char *rbuf,int rlen,pthread_t tid)

	Arguments:

	rbuf		result buffer
	rlen		length of supplied result buffer
	tid		pthread thread-ID to convert

	Returns:

	<0		error (mostly overflow)
	>=0		length of create string in result buffer


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<pthread.h>
#include	<localmisc.h>


/* external subroutines */

extern int	ctdecui(char *,int,uint) ;


/* local structures */


/* local variables */


/* exported subroutines */


int sntid(char *dp,int dl,pthread_t tid)
{
	uint	uv = tid ;
	return ctdecui(dp,dl,uv) ;
}
/* end subroutine (sntid) */


