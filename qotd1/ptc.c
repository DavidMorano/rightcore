/* ptc */

/* POSIX® Thread Condition manipulation */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a cleaned up version of the p-threads condition-variable
	locking facility.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>
#include	<time.h>		/* for 'struct timespec' */

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptca.h"
#include	"ptc.h"


/* local defines */

#define	TO_NOMEM	5
#define	TO_AGAIN	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		ptc_create(PTC *,PTCA *) ;


/* exported subroutines */


int ptc_create(PTC *op,PTCA *ap)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = pthread_cond_init(op,ap)) > 0) rs = (- rs) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
		        msleep(1000) ;
		    } else {
	                f_exit = TRUE ;
		    }
	            break ;
	        case SR_AGAIN:
	            if (to_again-- > 0) {
		        msleep(1000) ;
		    } else {
	                f_exit = TRUE ;
		    }
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (ptc_create) */


int ptc_destroy(PTC *op)
{
	int		rs ;

	if ((rs = pthread_cond_destroy(op)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_destroy) */


int ptc_broadcast(PTC *op)
{
	int		rs ;

	if ((rs = pthread_cond_broadcast(op)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_broadcast) */


int ptc_signal(PTC *op)
{
	int		rs ;

	if ((rs = pthread_cond_signal(op)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_signal) */


int ptc_wait(PTC *op,PTM *mp)
{
	int		rs ;

	if ((rs = pthread_cond_wait(op,mp)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_wait) */


int ptc_waiter(PTC *op,PTM *mp,int to)
{
	int		rs ;

	if (to >= 0) {
	    struct timespec	ts ;
	    clock_gettime(CLOCK_REALTIME,&ts) ;
	    ts.tv_sec += to ;
	    rs = ptc_timedwait(op,mp,&ts) ;
	} else {
	    rs = ptc_wait(op,mp) ;
	}

	return rs ;
}
/* end subroutine (ptc_waiter) */


int ptc_timedwait(PTC *op,PTM *mp,struct timespec *tp)
{
	int		rs ;

	if ((rs = pthread_cond_timedwait(op,mp,tp)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_timedwait) */


int ptc_reltimedwaitnp(PTC *op,PTM *mp,struct timespec *tp)
{
	int		rs ;

	if ((rs = pthread_cond_reltimedwait_np(op,mp,tp)) > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptc_reltimedwaitnp) */


