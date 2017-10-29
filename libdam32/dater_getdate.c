/* dater_getdate */

/* get a DATE object out of a DATER object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        Originally created due to frustration with various other "fuzzy" date
        conversion subroutines.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
        This subroutine is a method of the DATER object. It creates a DATE
        object from the DATER object.


*******************************************************************************/


#define	DATER_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dater.h"
#include	"date.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	CENTURY_BASE
#define	CENTURY_BASE	19
#endif

#define	ISSIGN(c)	(((c) == '-') || ((c) == '+'))

#ifndef	TZO_EMPTY
#define	TZO_EMPTY	SHORT_MAX
#endif
#ifndef	TZO_MAXZOFF
#define	TZO_MAXZOFF	(14*60)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strncpyuc(char *,const char *,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dater_getdate(dp,dop)
DATER		*dp ;
DATE		*dop ;
{
	int		rs ;

	if (dp == NULL) return SR_FAULT ;
	if (dop == NULL) return SR_FAULT ;

	if (dp->magic != DATER_MAGIC) return SR_NOTOPEN ;

	{
	    time_t	t = dp->b.time ;
	    int		zoff = dp->b.timezone ;
	    int		isdst = dp->b.dstflag ;
	    int		zl = DATER_ZNAMESIZE ;
	    const char	*zp = dp->zname ;
	    rs = date_start(dop,t,zoff,isdst,zp,zl) ;
	}

	return rs ;
}
/* end subroutine (dater_getdate) */


