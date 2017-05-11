/* uc_swapcontext */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<ucontext.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_swapcontext(uc1p,uc2p)
ucontext_t		*uc1p ;
const ucontext_t	*uc2p ;
{
	int	rs ;


	if ((rs = (int) swapcontext(uc1p,uc2p)) < 0) rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("uc_swapcontext: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_swapcontext) */


