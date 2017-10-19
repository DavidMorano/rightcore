/* ptrwa */

/* POSIX® Thread Read-Write lock attribute manipulation */


/* revision history:

	= 1999-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides operational attributes for the POSIX
	read-write lock.

	Side-note: The POXIX read-write lock is broken on the Solaris platform
	and is not used any long.  So this object here gets little use
	now-a-days.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptrwa.h"


/* local defines */

#define	TO_NOMEM	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		ptrwa_create(PTRWA *) ;


/* exported subroutines */


int ptrwa_create(PTRWA *op)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = pthread_rwlockattr_init(op)) > 0) rs = (- rs) ;
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
/* end subroutine (ptrwa_create) */


int ptrwa_destroy(PTRWA *op)
{
	int		rs ;

	rs = pthread_rwlockattr_destroy(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptrwa_destroy) */


int ptrwa_getpshared(PTRWA *op,int *oldp)
{
	int		rs ;

	rs = pthread_rwlockattr_getpshared(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptrwa_getpshared) */


int ptrwa_setpshared(PTRWA *op,int new)
{
	int		rs ;

	rs = pthread_rwlockattr_setpshared(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptrwa_setpshared) */


