/* timest_nist */

/* convert UNIX® time into a Julian like character string */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGTT	0		/* debug-test 'tt' */


/* revision history:

	= 1998-08-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little subroutine formats a time string that is close to what NIST
	itself uses for the 'daytime' Internet service.

	We are currently not exact with this string format.  The NIST format
	(that has its own RFC) contains information that we do not easily have
	available to us.  Everything except for the time itself is not really
	available to us at the present (we being too lazy to really keep it all
	and track it all), but we do try to do something with
	dalight-savings-time in the string format.  We put out the 50' code for
	daylight-savings time and '00' for standard time.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"nistinfo.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY		100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN		80
#endif


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;
extern int	getmjd(int,int,int) ;


/* local structures */


/* forward subroutines */


/* local variables */


/* exported subroutines */


char *timestr_nist(time_t t,struct nistinfo *nip,char *tbuf)
{
	struct tm	tsz, *tszp = &tsz ;
	struct tm	tsl, *tslp = &tsl ;
	const int	tlen = TIMEBUFLEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		mjd ;
	int		tt ;
	int		adv_int, adv_fra ;
	int		ocl = -1 ;
	const char	*ocp = "DAM" ;
	const char	*fmt ;

	if (nip == NULL) rs = SR_FAULT ;
	if (tbuf == NULL) rs = SR_FAULT ;

	if (t == 0) t = time(NULL) ;

	if (rs >= 0) rs = uc_gmtime(&t,tszp) ;

	if (rs >= 0) rs = uc_localtime(&t,tslp) ;

	if (rs >= 0) {

#if	CF_DEBUGS
	debugprintf("timestr_nist: y=%u m=%u d=%u\n",
		tszp->tm_year,tszp->tm_mon,tszp->tm_mday) ;
#endif

	mjd = getmjd(tszp->tm_year,tszp->tm_mon,tszp->tm_mday) ;

#if	CF_DEBUGS
	debugprintf("timestr_nist: getmjd() rs=%d\n",mjd) ;
#endif

	adv_int = (nip->adv / 10) ;
	adv_fra = (nip->adv % 10) ;

#if	CF_DEBUGTT
	tt = 1 ;
#else
	tt = nip->tt ;
	if (tt == 0) {
	    if (tslp->tm_isdst) tt = 50 ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("timestr_nist: tt=%d\n",tt) ;
#endif

	if (nip->org[0] != '\0') {
	    ocp = nip->org ;
	    ocl = strnlen(nip->org,NISTINFO_ORGSIZE) ;
	}

	fmt = "%05u %02u-%02u-%02u %02u:%02u:%02u" 
		" %02u %u %1u %03u.%01u UTC(%t) *",
	rs = bufprintf(tbuf,tlen,fmt,
	    mjd,
	    (tszp->tm_year % NYEARS_CENTURY),
	    (tszp->tm_mon + 1),
	    tszp->tm_mday,
	    tszp->tm_hour,
	    tszp->tm_min,
	    tszp->tm_sec,
	    tt,
	    nip->l,
	    nip->h,
	    adv_int,adv_fra,
	    ocp,ocl) ;

	} /* end if (ok) */

	return (rs >= 0) ? tbuf : NULL ;
}
/* end subroutine (timest_nist) */


