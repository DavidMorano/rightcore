/* uc_localtime */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*timestr_logz(time_t,char *) ;


/* exported subroutines */


int uc_localtime(const time_t *tp,struct tm *tsp)
{
	struct tm	*rp ;
	int		rs = SR_OK ;

	if (tp == NULL) return SR_FAULT ;
	if (tsp == NULL) return SR_FAULT ;

	errno = 0 ;
#if	defined(SYSHAS_LOCALTIMER) && SYSHAS_LOCALTIMER
	rp = (struct tm *) localtime_r(tp,tsp) ;
	if (rp == NULL) rs = (- errno) ;
#else
	rp = localtime(tp) ;
	if (rp == NULL) rs = (- errno) ;
	if (rs >= 0)
	    memcpy(tsp,rp,sizeof(struct tm)) ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_localtime: year=%d isdst=%d\n",
		tsp->tm_year,tsp->tm_isdst) ;
	if (tsp->tm_isdst >= 0) {
	    debugprintf("uc_localtime: tzname=%s\n",
		tzname[tsp->tm_isdst]) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (uc_localtime) */


