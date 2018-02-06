/* timestr_elapsed */

/* convert UNIX time into an elapsed time character string */


#define	CF_DEBUGS	0		/* compile-time debuging */


/* revision history:

	= 1998-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a string of elapsed time.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<tzfile.h>

#include	<localmisc.h>


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;


/* forward subroutines */


/* exported subroutines */


char *timestr_elapsed(t,rbuf)
CONST time_t	t ;
char		rbuf[] ;
{
	uint		days, hours, mins, secs ;
	uint		tmins, thours ;
	const int	rlen = TIMEBUFLEN ;
	int		rs ;
	cchar		*fmt ;

#if	CF_DEBUGS
	debugprintf("timestr_elapsed: ent\n") ;
#endif

	tmins = (t / 60) ;
	secs = (t % 60) ;
	thours = (tmins / 60) ;
	mins = (tmins % 60) ;
	days = (thours / 24) ;
	hours = (thours % 24) ;

	fmt = "%5u-%02u:%02u:%02u" ;
	rs = bufprintf(rbuf,rlen,fmt,days,hours,mins,secs) ;

	return (rs >= 0) ? rbuf : NULL ;
}
/* end subroutine (timestr_elapsed) */


