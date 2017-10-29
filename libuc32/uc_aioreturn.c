/* uc_aioreturn */

/* interface component for UNIX® library-3c */
/* get return for asynchronous operation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-05-22, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Note: All platforms we care about now have POSIX AIO. So we don't have
        to worry about compile-time switches to dummy suboutines.

	Synopsis:

	int uc_aioreturn(aiobp)
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

#define	TO_AGAIN	10


/* external subroutines */


/* forward references */


/* exported subroutines */


int uc_aioreturn(aiobp)
struct aiocb	*aiobp ;
{
	ssize_t	size ;

	int	rs ;
	int	ec ;


	if (aiobp == NULL)
	    return SR_FAULT ;

again:
	errno = 0 ;
	rs = SR_OK ;
	if ((ec = aio_error(aiobp)) == -1)
		rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("uc_aioreturn: aio_error() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (ec == 0) {
		size = aio_return(aiobp) ;
		rs = (size == -1) ? (- errno) : (int) size ;
	    } else
		rs = (- ec) ;

	} /* end if */

	return rs ;
}
/* end subroutine (uc_aioreturn) */


