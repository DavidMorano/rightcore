/* dayofmonth */

/* day-of-month operations (determinations) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* normal safety */
#define	CF_ALTADV	1		/* alternative advance */
#define	CF_ISLEAPYEAR	1		/* use 'isleapyear(3dam)' */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements a day-of-month calculator.


*******************************************************************************/


#define	DAYOFMONTH_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"dayofmonth.h"


/* local defines */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	ctdecpi(char *,int,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	isdigitlatin(int) ;
extern int	isleapyear(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */

enum wdays {
	wday_sunday,
	wday_monday,
	wday_tuesday,
	wday_wednesday,
	wday_thursday,
	wday_friday,
	wday_saturday,
	wday_overlast
} ;

enum odays {
	oday_first,
	oday_second,
	oday_third,
	oday_fourth,
	oday_fifth,
	oday_last,
	oday_overlast
} ;


/* forward references */

static int	dayofmonth_mkmonth(DAYOFMONTH *,int) ;


/* exported variables */


/* local variables */

static const int	daysmonth[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0
} ;

enum months {
	month_january,
	month_february,
	month_march,
	month_april,
	month_may,
	month_june,
	month_july,
	month_august,
	month_september,
	month_october,
	month_november,
	month_december,
	month_overlast
} ;


/* exported subroutines */


int dayofmonth_start(DAYOFMONTH *op,int year)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif

	memset(op,0,sizeof(DAYOFMONTH)) ;

	if (year >= 0) {
	    if ((year < 1970) || (year > 2038)) rs = SR_INVALID ;
	}

	if (rs >= 0) {
	    TMTIME	ts ;
	    time_t	t = time(NULL) ;
	    if ((rs = tmtime_localtime(&ts,t)) >= 0) {
	        op->isdst = ts.isdst ;
	        op->gmtoff = ts.gmtoff ;
	        op->year = (year >= 0) ? year : (ts.year + TM_YEAR_BASE) ;
	        op->magic = DAYOFMONTH_MAGIC ;
	    } /* end if (tmtime_localtime) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (dayofmonth_start) */


/* free up the entire vector string data structure object */
int dayofmonth_finish(DAYOFMONTH *op)
{
	const int	n = DAYOFMONTH_NMONS ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DAYOFMONTH_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("dayofmonth_finish: ent\n") ;
#endif

	for (i = 0 ; i < n ; i += 1) {
	    if (op->months[i] != NULL) {
	        c += 1 ;
	        rs1 = uc_free(op->months[i]) ;
	        if (rs >= 0) rs = rs1 ;
	        op->months[i] = NULL ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("dayofmonth_finish: ret rs=%d c=%u\n",rs,c) ;
#endif

	op->magic = 0 ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dayofmonth_finish) */


/* get a string by its index */
int dayofmonth_lookup(DAYOFMONTH *op,int m,int wday,int oday)
{
	DAYOFMONTH_MON	*mp ;
	const int	n = DAYOFMONTH_NMONS ;
	int		rs = SR_OK ;
	int		w ;
	int		mday = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DAYOFMONTH_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("dayofmonth_lookup: m=%u wday=%u oday=%u\n",
	    m,wday,oday) ;
#endif

	if ((m < 0) || (m >= n))
	    return SR_INVALID ;

	if ((wday < wday_sunday) || (wday >= wday_overlast))
	    return SR_INVALID ;

	if ((oday < oday_first) || (oday >= oday_overlast))
	    return SR_INVALID ;

	if (op->months[m] == NULL) {
	    rs = dayofmonth_mkmonth(op,m) ;
	}

	if (rs >= 0) {
	    mp = op->months[m] ;
	    if (oday != oday_last) { /* a numbered week */
#if	CF_DEBUGS
	        debugprintf("dayofmonth_lookup: numbered week=%u\n",oday) ;
#endif
#if	CF_ALTADV
	        w = 0 ;
	        if (mp->days[w][wday] < 0) w += 1 ;
#else
	        for (w = 0 ; w < 2 ; w += 1) {
	            if (mp->days[w][wday] >= 0)
	                break ;
	        } /* end for */
#endif /* CF_ALTADV */
	        w += oday ;
	        mday = mp->days[w][wday] ;
	    } else { /* the last week */
#if	CF_DEBUGS
	        debugprintf("dayofmonth_lookup: last week\n") ;
#endif
	        for (w = 5 ; w >= 3 ; w -= 1) {
	            if (mp->days[w][wday] >= 0)
	                break ;
	        } /* end for */
	        mday = mp->days[w][wday] ;
	    } /* end if */
	    if (mday < 1)
	        rs = SR_NOTFOUND ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("dayofmonth_lookup: ret rs=%d mday=%d\n",rs,mday) ;
#endif

	return (rs >= 0) ? mday : rs ;
}
/* end subroutine (dayofmonth_lookup) */


/* private subroutines */


static int dayofmonth_mkmonth(DAYOFMONTH *op,int m)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("dayofmonth_mkmonth: m=%u\n",m) ;
	debugprintf("dayofmonth_mkmonth: year=%u\n",op->year) ;
#endif

	if (op->months[m] == NULL) {
	    DAYOFMONTH_MON	*mp ;
	    const int		size = sizeof(DAYOFMONTH_MON) ;
	    int			w, wday ;
	    int			day ;
	    int			daymax ;
	    int			f ;
#if	CF_DEBUGS
	    debugprintf("dayofmonth_mkmonth: sizeof(DAYOFMONTH_MON)=%u\n",
	        size) ;
#endif
	    if ((rs = uc_malloc(size,&mp)) >= 0) {
	        struct tm	tms ;
	        op->months[m] = mp ;
/* determine how many days are in this month (could be a leap year) */
	        daymax = daysmonth[m] ;
	        if (m == month_february) {
#if	CF_ISLEAPYEAR
	            daymax = (isleapyear(op->year)) ? 29 : 28 ;
#else /* CF_ISLEAPYEAR */
	            memset(&tms,0,sizeof(struct tm)) ;
	            tms.tm_isdst = -1 ;
	            tms.tm_year = (op->year - TM_YEAR_BASE) ;
	            tms.tm_mon = m ;
	            tms.tm_mday = 29 ;
	            rs = uc_mktime(&tms,NULL) ;
	            daymax = (tms.tm_mon == m) ? 29 : 28 ;
#endif /* CF_ISLEAPYEAR */
	        } /* end if */
/* determine the first day of the month for where to start */
	        if (rs >= 0) {
	            memset(&tms,0,sizeof(struct tm)) ;
	            tms.tm_isdst = -1 ;
	            tms.tm_year = (op->year - TM_YEAR_BASE) ;
	            tms.tm_mon = m ;
	            tms.tm_mday = 1 ;
	            if ((rs = uc_mktime(&tms,NULL)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("dayofmonth_mkmonth: uc_mktime() rs=%d\n",
				rs) ;
	                debugprintf("dayofmonth_mkmonth: tms.wday=%u\n",
	                    tms.tm_wday) ;
#endif
	                f = FALSE ;
	                day = 1 ;
	                for (w = 0 ; w < 6 ; w += 1) {
	                    for (wday = 0 ; wday < 7 ; wday += 1) {
	                        if ((! f) && (wday == tms.tm_wday)) {
	                            f = TRUE ;
				}
	                        if (f && (day <= daymax)) {
	                            mp->days[w][wday] = day++ ;
	                        } else {
	                            mp->days[w][wday] = -1 ;
	                        }
	                    } /* end for */
#if	CF_DEBUGS
	                    {
	                        int	j, di = 0 ;
	                        int	ml ;
	                        char	dbuf[100] ;
	                        for (j = 0 ; j < 7 ; j += 1) {
	                            int	v ;
	                            dbuf[di++] = ' ' ;
	                            v = mp->days[w][j] ;
	                            ml = ctdecpi((dbuf+di),10,2,v) ;
	                            di += ml ;
	                        }
	                        dbuf[di] = '\0' ;
	                        debugprintf("dayofmonth_mkmonth: "
	                            "week=%u %s\n", w,dbuf) ;
	                    }
#endif /* CF_DEBUGS */
	                } /* end for */
	            } /* end if (uc_mktime) */
	        } /* end if (ok) */
	        if (rs < 0) {
	            if (op->months[m] != NULL) {
	                uc_free(op->months[m]) ;
	                op->months[m] = NULL ;
	            }
	        }
	    } /* end if (m-a) */
	} /* end if (work needed) */

#if	CF_DEBUGS
	debugprintf("dayofmonth_mkmonth: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dayofmonth_mkmonth)  */


