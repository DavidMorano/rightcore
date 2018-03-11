/* cvtdater */

/* date conversion object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-11-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This small object assists in converting text strings representing dates
        into UNIX® time values. The hard work is actually done by the included
        TMZ object.

	Source formats include:

	  YYMMDD
	CCYYMMDD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<dayspec.h>
#include	<tmz.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"cvtdater.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	hasalpha(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* local subroutines */


/* forward references */

static int	cvtdater_daytime(CVTDATER *,time_t *) ;


/* local variables */


/* exported subroutines */


int cvtdater_start(CVTDATER *cdp,time_t daytime)
{

	if (cdp == NULL) return SR_FAULT ;

	memset(cdp,0,sizeof(CVTDATER)) ;
	cdp->daytime = daytime ;

	return SR_OK ;
}
/* end subroutine (cvtdater_start) */


int cvtdater_load(CVTDATER *cdp,time_t *dp,cchar *cp,int cl)
{
	TMZ		stz ;
	int		rs ;

	if (cdp == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("cvtdater_load: ent s=>%t<\n",cp,cl) ;
#endif

	if (hasalpha(cp,cl)) {
	    DAYSPEC	ds ;
	    tmz_init(&stz) ;
	    if ((rs = dayspec_load(&ds,cp,cl)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("cvtdater_load: y=%d m=%d d=%d\n",ds.y,ds.m,ds.d) ;
#endif
		rs = tmz_setday(&stz,ds.y,ds.m,ds.d) ;
#if	CF_DEBUGS
	        debugprintf("cvtdater_load: tmz_setday() rs=%d\n",rs) ;
	        debugprintf("cvtdater_load: y=%d m=%d d=%d\n",
		      stz.st.tm_year,stz.st.tm_mon,stz.st.tm_mday) ;
#endif
	    }
#if	CF_DEBUGS
	debugprintf("cvtdater_load: alpha rs=%d \n",rs) ;
#endif
	} else {
	    rs = tmz_day(&stz,cp,cl) ;
#if	CF_DEBUGS
	debugprintf("cvtdater_load: tmz_day() rs=%d \n",rs) ;
#endif
	}
	if (rs >= 0) {
	    TMTIME	tmt ;

#if	CF_DEBUGS
	    debugprintf("cvtdater_load: mid1 rs=%d\n",rs) ;
#endif

	    if (tmz_hasyear(&stz) == 0) {
	        cvtdater_daytime(cdp,NULL) ;	/* get current date */
	        rs = tmtime_localtime(&tmt,cdp->daytime) ;
		tmz_setyear(&stz,tmt.year) ;
	    } /* end if (getting the current year) */

#if	CF_DEBUGS
	    debugprintf("cvtdater_load: mid2 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        time_t	t ;
	        tmtime_insert(&tmt,&stz.st) ;
	        if ((rs = tmtime_mktime(&tmt,&t)) >= 0) {
	            if (dp != NULL) *dp = t ;
		}
	    } /* end if (ok) */

#if	CF_DEBUGS
	    debugprintf("cvtdater_load: mid3 rs=%d\n",rs) ;
#endif

	} /* end if (tmz_day) */

#if	CF_DEBUGS
	debugprintf("cvtdater_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cvtdater_load) */


int cvtdater_finish(CVTDATER *cdp)
{

	if (cdp == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (cvtdater_finish) */


/* private subroutines */


static int cvtdater_daytime(CVTDATER *cdp,time_t *rp)
{

	if (cdp == NULL) return SR_FAULT ;

	if (cdp->daytime == 0)
	    cdp->daytime = time(NULL) ;

	if (rp != NULL)
	    *rp = cdp->daytime ;

	return SR_OK ;
}
/* end subroutine (cvtdater_daytime) */


