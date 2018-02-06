/* getlogpri */

/* get a log (SYSLOG) priority number by name */


/* revision history:

	= 1999-10-14, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines allow for lookup of either a SYSLOG priority name or
	a priority number (returning a name-string).

	The database is not dynamic, but so far it does not have to be.

	Synopsis:

	int getlogpri(name)
	const char	name[] ;

	Arguments:

	name		name of the SYSLOG priority name lookup

	Returns:

	>=0		resulting SYSLOG priority number
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/syslog.h>		/* for LOG_XXX numbers */
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PRINAMELEN
#define	PRINAMELEN	12		/* to handle 'information' */
#endif

#ifndef	LOGPRI
#define	LOGPRI		struct logpri
#endif


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;

extern char	*strwcpylc(char *,const char *,int) ;


/* local structures */

struct logpri {
	const char	*name ;
	int		pri ;
} ;


/* local variables */

static const struct logpri	logpris[] = {
	{ "emergency", LOG_EMERG },
	{ "alert", LOG_ALERT },
	{ "critical", LOG_CRIT },
	{ "error", LOG_ERR },
	{ "warning", LOG_WARNING },
	{ "notice", LOG_NOTICE },
	{ "information", LOG_INFO },
	{ "debug", LOG_DEBUG },
	{ NULL, 0 }
} ;


/* exported subroutines */


int getlogpri(const char *np,int nl)
{
	const LOGPRI	*lfs = logpris ;
	const int	n = 2 ;
	int		rs = SR_OK ;
	int		i ;
	int		m ;
	int		nlen ;
	int		ch ;
	int		logpri = LOG_INFO ;
	int		f = FALSE ;
	const char	*anp ;
	char		nbuf[PRINAMELEN + 1] ;

	if ((nl < 0) || (nl > PRINAMELEN)) nl = PRINAMELEN ;

	nlen = strwcpylc(nbuf,np,nl) - nbuf ;

	ch = nbuf[0] ;
	if (isdigitlatin(ch)) {
	    rs = cfdeci(nbuf,nlen,&logpri) ;
	    if (rs >= 0) {
		if ((logpri < 0) || (logpri > 7)) rs = SR_DOM ;
	    }
	} else if (isalphalatin(ch)) {
	    for (i = 0 ; lfs[i].name != NULL ; i += 1) {
	        anp = lfs[i].name ;
	        m = nleadstr(anp,nbuf,nlen) ;
	        f = ((m == nlen) && ((m >= n) || (anp[m] == '\0'))) ;
	        if (f) break ;
	    } /* end for */
	    logpri = lfs[i].pri ;
	    if (! f) rs = SR_NOTFOUND ;
	} else if (ch != '\0') {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? logpri : rs ;
}
/* end subroutine (getlogpri) */


#ifdef	COMMENT
const char *strfacname(int fac)
{
	const LOCPRI	*lfs = logpris ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; lfs[i].name != NULL ; i += 1) {
	    f = (lfs[i].fac == fac) ;
	    if (f) break ;
	} /* end for */

	return (f) ? lfs[i].name : NULL ;
}
/* end subroutine (strfacname) */
#endif /* COMMENT */


