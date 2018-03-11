/* timestr_date */

/* convert UNIX® time into a various date formats */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SNTMTIME	1		/* use 'sntmtime(3dam)' */


/* revision history:

	= 1998-08-01, David A­D­ Morano
	This subroutine was originally written.

	= 2013-11-24, David A­D­ Morano
        I changed this (after all of these years) to use 'sntmtime(3dam)' rather
        than 'bufprintf(3dam)'. This should be faster than before -- at least
        that was what was expected. The 'sntmtime(3dam)' subroutine is similar
        to the 'strftime(3c)' subroutine except that it has a couple of extra
        format codes to make creating time-strings with time-zone offsets within
        them a little bit easier.

*/

/* Copyright © 1998,2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
        Return the date (in UNIX® mail evelope format) into the user's supplied
        buffer.

	The correct (newer) UNIX® mail envelope format time string is:

		Wed Jun  4 20:52:47 EDT 1997

	The old UNIX® mail envelope format time string was:

		Wed Jun  4 20:52:47 1997

	Additional note: Even newer UNIX® systems use:

		Wed Jun  4 20:52:47 EDT 1997 -0400

        The program '/usr/lib/mail.local' uses the *old* format while the newer
        program '/usr/bin/mail' uses the new format. Most PCS utilities use the
        newer (newest) format (which is (far) suprerior since it includes the
        timezone abbreviation and the time-zone offset value).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<time.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<tmtime.h>
#include	<localmisc.h>

#include	"zoffparts.h"


/* local defines */

#define	TIMESTR_TSTD		0	/* standard (and MSG envelope) */
#define	TIMESTR_TGMSTD		1	/* standard for GMT */
#define	TIMESTR_TMSG		2	/* RFC-822 message */
#define	TIMESTR_TLOG		3	/* "log" format */
#define	TIMESTR_TGMLOG		4	/* "log" format for GMT */
#define	TIMESTR_TLOGZ		5	/* "logz" format */
#define	TIMESTR_TGMLOGZ		6	/* "logz" format for GMT */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_SNTMTIME
extern int	sntmtime(char *,int,TMTIME *,const char *) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

char *timestr_date(time_t,int,char *) ;


/* local variables */

#if	CF_SNTMTIME

#else /* CF_SNTMTIME */

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
} ;

static const char	*days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
} ;

#endif /* CF_SNTMTIME */


/* exported subroutines */


char *timestr_std(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TSTD,buf) ;
}
/* end subroutine (timestr_std) */


char *timestr_edate(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TSTD,buf) ;
}
/* end subroutine (timestr_edate) */


char *timestr_gmtstd(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TGMSTD,buf) ;
}
/* end subroutine (timestr_gmtstd) */


char *timestr_msg(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TMSG,buf) ;
}
/* end subroutine (timestr_msg) */


char *timestr_hdate(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TMSG,buf) ;
}
/* end subroutine (timestr_hdate) */


char *timestr_log(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TLOG,buf) ;
}
/* end subroutine (timestr_log) */


#ifdef	COMMENT
char *timestr_loggm(time-t t,char *buf)
{
	return timestr_date(t,TIMESTR_TGMLOG,buf) ;
}
/* end subroutine (timestr_loggm) */
#endif /* COMMENT */


char *timestr_gmlog(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TGMLOG,buf) ;
}
/* end subroutine (timestr_gmlog) */


char *timestr_logz(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TLOGZ,buf) ;
}
/* end subroutine (timestr_logz) */


#ifdef	COMMENT
char *timestr_loggmz(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TGMLOGZ,buf) ;
}
/* end subroutine (timestr_loggmz) */
#endif /* COMMENT */


char *timestr_gmlogz(time_t t,char *buf)
{
	return timestr_date(t,TIMESTR_TGMLOGZ,buf) ;
}
/* end subroutine (timestr_gmlogz) */


/* create a date-string as specified by its type-code */
char *timestr_date(time_t t,int type,char *tbuf)
{
	TMTIME		tmt ;
	const int	tlen = TIMEBUFLEN ;
	int		rs ;
	int		f_gmt = FALSE ;

	if (tbuf == NULL) return NULL ;

	tbuf[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("timestr_date: ent type=%u\n",type) ;
#endif

	switch (type) {
	case TIMESTR_TGMSTD:
	case TIMESTR_TGMLOG:
	case TIMESTR_TGMLOGZ:
	    f_gmt = TRUE ;
	    break ;
	} /* end switch */

/* split the time into its component parts */

	if (f_gmt) {
	    rs = tmtime_gmtime(&tmt,t) ;
	} else {
	    rs = tmtime_localtime(&tmt,t) ;
	}

/* create the appropriate string based on the type-code */

	if (rs >= 0) {
	    switch (type) {

	    case TIMESTR_TSTD:
	    case TIMESTR_TGMSTD:

#if	CF_SNTMTIME
	        rs = sntmtime(tbuf,tlen,&tmt,"%a %b %d %T %Z %Y %O") ;
#else
	        {
	            ZOFFPARTS	zo ;
	            zoffparts_set(&zo,tmt.gmtoff) ;
	            rs = bufprintf(tbuf,tlen,
	                "%t %t %2u %02u:%02u:%02u %s %04u %c%02u%02u",
	                days[tmt.wday],3,
	                months[tmt.mon],3,
	                tmt.mday,
	                tmt.hour,
	                tmt.min,
	                tmt.sec,
	                tmt.zname,
	                (tmt.year + TM_YEAR_BASE),
	                ((tmt.gmtoff >= 0) ? '-' : '+'),
	                zo.hours,zo.mins) ;
	        }
#endif /* CF_SNTMTIME */

	        break ;

	    case TIMESTR_TMSG:

#if	CF_SNTMTIME
	        rs = sntmtime(tbuf,tlen,&tmt,"%d %b %Y %T %O (%Z)") ;
#else
	        {
	            ZOFFPARTS	zo ;
	            zoffparts_set(&zo,tmt.gmtoff) ;
	            rs = bufprintf(tbuf,tlen,
	                "%2u %t %4u %02u:%02u:%02u %c%02u%02u (%s)",
	                tmt.mday,
	                months[tmt.mon],3,
	                (tmt.year + TM_YEAR_BASE),
	                tmt.hour,
	                tmt.min,
	                tmt.sec,
	                ((tmt.gmtoff >= 0) ? '-' : '+'),
	                zo.hours,zo.mins,
	                tmt.zname) ;
	        }
#endif /* CF_SNTMTIME */

	        break ;

	    case TIMESTR_TLOG:
	    case TIMESTR_TGMLOG:

#if	CF_SNTMTIME
	        rs = sntmtime(tbuf,tlen,&tmt,"%y%m%d_%H%M:%S") ;
#else
	        rs = bufprintf(tbuf,tlen,
	            "%02u%02u%02u_%02u%02u:%02u",
	            (tmt.year % NYEARS_CENTURY),
	            (tmt.mon + 1),
	            tmt.mday,
	            tmt.hour,
	            tmt.min,
	            tmt.sec) ;
#endif /* CF_SNTMTIME */

	        break ;

	    case TIMESTR_TLOGZ:
	    case TIMESTR_TGMLOGZ:

#if	CF_SNTMTIME
	        rs = sntmtime(tbuf,tlen,&tmt,"%y%m%d_%H%M:%S_%Z") ;
#else
	        rs = bufprintf(tbuf,tlen,
	            "%02u%02u%02u_%02u%02u:%02u_%s",
	            (tmt.year % NYEARS_CENTURY),
	            (tmt.mon + 1),
	            tmt.mday,
	            tmt.hour,
	            tmt.min,
	            tmt.sec,
	            tmt.zname) ;
#endif /* CF_SNTMTIME */

	        break ;

	    default:
	        rs = sncpy1(tbuf,tlen,"** invalid type **") ;
	        break ;

	    } /* end switch */
	} /* end if (ok) */

	if (rs < 0)
	    tbuf[0] = '\0' ;

#if	CF_DEBUGS
	if (buf != NULL)
	    debugprintf("timestr_date: ret rs=%d tbuf=>%s<\n",rs,tbuf) ;
#endif

	return tbuf ;
}
/* end subroutine (timestr_date) */


