/* getdefzinfo */

/* this is supposed to provide an OS-independent time management operation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We return some default time-zone information.


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

#include	"getdefzinfo.h"


/* local defines */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)
#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* exported subroutines */


int getdefzinfo(GETDEFZINFO *zip,int isdst)
{
	int		rs ;
	const char	*zp ;

	if (zip == NULL) return SR_FAULT ;

#if	defined(TMTIME_DARWIN) && (TMTIME_DARWIN > 0)
	{
	    zip->zoff = (tmp->tm_gmtoff / 60) ;
	    zp = tmp->tm_zone ;
	}
#else
	{
	    int	f_daylight ;
	    tzset() ;
	    f_daylight = (isdst >= 0) ? isdst : daylight ;
	    zip->zoff = (((f_daylight) ? altzone : timezone) / 60) ;
	    zp = (f_daylight) ? tzname[1] : tzname[0] ;
	}
#endif /* defined(TMTIME_DARWIN) && (TMTIME_DARWIN > 0) */

	rs = strwcpy(zip->zname,zp,GETDEFZINFO_ZNAMESIZE) - zip->zname ;

	return rs ;
}
/* end subroutine (getdefzinfo) */


