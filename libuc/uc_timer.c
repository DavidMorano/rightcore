/* uc_timer */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	POSIX® timer (|timer(3rt|) operations.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	ITIMERSPEC
#define	ITIMERSPEC	struct itimerspec
#endif

#define	TO_AGAIN	60


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	msleep(int) ;
extern int	haslc(const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_timercreate(clockid_t cid,struct sigevent *sep,timer_t *tmp)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = timer_create(cid,sep,tmp)) < 0) rs = (- errno) ;
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
	    }
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_timercreate) */


int uc_timerdelete(timer_t tid)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = timer_delete(tid)) < 0) rs = (- errno) ;
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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_timerdelete) */


int uc_timerdestroy(timer_t tid)
{
	return uc_timerdelete(tid) ;
}
/* end subroutine (uc_timerdestroy) */


int uc_timerset(timer_t tid,int tf,ITIMERSPEC *ntvp,ITIMERSPEC *otvp)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = timer_settime(tid,tf,ntvp,otvp)) < 0) rs = (- errno) ;
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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_timerset) */


int uc_timerget(timer_t tid,ITIMERSPEC *otvp)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = timer_gettime(tid,otvp)) < 0) rs = (- errno) ;
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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_timerget) */


int uc_timerover(timer_t tid)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = timer_getoverrun(tid)) < 0) rs = (- errno) ;
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
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_timerover) */


