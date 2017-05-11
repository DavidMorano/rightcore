/* pt_sigmask */

/* set a p-thread signal mask */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Yes, Virginia. We need this for p-thread work. We need it for the same
        reasons we need any other 'uc_xxx(3uc)' subroutine.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int pt_sigmask(int how,sigset_t *setp,sigset_t *osetp)
{
	int		rs ;

	errno = 0 ;
	rs = pthread_sigmask(how,setp,osetp) ;
	if (rs != 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (pt_sigmask) */


