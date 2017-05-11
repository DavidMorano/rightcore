/* uc_timer */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	POSIX® timer operations.


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

#define	TO_AGAIN	60


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
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
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = timer_create(cid,sep,tmp)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_AGAIN:
	        if (to_again-- > 0) goto again ;
		break ;
	    case SR_INTR:
		goto again ;
	    } /* end if */
	}

	return rs ;
}
/* end subroutine (uc_timercreate) */


int uc_timerdelete(timer_t tid)
{
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = timer_delete(tid)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_AGAIN:
	        if (to_again-- > 0) goto again ;
		break ;
	    case SR_INTR:
		goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (uc_timerdelete) */


int uc_timerset(tid,tf,ntvp,otvp)
timer_t		tid ;
int		tf ;
struct itimerspec	*ntvp ;
struct itimerspec	*otvp ;
{
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = timer_settime(tid,tf,ntvp,otvp)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_AGAIN:
	        if (to_again-- > 0) goto again ;
		break ;
	    case SR_INTR:
		goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (uc_timerset) */


int uc_timerget(tid,otvp)
timer_t		tid ;
struct itimerspec	*otvp ;
{
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = timer_gettime(tid,otvp)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_AGAIN:
	        if (to_again-- > 0) goto again ;
		break ;
	    case SR_INTR:
		goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (uc_timerget) */


int uc_timerover(tid)
timer_t		tid ;
{
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = timer_getoverrun(tid)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_AGAIN:
	        if (to_again--) goto nap ;
		break ;
	    case SR_INTR:
		goto again ;
	    } /* end switch */
	} /* end if */

	return rs ;

/* take a little nap */
nap:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (uc_timerover) */


