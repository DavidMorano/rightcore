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


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
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

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


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

	if ((rs = tmz_day(&stz,cp,cl)) >= 0) {
	    TMTIME	tmt ;

#if	CF_DEBUGS
	    debugprintf("cvtdater_load: tmz_day() rs=%d\n",rs) ;
#endif

	    if (! stz.f.year) {
	        cvtdater_daytime(cdp,NULL) ;	/* get current date */
	        rs = tmtime_localtime(&tmt,cdp->daytime) ;
	        stz.st.tm_year = tmt.year ;
	    } /* end if (getting the current year) */

	    if (rs >= 0) {
	        time_t	t ;
	        tmtime_insert(&tmt,&stz.st) ;
	        if ((rs = tmtime_mktime(&tmt,&t)) >= 0) {
	            if (dp != NULL) *dp = t ;
		}
	    } /* end if */

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


