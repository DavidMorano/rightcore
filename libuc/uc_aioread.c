/* uc_aioread */

/* interface component for UNIX® library-3c */
/* asynchronous read */


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

        Get some amount of data and time it also so that we can abort if it
        times out.

	Synopsis:

	int uc_aioread(aiobp)
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
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* forward references */


/* exported subroutines */


int uc_aioread(struct aiocb *aiobp)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if (aiobp == NULL) return SR_FAULT ;

	repeat {
	    errno = 0 ;
	    if ((rs = aio_read(aiobp)) == -1) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
		        msleep(1000) ;
		    } else {
		        f_exit = TRUE ;
		    }
	            break ;
		case SR_INTR:
		    break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_aioread) */


