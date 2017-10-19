/* ptma */

/* POSIX® Thread Mutex Attribute manipulation */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a cleaned up version of the p-threads mutex-attribute set of
	subroutines (object).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptma.h"


/* local defines */

#define	TO_NOMEM	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		ptma_create(PTMA *) ;


/* exported subroutines */


int ptma_create(PTMA *op)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = pthread_mutexattr_init(op)) > 0) rs = (- rs) ;
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
/* end subroutine (ptma_create) */


int ptma_destroy(PTMA *op)
{
	int		rs ;

	rs = pthread_mutexattr_destroy(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_destroy) */


int ptma_getprioceiling(PTMA *op,int *oldp)
{
	int		rs ;

	rs = pthread_mutexattr_getprioceiling(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_getprioceiling) */


int ptma_setprioceiling(PTMA *op,int new)
{
	int		rs ;

	rs = pthread_mutexattr_setprioceiling(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_setprioceiling) */


int ptma_getprotocol(PTMA *op,int *oldp)
{
	int		rs ;

	rs = pthread_mutexattr_getprotocol(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_getprotocol) */


int ptma_setprotocol(PTMA *op,int new)
{
	int		rs ;

	rs = pthread_mutexattr_setprotocol(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_setprotocol) */


int ptma_getpshared(PTMA *op,int *oldp)
{
	int		rs ;

	rs = pthread_mutexattr_getpshared(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_getpshared) */


int ptma_setpshared(PTMA *op,int new)
{
	int		rs ;

	rs = pthread_mutexattr_setpshared(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_setpshared) */


int ptma_getrobustnp(PTMA *op,int *oldp)
{
	int		rs ;

	rs = pthread_mutexattr_getrobust_np(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_getrobustnp) */


int ptma_setrobustnp(PTMA *op,int new)
{
	int		rs ;

#if	defined(SYSHAS_MUTEXROBUST) && (SYSHAS_MUTEXROBUST > 0)
	rs = pthread_mutexattr_setrobust_np(op,new) ;
	if (rs > 0) rs = (- rs) ;
#else
	rs = SR_OK ;
#endif

	return rs ;
}
/* end subroutine (ptma_setrobustnp) */


int ptma_gettype(PTMA *op,int *oldp)
{
	int		rs ;

	rs = pthread_mutexattr_gettype(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_gettype) */


int ptma_settype(PTMA *op,int new)
{
	int		rs ;

	rs = pthread_mutexattr_settype(op,new) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptma_settype) */


