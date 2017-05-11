/* ctut */

/* convert a 'time_t' value into a "UNIX time" character string */


#define	CF_SNTMTIME	0		/* use 'sntmtime(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This routine takes as input a long integer that is interpreted to be a
	UNIX® time value in seconds.  This routine returns a string
	representation of the UNIX® time value as:

		yymmddhhmmss

	The time zone GMT (also UTC) is assumed.  Subroutines that read this
	string understand that leading year digits less than '7' are assummed
	to refer to years after 2000 AD.  Thirteen characters are always
	returned in the string ; twelve digits and one zero valued byte.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<localmisc.h>


/* local defines */

#define	RBUFLEN		12		/* maximum size needed */


/* external subroutines */

extern int	bufprintf(char *,int,cchar *,...) ;


/* exported subroutines */


int ctut(char *rbuf,time_t t)
{
	TMTIME		vals ;
	int		rs ;

	if (rbuf == NULL)
	    return SR_FAULT ;

	if (t == 0) t = time(NULL) ;

	rs = tmtime_gmtime(&vals,t) ;

#if	CF_SNTMTIME
	if (rs >= 0) {
	    const char	*fmt = "%y%m%d%H%M%S" ;
	    rs = sntmtime(rbuf,RBUFLEN,&vals,fmt) ;
	}
#else /* CF_SNTMTIME */
	if (rs >= 0) {
	    const char	*fmt = "%02u%02u%02u%02u%02u%02u" ;
	    rs = bufprintf(rbuf,RBUFLEN,fmt,
		vals.year,
		(vals.mon+1),
		vals.mday,
		vals.hour,
		vals.min,
		vals.sec) ;
	}
#endif /* CF_SNTMTIME */

	return rs ;
}
/* end subroutine (ctut) */


