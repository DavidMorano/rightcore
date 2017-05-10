/* uc_aiolist */

/* interface component for UNIX® library-3c */
/* asynchronous list I/O */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */


/* revision history:

	= 1998-05-22, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Note: All platforms we care about now have POSIX AIO. So we don't have
        to worry about compile-time switches to dummy suboutines.

	Queue up a whole group of asynchronous operations.

	Synopsis:

	int uc_aiolist(aiobp)
	struct aiocb	*aiobp ;

	Arguments:

	aiobp		AIO block pointer

	Returns:

	>=0		amount of data returned
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<poll.h>
#include	<aio.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	TO_INTR		(10 * 60)


/* external subroutines */


/* forward references */


/* exported subroutines */


int uc_aiolist(mode,aiobp,n,sep)
int		mode ;
struct aiocb	*const aiobp[] ;
int		n ;
struct sigevent	*sep ;
{
	int	rs ;


	if (aiobp == NULL)
	    return SR_FAULT ;

again:
	errno = 0 ;
	if ((rs = lio_listio(mode,aiobp,n,sep)) == -1)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_aiolist) */


