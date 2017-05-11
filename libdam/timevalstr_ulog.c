/* timevalstr_ulog */

/* convert UNIX time into a Julian like character string */


/* revision history:

	= 1995-08-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/time.h>
#include	<time.h>

#include	<localmisc.h>


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY		100
#endif


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;


/* exported subroutines */


char *timevalstr_ulog(tvp,buf)
struct timeval	*tvp ;
char		buf[] ;
{
	struct tm	*timep ;

#if	defined(SYSHAS_LOCALTIMER) && (SYSHAS_LOCALTIMER > 0)
	struct tm	ts ;
#endif


#if	defined(SYSHAS_LOCALTIMER) && (SYSHAS_LOCALTIMER > 0)
	timep = (struct tm *) localtime_r(&tvp->tv_sec,&ts) ;
#else
	timep = localtime(&tvp->tv_sec) ;
#endif

/* this (below) reallly cannot fail */

	if (timep != NULL)
	    bufprintf(buf,TIMEBUFLEN,"%02u%02u%02u_%02u%02u:%02u.%06u",
	        (timep->tm_year % NYEARS_CENTURY),
	        (timep->tm_mon + 1),
	        timep->tm_mday,
	        timep->tm_hour,
	        timep->tm_min,
	        timep->tm_sec,
	        tvp->tv_usec) ;

	return (timep != NULL) ? buf : NULL ;
}
/* end subroutine (timevalstr_ulog) */



