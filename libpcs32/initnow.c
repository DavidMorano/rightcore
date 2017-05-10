/* initnow */

/* initialize both TIMEB and ZNAME */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FTIME	0		/* actually call 'uc_ftime(3dam)' */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine initializes a couple of time-related variables. One of
        these variables is a TIMEB structure. The other is a time-zone string.

	Synopsis:

	int initnow(tbp,zbuf,zlen)
	struct timeb	*tbp ;
	char		zbuf[] ;
	int		zlen ;

	Arguments:

	tbp		pointer to TIMEB structure
	zbuf		zone-name (a result that is returned to caller)
	zlen		length of caller buffer to hold result

	Returns:

	>=0		OK
	<0		error


	Implementation notes:

        Calling 'ftime(3c)' sets the local time-zone information (through) a
        secret call to (as you know) 'tzset(3c)'.

        Note also that the Darwin OS (used on Macs as the core of MacOS) does
        not maintain the normal external variables that are set by 'tzset(3c)'
        as previous, more traditional, OSes did. This is a positive development
        and one that should have been in there from the beginning, but provision
        has to made for it none-the-less.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>			/* for |memset(3c)| */

#include	<vsystem.h>
#include	<tmtime.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int initnow(struct timeb *tbp,char *zbuf,int zlen)
{
	int		rs ;
	int		len = 0 ;

	if (tbp == NULL) return SR_FAULT ;

	if (zbuf != NULL) zbuf[0] = '\0' ;

	memset(tbp,0,sizeof(struct timeb)) ;

#if	CF_FTIME
	if ((rs = uc_ftime(tbp)) >= 0) {
	    if (zbuf != NULL) {
	        TMTIME		tmt ;
	        if ((rs = tmtime_localtime(&tmt,tbp->time)) >= 0) {
	                rs = sncpy1(zbuf,zlen,tmt.zname) ;
	                len = rs ;
	        } /* end if (tmtime_localtime) */
	    }
	} /* end if (uc_ftime) */
#else /* CF_FTIME */
	{
	    struct timeval	tv ;
	    if ((rs = uc_gettimeofday(&tv,NULL)) >= 0) {
	        TMTIME		tmt ;
	        tbp->time = tv.tv_sec ;
	        tbp->millitm = (tv.tv_usec / 1000) ;
	        if ((rs = tmtime_localtime(&tmt,tbp->time)) >= 0) {
	            tbp->timezone = (tmt.gmtoff / 60) ;
	            tbp->dstflag = tmt.isdst ;
	            if (zbuf != NULL) {
	                rs = sncpy1(zbuf,zlen,tmt.zname) ;
	                len = rs ;
	            }
	        } /* end if (tmtime_localtime) */
	    } /* end if (uc_gettimeofday) */
	}
#endif /* CF_FTIME */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (initnow) */


