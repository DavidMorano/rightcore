/* cfbbdate */

/* convert a digit string representing a date to its UNIX time value */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 1995-08-01, David A­D­ Morano
        This routine also ignores white space at the front or back of the digit
        string and handles a minus sign.

	= 2000-01-18, David A­D­ Morano
	Hacked to handle Y2K things a little better.

*/

/* Copyright © 1995,1998,2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	CENTURY_BASE
#define	CENTURY_BASE	19
#endif


/* external subroutines */


/* exported subroutines */


/* convert from a date-like or decimal digit time string */
int cfbbdate(PROGINFO *pip,cchar *s,int slen,time_t *rp)
{
	struct tm	ts ;
	time_t		t ;
	int		rs = SR_OK ;
	int		i, lr ;
	int		century, year, month, day, hour, min, sec ;
	int		ch ;
	const char	*cp = s ;
	char		name[DATE_TZNAMESIZE + 1] ;

	if (rp != NULL)
	    *rp = 0 ;

	if (s == NULL)
	    return -1 ;

	if (slen <= 0)
	    return -1 ;

#if	CF_DEBUGS
	debugprintf("cfbbdate: string=%t\n",s,slen) ;
#endif

	name[0] = '\0' ;

/* skip leading white space */

	for (i = 0 ; (i < slen) && CHAR_ISWHITE(*cp) ; i += 1) {
	    cp += 1 ;
	}

	lr = slen - i ;

#if	CF_DEBUGS
	debugprintf("cfbbdate: stripped string=%t\n",cp,lr) ;
#endif

	century = -1 ;
	month = 0 ;
	day = 1 ;
	hour = 0 ;
	min = 0 ;
	sec = 0 ;

/* get year */

	if (lr < 2)
	    return -2 ;

/* special handling for 4 digit year */

	for (i = 0 ; i < lr ; i += 1) {
	    ch = MKCHAR(cp[i]) ;
	    if (! isdigitlatin(ch)) break ;
	} /* end for */

	if (i >= 14) {
	    century = 10 * (*cp++ - '0') ;
	    century += (*cp++ - '0') ;
	}

	year = 10 * (*cp++ - '0') ;
	year += (*cp++ - '0') ;
	if (century < 0) {

	    if (year < 70)
	        year += 100 ;

	} else {
	    year += ((century - CENTURY_BASE) * 100) ;
	}

	lr -= 2 ;

/* get month */

	if (lr < 2)
	    goto convert ;

	month = 10 * (*cp++ - '0') ;
	month += (*cp++ - '0') ;
	if (month > 12)
	    return -3 ;

	if (month > 0)
	    month -= 1 ;	/* 'mktime' starts months from '0' */

	lr -= 2 ;

/* get day */

	if (lr < 2)
	    goto convert ;

	day = 10 * (*cp++ - '0') ;
	day += (*cp++ - '0') ;
	if ((day < 1) || (day > 31))
	    return -4 ;

	lr -= 2 ;

/* get hours */

	if (lr < 2)
	    goto convert ;

	hour = 10 * (*cp++ - '0') ;
	hour += (*cp++ - '0') ;
	if (hour >= 24)
	    return -5 ;

	lr -= 2 ;

/* get minutes */

	if (lr < 2)
	    goto convert ;

	min = 10 * (*cp++ - '0') ;
	min += (*cp++ - '0') ;
	if (min >= 60)
	    return -6 ;

	lr -= 2 ;

/* get seconds */

	if (lr < 2)
	    goto convert ;

	sec = 10 * (*cp++ - '0') ;
	sec += (*cp++ - '0') ;
	if (sec > 61)
	    return -7 ;

	lr -= 2 ;

/* get possible timezone name */

	if (lr < 1)
	    goto convert ;

	for (i = 0 ; lr && isalpha(*cp) && (i < DATE_TZNAMESIZE) ; i += 1) {
		name[i] = *cp++ ;
		lr -= 1 ;
	} /* end for */

	name[i] = '\0' ;

#if	CF_DEBUGS
	debugprintf("cfbbdate: name=%s\n",name) ;
#endif

/* do it and see what happens */
convert:
	ts.tm_year = year ;
	ts.tm_mon = month ;
	ts.tm_mday = day ;
	ts.tm_hour = hour ;
	ts.tm_min = min ;
	ts.tm_sec = sec ;
	ts.tm_isdst = -1 ;

	    if (name[0] != '\0') {
	        DATE	d ;

#if	CF_DEBUGS
	debugprintf("cfbbdate: converting w/ name=%s\n",name) ;
#endif

	        rs = dater_startsplitname(&d,&ts,name,i) ;

	        if (rs >= 0) {

	            rs = dater_gettime(&d,&t) ;

	            dater_finish(&d) ;

	        }

	    } else
	        rs = uc_mktime(&ts,&t) ;

	if (rp != NULL)
		*rp = t ;

#if	CF_DEBUGS
	debugprintf("cfbbdate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cfbbdate) */


