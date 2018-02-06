/* getmjd */

/* get the Modified Julian Day (MJD) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Introduction:

	Q. What is a Modified-Julian-Day (MJD)?
	A. Is the the number of days since midnight (morning) of 17 Nov 1858.

	Q. How are Modified-Julian-Days related to Julian-Days?
        A. The base date for Modified-Julian-Days (17 Nov 1858) is 2400000.5
        days after the start of day zero (0) of the Julian calendar.

	Validity of thse subroutines:

        These subroutines are only valid for years starting from 1900.


	Name:

	getmjd (Get Modified Julian Day)

	Description:

	This subroutine calculates the Modified Julian Day (MJD) from the
	current: year, month, day.  The year may be specified as (for example)
	90 or 1990.

	Synopsis:

	int getmjd(int yr,int mo,int day)

	Arguments:

	yr		year (ex: 1998)
	mo		month (0-11, Jan == 0, Dec == 11)
	day		day-of-month (1-31)

	Returns:

	mjd		modified-julian-day


	Name:

	getyrd (Get Year-Day)

	Description:

	This subroutine calculates the Day-of-the-Year (YRD) from the current:
	year, month, day.  The year may be specified as (for example) 90 or
	1990.

	Synopsis:

	int getyrd(int yr,int mo,int day)

	Arguments:

	yr		year (ex: 1998)
	mo		month (0-11, Jan == 0, Dec == 11)
	day		day-of-month (1-31)

	Returns:

	yrd		day-of-year


	Notes:

	= Remember to account for leap-year:

define	isleap(y) ((((y) % 4) == 0) && (((y) % 100) != 0 || ((y) % 400) == 0))

	= Notes:

	Q. Do you check for all errors in the input specification?
	A. No.  We do not check that the proper number of days for
	the specified month is given.  For example, a specification
	(y-m-d) of "feb31" will be accepted (not flagged as as error).
	Yet, We do check for some domain errors (see the code below).

	Q. Could we have checked for the proper number of days for any
	given month?
	A. Yes, of course; But we felt that it was not so necessary.

	Q. What if I want the number of days for any given month to
	be flagged as an error (as if it is a domain error)?
	A. Of course you can check that yourself before calling this
	subroutine!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	isleapyear(int) ;


/* local structures */

enum mons {
	mon_jan,
	mon_feb,
	mon_mar,
	mon_apr,
	mon_may,
	mon_jun,
	mon_jul,
	mon_aug,
	mon_sep,
	mon_oct,
	mon_nov,
	mon_dec,
	mon_overlast
} ;


/* forward references */


/* local variables */

/* Days-Of-Year (DOY) */
static const int doy[12] = { 0,31,59,90,120,151,181,212,243,273,304,334 } ;


/* exported subroutines */


int getmjd(int yr,int mo,int day)
{
	const int	yrbase = TM_YEAR_BASE ; /* TZFILE 1900 */
	int		mjd = 15020 ;	/* MJD of 1900-01-01 */
	int		nlyears ;	/* leap years from 1900 */
	int		myr ;		/* modified-year (years since 1900) */

	if (yr < 0)
	    return SR_DOM ;

	if ((mo < mon_jan) || (mo >= mon_overlast))
	    return SR_DOM ;

	if ((day < 1) || (day > 31))
	    return SR_DOM ;

/* adjust year as needed (for convenience to caller) */

	if (yr < yrbase) yr += yrbase ;

/* continue to calculate MHD */

	myr = (yr - yrbase) ;		/* get modified year */

	nlyears = (myr - 1)/4 ;		/* number of leap years since 1900 */

/*
	compute number of days since 1900 
	+ 1 day for each leap year
	+ number of days since start of this year
*/

	mjd += ((365*myr) + nlyears + doy[mo] + (day - 1)) ;

/*
	If the current month is March or later, then we must add 1 day if
	the current year is a leap year.
*/

	if ((mo >= mon_mar) && isleapyear(yr))
	    mjd += 1 ;

	return mjd ;
}
/* end subroutine (getmjd) */


int getyrd(int yr,int mo,int day)
{
	const int	yrbase = TM_YEAR_BASE ;
	int		yday = 0 ;

	if (yr < 0)
	    return SR_DOM ;

	if ((mo < mon_jan) || (mo >= mon_overlast))
	    return SR_DOM ;

	if ((day < 1) || (day > 31))
	    return SR_DOM ;

/* calculate */

	if (yr < yrbase) yr += yrbase ;

	yday += (doy[mo] + (day - 1)) ;

/*
	If the current month is March or later, then we must add 1 day if
	the current year is a leap year.
*/

	if ((mo >= mon_mar) && isleapyear(yr))
	    yday += 1 ;

	return yday ;
}
/* end subroutine (getyrd) */


