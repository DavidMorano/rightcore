/* cfjulian */

/* convert a digit string (possibly a Julian date) to its UNIX time value */


/* revision history:

	= 2004-08-01, David A­D­ Morano
        This subroutine was modeled (adapated) from assembly language
        (although there isn't the shred of similarity between the two).

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is ___ ?


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>

#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	CENTURY_BASE	19


/* exported subroutines */


int cfjulian(cchar *s,int slen,time_t *rp)
{
	struct tm	ts ;
	int		i ;
	int		century, year, month, day, hour, min, sec ;
	int		lr = slen ;
	cchar		*cp = s ;

	if (slen < 0)
	    slen = strlen(s) ;

	if (slen == 0)
	    return -1 ;

/* skip leading white space */

	for (i = 0 ; (i < slen) && CHAR_ISWHITE(*cp) ; i += 1) {
	    cp += 1 ;
	}

	century = CENTURY_BASE ;
	month = 0 ;
	day = 1 ;
	hour = 0 ;
	min = 0 ;
	sec = 0 ;

/* get year */

	if (lr < 2)
	    return -2 ;

	if (lr >= 14) {
	    century = 10 * (*cp++ - '0') ;
	    century += (*cp++ - '0') ;
	}

	year = 10 * (*cp++ - '0') ;
	year += (*cp++ - '0') ;
	if (century == CENTURY_BASE) {
	    if (year < 70)
	        year += 100 ;
	} else
	    year += ((century - CENTURY_BASE) * 100) ;

	lr -= 2 ;
	if (lr < 2)
	    goto convert ;

	month = 10 * (*cp++ - '0') ;
	month += (*cp++ - '0') ;
	if (month > 12)
	    return -3 ;

	if (month > 0)
	    month -= 1 ;	/* 'mktime' start months from '0' */

	lr -= 2 ;
	if (lr < 2)
	    goto convert ;

	day = 10 * (*cp++ - '0') ;
	day += (*cp++ - '0') ;
	if ((day < 1) || (day > 31))
	    return -4 ;

	lr -= 2 ;
	if (lr < 2)
	    goto convert ;

	hour = 10 * (*cp++ - '0') ;
	hour += (*cp++ - '0') ;
	if (hour >= 24)
	    return -5 ;

	lr -= 2 ;
	if (lr < 2)
	    goto convert ;

	min = 10 * (*cp++ - '0') ;
	min += (*cp++ - '0') ;
	if (min >= 60)
	    return -6 ;

	lr -= 2 ;
	if (lr < 2)
	    goto convert ;

	sec = 10 * (*cp++ - '0') ;
	sec += (*cp++ - '0') ;
	if (sec > 61)
	    return -7 ;

convert:
	ts.tm_year = year ;
	ts.tm_mon = month ;
	ts.tm_mday = day ;
	ts.tm_hour = hour ;
	ts.tm_min = min ;
	ts.tm_sec = sec ;
	ts.tm_isdst = -1 ;
	*rp = mktime(&ts) ;

	if (*rp != ((time_t) -1))
	    return OK ;

	return -8 ;
}
/* end subroutine (cfjulian) */


