/* getlogfac */

/* get a log (SYSLOG) facility number by name */


/* revision history:

	= 1999-10-14, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines allow for lookup of either a SYSLOG facilty name or a
	facility number (returning a name-string).

	The database is not dynamic, but so far it does not have to be.

	Synopsis:

	int getlogfac(np,nl)
	const char	np[] ;
	int		nl ;

	Arguments:

	np		name of the SYSLOG facility name lookup
	nl		length of name

	Returns:

	>=0		resulting SYSLOG facility number
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/syslog.h>		/* for LOG_XXX numbers */
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	FACNAMELEN
#define	FACNAMELEN	12
#endif

#ifndef	LOCFAC
#define	LOCFAC		struct logfac
#endif


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;

extern char	*strwcpylc(char *,const char *,int) ;


/* local structures */

struct logfac {
	const char	*name ;
	int		fac ;
} ;


/* local variables */

static const struct logfac	logfacs[] = {
	{ "kernel", LOG_KERN },
	{ "user", LOG_USER },
	{ "mail", LOG_MAIL },
	{ "daemon", LOG_DAEMON },
	{ "auth", LOG_AUTH },
	{ "syslog", LOG_SYSLOG },
	{ "lpr", LOG_LPR },
	{ "news", LOG_NEWS },
	{ "uucp", LOG_UUCP },
	{ "cron", LOG_CRON },
	{ "local0", LOG_LOCAL0 },
	{ "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 },
	{ "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 },
	{ "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 },
	{ "local7", LOG_LOCAL7 },
	{ NULL, 0 }
} ;


/* exported subroutines */


int getlogfac(const char *np,int nl)
{
	const LOCFAC	*lfs = logfacs ;
	const int	n = 2 ;
	int		i ;
	int		m ;
	int		nlen ;
	int		f = FALSE ;
	const char	*anp ;
	char		nbuf[FACNAMELEN + 1] ;

	if ((nl < 0) || (nl > FACNAMELEN)) nl = FACNAMELEN ;

	nlen = strwcpylc(nbuf,np,nl) - nbuf ;

	for (i = 0 ; lfs[i].name != NULL ; i += 1) {

	    anp = lfs[i].name ;
	    m = nleadstr(anp,nbuf,nlen) ;

	    f = ((m == nlen) && ((m >= n) || (anp[m] == '\0'))) ;
	    if (f) break ;

	} /* end for */

	return (f) ? lfs[i].fac : SR_NOTFOUND ;
}
/* end subroutine (getlogfac) */


const char *strfacname(int fac)
{
	const LOCFAC	*lfs = logfacs ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; lfs[i].name != NULL ; i += 1) {
	    f = (lfs[i].fac == fac) ;
	    if (f) break ;
	} /* end for */

	return (f) ? lfs[i].name : NULL ;
}
/* end subroutine (strfacname) */


