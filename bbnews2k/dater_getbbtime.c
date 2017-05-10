/* dater_getbbtime */

/* extension to DATER object to parse BBNEWS time-stamps */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-03-01, David A­D­ Morano

        We collect the code that accesses the user currency file pretty much
        into one place. The functions handled by this module were previously
        scattered around in the past!

	= 1998-11-13, David A­D­ Morano

        This is enhanced from the older version of the same (that I wrote back
        in the early 90s).


*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<dater.h>
#include	<localmisc.h>


/* local object defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	DATE1970
#define	DATE1970	(24 * 3600)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	siskipwhite(const char *,int) ;
extern int	cfdecul(const char *,int,ulong *) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dater_getbbtime(DATER *dp,cchar sp[],int sl,time_t *tp)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {

	    if ((cl == 1) && (cp[0] == '0')) {
	        *tp = 0 ;
	    } else if ((rs = dater_setstrdig(dp,cp,cl)) >= 0) {
	        dater_gettime(dp,tp) ;
	    } else if (rs == SR_INVALID) {
	        ulong	ulw ;

#if	CF_DEBUGS
	        debugprintf("bbnewsrc/dater_getbbtime: "
		    "dater_setstrdig() rs=%d\n",rs) ;
#endif

	        if ((rs = cfdecul(cp,cl,&ulw)) >= 0) {
	            *tp = (time_t) ulw ;
	        }

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	{
	    char	tbuf[TIMEBUFLEN + 1] ;
	    timestr_log(*tp,tbuf) ;
	    debugprintf("bbnewsrc/dater_getbbtime: ret rs=%d mtime=%s\n",
	        rs,tbuf) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (dater_getbbtime) */


