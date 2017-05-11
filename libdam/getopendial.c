/* getopendial */

/* get the OPENDIAL code for a dialer specification (string) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the open-dial code (an integer value) for an open-dial name string.

	Synopsis:

	int getopendial(cchar *dialspec)

	Arguments:

	dialspec	the dialer specification string

	Returns:

	>=0		the open-dialer code for the specified dialer
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"opendial.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

struct opendialer {
	const char	*name ;
	int		dialer ;
} ;


/* forward references */


/* local variables */

static const struct opendialer	dialers[] = {
	{ "udp", opendialer_udp },
	{ "tcp", opendialer_tcp },
	{ "tcpmux", opendialer_tcpmux },
	{ "tcpnls", opendialer_tcpnls },
	{ "uss", opendialer_uss },
	{ "ussmux", opendialer_ussmux },
	{ "ussnls", opendialer_ussnls },
	{ "ticotsord", opendialer_ticotsord },
	{ "ticotsordnls", opendialer_ticotsordnls },
	{ "pass", opendialer_pass },
	{ "open", opendialer_open },
	{ "prog", opendialer_prog },
	{ "finger", opendialer_finger },
	{ NULL, 0 }
} ;


/* exported subroutines */


int getopendial(const char *name)
{
	int		v = SR_NOTFOUND ;
	int		i ;
	int		f = FALSE ;
	const char	*np ;

	for (i = 0 ; dialers[i].name != NULL ; i += 1) {
	        np = dialers[i].name ;
	        f = (strcmp(np,name) == 0) ;
	        if (f) break ;
	} /* end for */

	if (f) {
	    v = dialers[i].dialer ;
	} else {
	    if (strcmp(name,"unix") == 0) {
		v = opendialer_uss ;
	    }
	}

	return v ;
}
/* end subroutine (getopendial) */


