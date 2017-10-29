/* ptm */

/* POSIX Thread Mutex manipulation */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a cleaned up version of the p-threads mutex locking facility.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ptm.h"
#include	"ptma.h"


/* local defines */

#define	TO_NOMEM	5
#define	TO_AGAIN	5

/* number of polls per second */
#undef	NLPS
#define	NLPS		5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		ptm_create(PTM *,PTMA *) ;
int		ptm_lockto(PTM *,int) ;


/* exported subroutines */


int ptm_init(PTM *op,PTMA *ap)
{
	return ptm_create(op,ap) ;
}
/* end subroutine (ptm_init) */


int ptm_create(PTM *op,PTMA *ap)
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;
	int	to_again = TO_AGAIN ;

#if	CF_DEBUGS
	debugprintf("ptm_init: op=%p ap=%p\n",op,ap) ;
#endif

	memset(op,0,sizeof(PTM)) ; /* doesn't hurt anything - right? */

again:
	rs = pthread_mutex_init(op,ap) ;
	if (rs > 0) rs = (- rs) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	        if (to_nomem-- > 0) {
			msleep(1000) ;
	            goto again ;
		}
	        break ;
	    case SR_AGAIN:
	        if (to_again-- > 0) {
			msleep(1000) ;
	            goto again ;
		}
	        break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ptm_create) */


int ptm_destroy(PTM *op)
{
	int	rs ;

	rs = pthread_mutex_destroy(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptm_destroy) */


int ptm_setprioceiling(PTM *op,int new,int *oldp)
{
	int	rs ;

	rs = pthread_mutex_setprioceiling(op,new,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptm_setprioceiling) */


int ptm_getprioceiling(PTM *op,int *oldp)
{
	int	rs ;

	rs = pthread_mutex_getprioceiling(op,oldp) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptm_getprioceiling) */


int ptm_lock(PTM *op)
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;
	int	to_again = TO_AGAIN ;

again:
	rs = pthread_mutex_lock(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	        if (to_nomem-- > 0) {
			msleep(1000) ;
	            goto again ;
		}
	        break ;
	    case SR_AGAIN:
	        if (to_again-- > 0) {
			msleep(1000) ;
	            goto again ;
		}
	        break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ptm_lock) */


int ptm_locker(PTM *op,int to)
{

	return ptm_lockto(op,to) ;
}
/* end subroutine (ptm_locker) */


int ptm_lockto(PTM *op,int to)
{
	const int	mint = (1000/NLPS) ;
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_again = TO_AGAIN ;
	int		cto ;
	int		c = 0 ;

	if (to < 0) to = (INT_MAX/(2*NLPS)) ;
	cto = (to*NLPS) ;

again:
	rs = pthread_mutex_trylock(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	        if (to_nomem-- > 0) {
		    msleep(1000) ;
	            goto again ;
		}
	        break ;
	    case SR_AGAIN:
	        if (to_again-- > 0) {
		    msleep(1000) ;
	            goto again ;
		}
	        break ;
	    } /* end switch */
	} /* end if */

	if ((rs == SR_BUSY) && (++c < cto)) {
	    msleep(mint) ;
	    goto again ;
	}

	return rs ;
}
/* end subroutine (ptm_lockto) */


int ptm_trylock(PTM *op)
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;
	int	to_again = TO_AGAIN ;

again:
	rs = pthread_mutex_trylock(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	        if (to_nomem-- > 0) {
		    msleep(1000) ;
	            goto again ;
		}
	        break ;
	    case SR_AGAIN:
	        if (to_again-- > 0) {
		    msleep(1000) ;
	            goto again ;
		}
	        break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ptm_trylock) */


int ptm_unlock(PTM *op)
{
	int	rs ;

	rs = pthread_mutex_unlock(op) ;
	if (rs > 0)
	    rs = (- rs) ;

	return rs ;
}
/* end subroutine (ptm_unlock) */


