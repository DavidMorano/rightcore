/* ftime */

/* this is the intercept for the UNIX® System |ftime(3c)| call */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a version of |ftime(3c)| that is preloaded to over-ride the
	standard UNIX® system version.

	Q. Is this multi-thread safe?
	A. Since it is a knock-off of an existing UNIX® system LIBC (3c)
	   subroutine that is already multi-thread safe -- then of course
	   it is!

	Q. Is this much slower than the default system version?
	A. No, not really.

	Q. How are we smarter than the default system version?
	A. Let me count the ways:
		+ value is cached!

	Q. Why did you not also interpose something for |sysinfo(2)|?
	A. Because we did not want to.

	Q. Why are you so smart?
	A. I do not know.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/timeb.h>		/* |ftime(3c)| */
#include	<errno.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	"pretime.h"


/* local defines */

#define	NDF	"libpretime.nd"


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	cfhexui(const char *,int,uint *) ;
extern int	cfhexul(const char *,int,ulong *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int ftime(struct timeb *tbp)
{
	int		rs ;
	if ((rs = pretime_modtimeb(tbp)) >= 0) {
#if	CF_DEBUGN
	{
	    int	sn = pretime_serial() ;
	    nprintf(NDF,"libpretime_ftime: ent sn=%u\n",sn) ;
	}
#endif
	    rs = 0 ;
	} else {
	    errno = (-rs) ;
	    rs = -1 ;
	}
	return rs ;
}
/* end subroutine (gettimeoftime) */


