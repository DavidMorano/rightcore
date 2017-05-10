/* uc_ftime */

/* interface component for UNIX® library-3c */
/* cleaned up version of 'ftime(3c)' */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This was originally supposed to implement the (so-called) |ftime(3c)|
        subroutine. But that subroutine is *broken* by design. Why is it broken?
        It is broken because if always returns the standard time-zone offset
        from GMT in its 'timezone' structure member. This is essentually useless
        in real life.  The only useful thing to return there is the *current*
	time-zone offset as indicated by the 'dstflag' member. So this is what
	we do instead of the original behavior.  We instead return the current
	time-zone offset in the 'timezone' variable, as indicated by the
	'dstflag' member variable.

        Use of this subroutine (|uc_ftime(3dam)|) and its UNIX® inspired piece
        of crap (|ftime(3c)|) should be eliminated from all code as soon as time
        permits.

A 'timeb' structure looks like:

struct timeb {
	time_t		time ; 		// time, seconds since the epoch
	unsigned short 	millitm ;	// 1000 msec of additional accuracy
	short		timezone ;	// timezone, minutes west of GMT
	short		dstflag ;	// DST flag
} ;


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmtime.h>


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */


/* exported subroutines */


int uc_ftime(struct timeb *tbp)
{
	struct timeval	tv ;
	int		rs ;

	if (tbp == NULL) return SR_FAULT ;

	memset(tbp,0,sizeof(struct timeb)) ;
	if ((rs = uc_gettimeofday(&tv,NULL)) >= 0) {
	    TMTIME	tmt ;
	    tbp->time = tv.tv_sec ;
	    tbp->millitm = (tv.tv_usec / 1000) ;
	    if ((rs = tmtime_localtime(&tmt,tbp->time)) >= 0) {
	        tbp->timezone = (tmt.gmtoff / 60) ;
	        tbp->dstflag = tmt.isdst ;
	        rs = (tmt.isdst > 0) ? 1 : 0 ;
	    } /* end if (tmtime_localtime) */
	} /* end if (uc_gettimeofday) */

	return rs ;
}
/* end subroutine (uc_ftime) */


