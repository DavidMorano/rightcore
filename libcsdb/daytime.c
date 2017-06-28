/* daytime */

/* general time object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-01, David A­D­ Morano
	Originally created due to frustration with various other "fuzzy" time
	convertion subroutines.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
	This object can be used to create times from various input data,
	including strings.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<tzfile.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<sbuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"daytime.h"


/* local defines */


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;


/* external variables */


/* local (static) variables */


/* exported subroutines */


int daytime_start(DAYTIME *tp)
{

	if (tp == NULL) return SR_FAULT ;

	*tp = 0 ;
	return SR_OK ;
}
/* end subroutine (daytime_start) */


int daytime_finish(DAYTIME *tp)
{

	if (tp == NULL) return SR_FAULT ;

	*tp = 0 ;
	return SR_OK ;
}
/* end subroutine (daytime_finish) */


/* convert an elapsed time string into the object */

/* 
Here we want to convert a string like:
      23 days 14:53:32
or
      23-14:53:32
or
      14:53:32
into a time.
*/

int daytime_loadelapsed(tp,sbuf,slen)
DAYTIME		*tp ;
const char	sbuf[] ;
int		slen ;
{
	int		rs = SR_OK ;
	int		days, hours, minutes, seconds ;
	int		si ;
	int		sl, cl ;
	const char	*sp ;
	const char	*cp ;

	if (tp == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	if (slen < 0)
	    slen = strlen(sbuf) ;

	if ((slen >= 0) && (slen < 18))
	    return SR_INVALID ;

	sp = sbuf ;
	sl = slen ;
	if ((si = sisub(sp,sl,"days")) >= 0) {

/* get days */

	    cl = nextfield(sp,sl,&cp) ;

	    rs = cfdeci(cp,cl,&days) ;
	    if (rs < 0) goto ret0 ;

	    sl -= (cp - sp) ;
	    sp = (cp + cl) ;

/* through away this next field but advance through source string */

	    cl = nextfield(sp,sl,&cp) ;

	    sl -= (cp - sp) ;
	    sp = (cp + cl) ;

	} else if ((si = sibreak(sp,sl,"-")) >= 0) {
	    cp = (sp+si) ;

/* get days */

	    rs = cfdeci(sp,si,&days) ;
	    if (rs < 0) goto ret0 ;

	    sl -= ((cp + 1) - sp) ;
	    sp = (cp + 1) + si ;

	} /* end if (getting days) */

/* get the hours */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if (sl < 2) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = cfdeci(sp,2,&hours) ;
	if (rs < 0) goto ret0 ;

	sl += 2 ;
	sl -= 2 ;
	if ((sl <= 0) || (*sp != ':')) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	sl += 1 ;
	sl -= 1 ;
	if (sl < 2) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}
	rs = cfdeci(sp,2,&minutes) ;
	if (rs < 0) goto ret0 ;

	sp += 2 ;
	sl -= 2 ;
	if ((sl <= 0) || (*sp != ':')) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	sp += 1 ;
	sl -= 1 ;
	if (sl < 2) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}
	rs = cfdeci(sp,2,&seconds) ;
	if (rs < 0) goto ret0 ;

	*tp = (days * 24 * 3600) ;
	*tp += (hours * 3600) ;
	*tp += (minutes * 60) ;
	*tp += seconds ;

ret0:
	return rs ;
}
/* end subroutine (daytime_loadelapsed) */


/* make up a string representing the elapsed time */

/*
We will use a field width of 6 for the days.
*/
#define	DAYTIME_DAYSWIDTH	6

int daytime_mkelapsed(tp,rbuf,rlen)
DAYTIME		*tp ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		len, i ;
	int		days, hours, minutes, seconds ;
	int		remainder ;
	char		daystr[DAYTIME_DAYSWIDTH + 2] ;

	if (tp == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

	    days = *tp / (24 * 3600) ;
	    remainder = *tp % (24 * 3600) ;

	    hours = remainder / 3600 ;
	    remainder = remainder % 3600 ;

	    minutes = remainder / 60 ;
	    seconds = remainder % 60 ;

	    rs = ctdeci(daystr,(DAYTIME_DAYSWIDTH + 1),days) ;
	    len = rs ;
	    if (rs >= 0) {

	        for (i = 0 ; i < (DAYTIME_DAYSWIDTH - len) ; i += 1)
	            len = sbuf_char(&b,' ') ;

	        sbuf_strw(&b,daystr,len) ;

	        sbuf_strw(&b," day(s) ",8) ;

	        sbuf_deci(&b,hours) ;

	        sbuf_char(&b,':') ;

	        sbuf_deci(&b,minutes) ;

	        sbuf_char(&b,':') ;

	        sbuf_deci(&b,seconds) ;

	        rs = sbuf_getlen(&b) ;

	    } /* end if */

	    sbuf_finish(&b) ;
	} /* end if */

	return rs ;
}
/* end subroutine (daytime_mkelapsed) */


