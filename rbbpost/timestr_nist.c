/* timest_nist */

/* convert UNIX® time into a Julian like character string */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
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
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"nistinfo.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	ORG
#define	ORG		"RNS" ;
#endif

#define	NDF		"timestr.nd"


/* external subroutines */

extern int	bufprintf(char *,int,cchar *,...) ;
extern int	getmjd(int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif


/* local structures */


/* forward subroutines */


/* local variables */


/* exported subroutines */


char *timestr_nist(time_t t,struct nistinfo *nip,char *tbuf)
{
	struct tm	tsz, *tszp = &tsz ;
	struct tm	tsl, *tslp = &tsl ;
	const int	tlen = NISTINFO_BUFLEN ;
	int		rs ;

#if	CF_DEBUGN
	nprintf(NDF,"timestr_nist: ent\n") ;
#endif

	if (nip == NULL) rs = SR_FAULT ;
	if (tbuf == NULL) rs = SR_FAULT ;

	if (t == 0) t = time(NULL) ;

	tbuf[0] = '\0' ;
	if ((rs = uc_gmtime(&t,tszp)) >= 0) {
	    if ((rs = uc_localtime(&t,tslp)) >= 0) {
	        const int	y = tszp->tm_year ;
	        const int	m = tszp->tm_mon ;
	        const int	d = tszp->tm_mday ;

#if	CF_DEBUGS
	        debugprintf("timestr_nist: y=%u m=%u d=%u\n",y,m,d) ;
#endif

	        if ((rs = getmjd(y,m,d)) >= 0) {
	            const int	mjd = rs ;
	            const int	adv_int = (nip->adv / 10) ;
	            const int	adv_fra = (nip->adv % 10) ;
		    const int	olen = NISTINFO_ORGLEN ;
	            int		tt = nip->tt ;
	            cchar	*deforg = ORG ;
	            cchar	*fmt ;

#if	CF_DEBUGS
	            debugprintf("timestr_nist: getmjd() rs=%d\n",mjd) ;
#endif

	            if (tt == 0) {
	                if (tslp->tm_isdst) tt = 50 ;
	            }

#if	CF_DEBUGS
	            debugprintf("timestr_nist: tt=%d\n",tt) ;
#endif

#if	CF_DEBUGN
	            nprintf(NDF,"timestr_nist: org=%s\n",nip->org) ;
#endif

	            if (nip->org[0] != '\0') {
			const int	ol = strlen(nip->org) ;
			if (ol > olen) {
			    nip->org[olen] = '\0' ;
			}
		    } else {
			strdcpy1(nip->org,olen,deforg) ;
	            }

#if	CF_DEBUGN
	            nprintf(NDF,"timestr_nist: org{%p}=%s\n",
			nip->org,nip->org) ;
#endif

	            fmt = "%05u %02u-%02u-%02u %02u:%02u:%02u"
	                " %02u %u %1u %03u.%01u UTC(%s) *" ;

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
	                nip->org) ;

#if	CF_DEBUGN
	            nprintf(NDF,"timestr_nist: org=>%s<\n",nip->org) ;
	            nprintf(NDF,"timestr_nist: tbuf=>%s<\n",tbuf) ;
#endif

	        } /* end if (getmjd) */

	    } /* end if (uc_getlocaltime) */
	} /* end if (uc_gmtime) */

	return (rs >= 0) ? tbuf : NULL ;
}
/* end subroutine (timest_nist) */


