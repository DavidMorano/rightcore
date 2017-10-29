/* gettid */

/* get the current thread ID */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int gettid()

	Arguments:

	Returns:

	-		thread ID


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<upt.h>
#include	<localmisc.h>


/* local defines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


pthread_t gettid()
{
	pthread_t	tid ;
	uptself(&tid) ;
	return tid ;
}
/* end subroutine (gettid) */


