/* tmtime */

/* this is supposed to provide OS-independent time management operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code provides an interface to some time mangement functions that
	hides some of the (stupid) gory details of the internal UNIX® time-zone
	managemtnt system.

	Implementation notes:

        Calling 'ftime(3c)' sets the local time-zone information (through) a
        secret call to (as you know) 'tzset(3c)'.

        Note also that the Darwin OS (used on Macs as the core of MacOS) does
        not maintain the normal external variables that are set by 'tzset(3c)'
        as previous, more traditional, OSes did. This is a positive development
        and one that should have been in there from the beginning, but provision
        has to made for it none-the-less.

        Finally, note that SlowLaris has a 'define' bug in that it does not
        declare the 'altzone' variable unless some other defines are made (see
        the code). It is not clear if and when this will be or has been fixed.
        This subroutine does not currently use the 'altzone' variable anyway.


*******************************************************************************/


#define	TMTIME_MASTER		0

#undef	TMTIME_DARWIN
#define	TMTIME_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))

#undef	TMTIME_SUNOS
#define	TMTIME_SUNOS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"tmtime.h"


/* local defines */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)
#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

int		tmtime_gmtime(TMTIME *,time_t) ;
int		tmtime_gmtime(TMTIME *,time_t) ;
int		tmtime_insert(TMTIME *,struct tm *) ;
int		tmtime_extract(TMTIME *,struct tm *) ;

static int	tmtime_mktimer(TMTIME *,int,time_t *) ;


/* exported subroutines */


int tmtime_ztime(TMTIME *op,int z,time_t t)
{
	int		rs ;
	if (z) {
	    rs = tmtime_localtime(op,t) ;
	} else {
	    rs = tmtime_gmtime(op,t) ;
	}
	return rs ;
}
/* end subroutine (tmtime_ztime) */


/* write of TMTIME object */
int tmtime_gmtime(TMTIME *op,time_t t)
{
	struct tm	tms ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (t == 0) t = time(NULL) ;

	memset(op,0,sizeof(TMTIME)) ;

	if ((rs = uc_gmtime(&t,&tms)) >= 0) {
	    if ((rs = tmtime_insert(op,&tms)) >= 0) {
	        op->gmtoff = 0 ;
	        rs = strwcpy(op->zname,"GMT",TMTIME_ZNAMESIZE) - op->zname ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (tmtime_gmtime) */


/* write of object TMTIME */
int tmtime_localtime(TMTIME *op,time_t t)
{
	struct tm	tms ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (t == 0) t = time(NULL) ;

	memset(op,0,sizeof(TMTIME)) ;

	if ((rs = uc_localtime(&t,&tms)) >= 0) {
	    rs = tmtime_insert(op,&tms) ;
	}

	return rs ;
}
/* end subroutine (tmtime_localtime) */


int tmtime_insert(TMTIME *op,struct tm *tmp)
{
	struct tm	tc ;
	int		rs = SR_OK ;
	const char	*zp ;

	if ((op == NULL) || (tmp == NULL)) return SR_FAULT ;

	op->gmtoff = 0 ;
	op->sec = tmp->tm_sec ;
	op->min = tmp->tm_min ;
	op->hour = tmp->tm_hour ;
	op->mday = tmp->tm_mday ;
	op->mon = tmp->tm_mon ;
	op->year = tmp->tm_year ;
	op->wday = tmp->tm_wday ;
	op->yday = tmp->tm_yday ;
	op->isdst = tmp->tm_isdst ;

	tc = *tmp ;
	if (tmp->tm_isdst < 0) {
	    time_t	t ; /* dummy */
	    rs = uc_mktime(&tc,&t) ;
	} /* end if (need DST indicator) */

	if (rs >= 0) {

#if	defined(TMTIME_DARWIN) && (TMTIME_DARWIN > 0)
	    {
	        op->gmtoff = tc.tm_gmtoff ;
	        zp = tc.tm_zone ;
	    }
#else
	    {
	        int	f_isdst = (tc.tm_isdst > 0) ;
	        op->gmtoff = (f_isdst) ? altzone : timezone ;
	        zp = (f_isdst) ? tzname[1] : tzname[0] ;
	    }
#endif /* defined(TMTIME_DARWIN) && (TMTIME_DARWIN > 0) */

	    rs = strwcpy(op->zname,zp,TMTIME_ZNAMESIZE) - op->zname ;

	} /* end if (getting zone-name) */

	return rs ;
}
/* end subroutine (tmtime_insert) */


/* read of object TMTIME */
int tmtime_extract(TMTIME *op,struct tm *tmp)
{

	if ((op == NULL) || (tmp == NULL)) return SR_FAULT ;

	memset(tmp,0,sizeof(struct tm)) ;

	tmp->tm_sec = op->sec ;
	tmp->tm_min = op->min ;
	tmp->tm_hour = op->hour ;
	tmp->tm_mday = op->mday ;
	tmp->tm_mon = op->mon ;
	tmp->tm_year = op->year ;
	tmp->tm_wday = op->wday ;
	tmp->tm_yday = op->yday ;
	tmp->tm_isdst = op->isdst ;

#if	defined(TMTIME_DARWIN) && (TMTIME_DARWIN > 0)
	tmp->tm_gmtoff = op->gmtoff ;
	tmp->tm_zone = op->zname ;
#endif

	return 0 ;
}
/* end subroutine (tmtime_extract) */


/* read of object TMTIME */
int tmtime_mktime(TMTIME *op,time_t *tp)
{

	return tmtime_mktimer(op,0,tp) ;
}
/* end subroutine (tmtime_mktime) */


/* read-write of object TMTIME */
int tmtime_adjtime(TMTIME *op,time_t *tp)
{

	return tmtime_mktimer(op,1,tp) ;
}
/* end subroutine (tmtime_adjtime) */


/* local subroutine */


/* read-write of object TMTIME */
int tmtime_mktimer(TMTIME *op,int f_adj,time_t *tp)
{
	struct tm	tms ;
	time_t		t = 0 ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("tmtime_mktime: ent f_adj=%u\n",f_adj) ;
#endif

	if (op == NULL) return SR_FAULT ;

	tmtime_extract(op,&tms) ;

	if ((rs = uc_mktime(&tms,&t)) >= 0) {
	    int	f_isdst = (tms.tm_isdst > 0) ;
	    int	taroff = op->gmtoff ;
	    int locoff ;
	    locoff = (f_isdst) ? altzone : timezone ;
	    t += (taroff - locoff) ;
	    if (f_adj) {
	        op->sec = tms.tm_sec ;
	        op->min = tms.tm_min ;
	        op->hour = tms.tm_hour ;
	        op->mday = tms.tm_mday ;
	        op->mon = tms.tm_mon ;
	        op->year = tms.tm_year ;
	        op->wday = tms.tm_wday ;
	        op->yday = tms.tm_yday ;
	        op->isdst = tms.tm_isdst ;
	    }
	} /* end if (uc_mktime) */

	if (tp != NULL) {
	    *tp = (rs >= 0) ? t : 0 ;
	}

#if	CF_DEBUGS
	debugprintf("tmtime_mktime: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tmtime_mktimer) */


