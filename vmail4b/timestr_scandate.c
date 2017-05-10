/* timestr_scandate */

/* convert UNIX® time into a VMAIL scan date-string format */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-01, David A­D­ Morano
        This subroutine was originally written. This subroutine serves to create
        the date-string that is expected to be seen in the msg-scan line of the
        display. I have to provide the msg-date-string that the program has
        previously displayed.

	= 2001-08-23, David A­D­ Morano
        I changed the order of elements in the displayed date. The old
        date-string was organized to be rather consistent with the stupid way
        that RFC-822 dates are represented. That was always a stupid order to be
        presented to human beings.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
	Return a date string in the supplied buffer in a format (the new
	format as of 2001-08-23) as (for example):
		 4 Jun 20:52 97

	This amounts to using a 'sntmtime(3dam)' format string of:
	    	"%e %b %R %y"


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<tmtime.h>
#include	<sntmtime.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */


/* external variables */


/* local (static) variables */


/* exported subroutines */


char *timestr_scandate(t,buf)
time_t		t ;
char		buf[] ;
{
	TMTIME		ts ;

	int	rs ;


	if (buf == NULL) return NULL ;

	if ((rs = tmtime_localtime(&ts,t)) >= 0)
	    rs = sntmtime(buf,TIMEBUFLEN,&ts,"%e %b %R %y") ;

	if (rs < 0) buf[0] = '\0' ; /* probably happens already! */

	return buf ;
}
/* end subroutine (timestr_scandate) */


