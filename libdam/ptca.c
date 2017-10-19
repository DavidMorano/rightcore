/* ptca */

/* POSIX® Thread Condition Attribute manipulation */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a cleaned up version of the p-threads condition-variable
	attribute set of subroutines (object).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptca.h"


/* local defines */

#define	TO_NOMEM	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		ptca_create(PTCA *) ;


/* exported subroutines */


int ptca_create(PTCA *op)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = pthread_condattr_init(op)) > 0) rs = (- rs) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
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
/* end subroutine (ptca_create) */


int ptca_destroy(PTCA *op)
{
	int		rs ;

	rs = pthread_condattr_destroy(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptca_destroy) */


int ptca_getpshared(PTCA *op,int *oldp)
{
	int		rs ;

	rs = pthread_condattr_getpshared(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptca_getpshared) */


int ptca_setpshared(PTCA *op,int new)
{
	int		rs ;

	rs = pthread_condattr_setpshared(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptca_setpshared) */


