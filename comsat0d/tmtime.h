/* tmtime */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TMTIME_INCLUDE
#define	TMTIME_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>


#define	TMTIME			struct tmtime
#define	TMTIME_ZNAMESIZE	8
#define	TMTIME_BASEYEAR		1900


struct tmtime {
	int	sec ;		/* 0-61 (for up to two leap-seconds) */
	int	min ;		/* 0-59 */
	int	hour ;		/* 0-23 */
	int	mday ;		/* month-day (day-of-month 1-31) */
	int	mon ;		/* month 0-11 */
	int	year ;		/* number of years since 1900 */
	int	wday ;		/* week-day (day-of-week 0-6) */
	int	yday ;		/* year-day (day-of-year) */
	int	isdst ;
	int	gmtoff ;	/* offset from GMT (seconds west of GMT) */
	char	zname[TMTIME_ZNAMESIZE + 1] ;	/* TZ name abbreviation */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	tmtime_insert(TMTIME *,struct tm *) ;
extern int	tmtime_ztime(TMTIME *,int,time_t) ;
extern int	tmtime_gmtime(TMTIME *,time_t) ;
extern int	tmtime_localtime(TMTIME *,time_t) ;
extern int	tmtime_extract(TMTIME *,struct tm *) ;
extern int	tmtime_mktime(TMTIME *,time_t *) ;
extern int	tmtime_adjtime(TMTIME *,time_t *) ;

#ifdef	COMMENT
extern int	tmtime_setznoe(TMTIME *,const char *,int) ;
extern int	mktime_settimez(TMTIME *,const char *,const char *,time_t) ;
extern int	mktime_gettime(TMTIME *,const char *,time_t *) ;
#endif /* COMMENT */

#ifdef	__cplusplus
}
#endif

#endif /* TMTIME_INCLUDE */


