/* date */

/* general date object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        Originally created due to frustration with various other "fuzzy" date
        conversion subroutines.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
        This object can be used to create dates from various input data
        including strings.

	Note:

        The timezone offset value in 'struct timeb' is the minutes west of GMT.
        This is a positive value for timezones that are west of Greenwich. It is
        negative for timezones east of Greenwich. This is opposite from what you
        will see in email headers (for example). Our number here must be
        subtracted from GMT in order to get the local time.

	Frustration note:

        What an incredible pain this time/date handling stuff all is? This file
        doesn't even do justice to a small fraction of the real problems
        associated with data management ! The problem is that the date changes
        as time progresses. Changes are due to timezone differences and year
        leaps and leap seconds. The fact that timezone data is not a part of
        many dates only complicates matters worse as we then have to try and
        figure out a reasonable timezone when they are not supplied.


*******************************************************************************/

#define	DATE_MASTER	0

#include	<envstandards.h>	/* must be first (to configure) */

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<localmisc.h>

#include	"date.h"
#include	"tmz.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	CENTURY_BASE
#define	CENTURY_BASE	19
#endif

#define	ISSIGN(c)	(((c) == '-') || ((c) == '+'))

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strncpyuc(char *,const char *,int) ;
extern char	*strnncpy(char *,const char *,int,int) ;

extern char	*timestr_gmlog(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local (static) variables */


/* exported subroutines */


int date_start(DATE *dp,time_t t,int zoff,int isdst,const char *zbuf,int zlen)
{
	int		rs ;
	if (dp == NULL) return SR_FAULT ;
	if (zbuf == NULL) return SR_FAULT ;
	memset(dp,0,sizeof(DATE)) ;
	dp->time = (LONG) t ;
	dp->zoff = zoff ;
	dp->isdst = isdst ;
	rs = (strnncpy(dp->zname,zbuf,zlen,DATE_ZNAMESIZE) - dp->zname) ;
	return rs ;
}
/* end subroutine (date_start) */


int date_finish(DATE *dp)
{

	if (dp == NULL) return SR_FAULT ;
	dp->time = 0 ;
	return SR_OK ;
}
/* end subroutine (date_finish) */


/* copy a DATE object */
int date_copy(DATE *dp,DATE *d2p)
{

	if (dp == NULL) return SR_FAULT ;
	if (d2p == NULL) return SR_FAULT ;
	memcpy(dp,d2p,sizeof(DATE)) ;
	return SR_OK ;
}
/* end subroutine (date_copy) */


int date_gettime(DATE *dp,time_t *tp)
{

	if (dp == NULL) return SR_FAULT ;
	if (tp == NULL) return SR_FAULT ;
	*tp = (time_t) dp->time ;
	return SR_OK ;
}
/* end subroutine (date_gettime) */


int date_getzoff(DATE *dp,int *zop)
{

	if (dp == NULL) return SR_FAULT ;
	if (zop == NULL) return SR_FAULT ;
	*zop = (time_t) dp->zoff ;
	return SR_OK ;
}
/* end subroutine (date_getzoff) */


int date_getisdst(DATE *dp,int *dstp)
{

	if (dp == NULL) return SR_FAULT ;
	if (dstp == NULL) return SR_FAULT ;
	*dstp = (time_t) dp->isdst ;
	return SR_OK ;
}
/* end subroutine (date_getisdst) */


int date_getzname(DATE *dp,char *zbuf,int zlen)
{
	int		rs ;
	if (dp == NULL) return SR_FAULT ;
	if (zbuf == NULL) return SR_FAULT ;
	rs = snwcpy(zbuf,zlen,dp->zname,DATE_ZNAMESIZE) ;
	return rs ;
}
/* end subroutine (date_getzname) */


