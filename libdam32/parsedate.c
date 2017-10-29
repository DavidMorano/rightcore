/* parsedate */

/* parse almost any absolute date 'getdate(3)' can (& some it can't) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-14, David A­D­ Morano
	I modified the 'prsindate' subroutine to make it "Year 2000" safe.

	= 1999-11-23, David A­D­ Morano
	I enhanced something.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This code was borrowed (salvaged) from the original version that came
        with (I think) the old NetNews-C software. The original code has been
        made a bit more portable but is otherwise almost the same as before. for
        the most part, subroutine names have remained the same with the original
        code.


******************************************************************************/


#include	<envstandards.h>

#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>
#include <time.h>
#include <tzfile.h>

#include	<mallocstuff.h>

#include "dateconv.h"
#include "datetok.h"


/* local defines */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif

#define MAXDATEFIELDS	25
#define	TIMESTRLEN	100

#ifndef	TRUE
#define	TRUE	1
#undef	FALSE
#define	FALSE	0
#endif


/* external subroutines */

extern int	parsetime() ;


/* external variables */

extern int	dtok_numparsed ;


/* forward references */

static int	prsabsdate() ;
static int	tryabsdate() ;


/* local vaiables */

static char	delims[] = "- \t\n/," ;


/* exported subroutines */


/* parse and convert absolute date in timestr (the normal interface) */
time_t parsedate(timestr, nowp)
char		timestr[] ;
struct timeb	*nowp ;
{
	struct tm	date ;

	struct timeb	then ;

	time_t		result = 0 ;

	int		rs ;
	int		len ;
	int		tz = 0 ;
	int		f_tzset = FALSE ;
	int		f_malloc = FALSE ;

	char		parsestring[TIMESTRLEN + 1], *psp = parsestring ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("parsedate: ent\n") ;
#endif

	result = (-1) ;
	if ((len = strlen(timestr)) > TIMESTRLEN) {

	    if ((psp = mallocstrw(timestr,len)) == NULL)
	        return result ;

	    f_malloc = TRUE ;

	} else
	    strcpy(psp,timestr) ;


	if (nowp == NULL) {

#if	CF_DEBUGS
	    debugprintf("parsedate: internal compensation requested\n") ;
#endif

	    nowp = &then ;

	    f_tzset = TRUE ;
	    (void) tzset() ;		/* set the external variables ! */

#if	CF_DEBUGS
	    debugprintf("parsedate: timezone=%d altzone=%d, f_dls=%d\n",
	        timezone,altzone,daylight) ;
#endif

/* set from external variable ! */

	    nowp->timezone = (short) timezone ;

	} /* end if */

	if ((rs = prsabsdate(psp, nowp, &date, &tz)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("parsedate: parsed OK, tz=%d\n",tz) ;
#endif

	    result = dateconv(&date, tz) ;

	} /* end if */

/* do we need extra processing because of a missing timezone in input? */

	if (rs > 0) {
	    struct tm	breakout, *bop = &breakout ;

#if	CF_DEBUGS
	    debugprintf("parsedate: possible extra processing\n") ;
#endif

	    if (! f_tzset)
	        (void) tzset() ;

	    if (daylight) {

#if	CF_DEBUGS
	        debugprintf("parsedate: extra processing\n") ;
#endif

#if	CF_DEBUGS
	        debugprintf("parsedate: uncorrected result=%s\n",
	            timestr_log(result,timebuf)) ;
#endif

#if	defined(SYSHAS_LOCALTIMER) && SYSHAS_LOCALTIMER
	        bop = (struct tm *) localtime_r(&result,&breakout) ;
#else
	        bop = localtime(&result) ;
#endif

	        if (bop->tm_isdst > 0) {

	            strcpy(psp,timestr) ;

	            nowp->timezone = (short) altzone ;
	            if ((rs = prsabsdate(psp, nowp, &date, &tz)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("parsedate: parsed OK, tz=%d\n",tz) ;
#endif

	                result = dateconv(&date, tz) ;

	            } /* end if */

	        } /* end if */

	    } /* end if (extra processing was required) */

	} /* end if (possible extra processing) */

#if	CF_DEBUGS
	debugprintf("parsedate: exiting result=%s\n",
	    timestr_log(result,timebuf)) ;
#endif

	if (f_malloc)
	    uc_free(psp) ;

	return result ;
}
/* end subroutine (parsedate) */


/* local subroutines */


/* just parse the absolute date in timestr and get back a broken-out date */
static int prsabsdate(timestr, nowp, tm, tzp)
char		timestr[] ;
struct timeb	*nowp ;
struct 		tm *tm ;
int		*tzp ;
{
	int		nf ;
	int		rs ;
	char		*fields[MAXDATEFIELDS + 1] ;

#if	CF_DEBUGS
	debugprintf("prsabsdate: ent\n") ;
#endif

	nf = split(timestr, fields, MAXDATEFIELDS, delims+1) ;

	if (nf > MAXDATEFIELDS)
	    return -1 ;

#if	CF_DEBUGS
	debugprintf("prsabsdate: calling 'tryabsdate'\n") ;
#endif

	if ((rs = tryabsdate(fields, nf, nowp, tm, tzp)) < 0) {
	    char	*p = timestr ;

#if	CF_DEBUGS
	    debugprintf("prsabsdate: bad date on first try\n") ;
#endif

/*
		 * could be a DEC-date; glue it all back together, split it
		 * with dash as a delimiter and try again.  Yes, this is a
		 * hack, but so are DEC-dates.
*/

/* put the spaces back into the input ! */

	    while (--nf > 0) {
	        while (*p++ != '\0') ;
	        p[-1] = ' ' ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("prsabsdate: 'split()'\n") ;
#endif

	    nf = split(timestr, fields, MAXDATEFIELDS, delims) ;

	    if (nf > MAXDATEFIELDS)
	        return -1 ;

#if	CF_DEBUGS
	    debugprintf("prsabsdate: 2 'tryabsdate()'\n") ;
#endif

	    rs = tryabsdate(fields, nf, nowp, tm, tzp) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("prsabsdate: exiting, rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (prsabsdate) */


/* try to parse pre-split timestr as an absolute date */
static int tryabsdate(fields, nf, nowp, tm, tzp)
char			*fields[] ;
int			nf ;
struct timeb		*nowp ;
struct tm		*tm ;
int			*tzp ;
{
	struct timeb	ftz ;
	datetkn		*tp ;
	long		flg = 0, ty ;
	int		i ;
	int		mer = HR24, bigval = -1 ;
	int		f_tz = FALSE ;

#if	CF_DEBUGS
	debugprintf("tryabsdate: ent\n") ;
#endif

	if (nowp == NULL) {		/* default to local time (zone) */

	    nowp = &ftz ;
	    (void) ftime(nowp) ;

#if	CF_DEBUGS
	    debugprintf("tryabsdate: loaded from 'ftime()'\n") ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("tryabsdate: past possible 'ftime()'\n") ;
#endif

/* the system has seconds, we want minutes */

	*tzp = ((int) nowp->timezone) / 60 ;

#if	CF_DEBUGS
	debugprintf("tryabsdate: initial tzp=%d\n",*tzp) ;
#endif

	tm->tm_mday = tm->tm_mon = tm->tm_year = -1 ;	/* mandatory */
	tm->tm_hour = tm->tm_min = tm->tm_sec = 0 ;
	tm->tm_isdst = 0 ;
	dtok_numparsed = 0 ;

#if	CF_DEBUGS
	debugprintf("tryabsdate: entering for\n") ;
#endif

	for (i = 0 ; i < nf ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("tryabsdate: top for\n") ;
#endif

	    if (fields[i][0] == '\0')
	        continue ;

	    tp = datetoktype(fields[i], &bigval) ;
	    ty = (1L << tp->type) & ~(1L << IGNORE) ;
	    if (flg & ty)
	        return -1;		/* repeated type */

	    flg |= ty ;
	    switch (tp->type) {

	    case YEAR:
	        tm->tm_year = bigval ;

/* convert 4-digit year to 1900 origin */
	        if (tm->tm_year >= 1900)
	            tm->tm_year -= 1900 ;

	        if (tm->tm_year < 70)
	            tm->tm_year += 100 ;

	        break ;

	    case DAY:
	        tm->tm_mday = bigval ;
	        break ;
	    case MONTH:
	        tm->tm_mon = tp->value - 1; /* convert to zero-origin */
	        break ;
	    case TIME:
	        if (parsetime(fields[i], tm) < 0)
	            return -1 ;
	        break ;

	    case DTZ:
#if 0
	        tm->tm_isdst += 1 ;
#endif

#if	CF_DEBUGS
	        debugprintf("tryabsdate: got a DTZ\n") ;
#endif

/* FALLTHROUGH */

	    case TZ:
	        f_tz = TRUE ;

#if	CF_DEBUGS
	        debugprintf("tryabsdate: got a TZ\n") ;
#endif

	        *tzp = FROMVAL(tp) ;

#if	CF_DEBUGS
	        debugprintf("tryabsdate: (D)TZ tzp=%d\n",*tzp) ;
#endif

	        break ;

	    case IGNORE:
	        break ;
	    case AMPM:
	        mer = tp->value ;
	        break ;

	    default:
	        return -1 ;	/* bad token type: CANTHAPPEN */

	    } /* end switch */

#if	CF_DEBUGS
	    debugprintf("tryabsdate: bottom for\n") ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("tryabsdate: past for\n") ;
#endif

	if (tm->tm_year == -1 || tm->tm_mon == -1 || tm->tm_mday == -1)
	    return -1;		/* missing component */

	if (mer == PM)
	    tm->tm_hour += 12 ;

#if	CF_DEBUGS
	debugprintf("tryabsdate: exiting, f_tz=%d\n",f_tz) ;
#endif

	return ((f_tz) ? 0 : 1) ;
}
/* end subroutine (tryabsdate) */


