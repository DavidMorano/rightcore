/* opensvc_weather */

/* LOCAL facility open-service (weather) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano

        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_weather(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<nistinfo.h>
#include	<localmisc.h>

#include	"opensvc_weather.h"
#include	"defs.h"


/* local defines */

#define	DEFWS	"kbos"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	openweather(const char *,const char *,int,int) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_nist(time_t,struct nistinfo *,char *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensvc_weather(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		fd = -1 ;
	const char	*ws = NULL ;

	if (argv) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc > 1) && (argv[1] != NULL)) ws = argv[1] ;
	}

	if ((ws == NULL) || (ws[0] == '\0')) ws = DEFWS ;

	rs = openweather(pr,ws,of,to) ;
	fd = rs ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_weather) */


