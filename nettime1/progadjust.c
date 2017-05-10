/* progadjust */

/* program to get time from a network time server host */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will get the time-of-day from a time server specified by a
	hostname given on the command line.  The program tries to connect to a
	TCP listener on the time server and will read 4 bytes out of the
	socket.  These four bytes, when organized as a long word in network
	byte order, represent the time in seconds since Jan 1, 1900.  We will
	subtract the value "86400 * ((365 * 70) + 17)" to get the time in
	seconds since Jan 1, 1970 (which was when the UNIX OS started, or
	something like this).  The bytes received from the socket are in
	network byte order, so we will have to convert them into a long word
	order recognized by the local machine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	CHANGELARGE	(10 * 60)
#define	CHANGESMALL	(50)

#define	EPOCHDIFF	(86400 * ((365 * 70) + 17))

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	matostr(const char **,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int progadjust(PROGINFO *pip,struct timeval *tvp)
{
	int		rs = SR_OK ;

	if (tvp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progadjust: time-off sec=%ld usec=%ld\n",
	    		tvp->tv_sec,tvp->tv_usec) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: time-off sec=%ld usec=%ld",
	    		pip->progname,tvp->tv_sec,tvp->tv_usec) ;
	}

	if (labs(tvp->tv_sec) > CHANGELARGE)
	    return SR_TOOBIG ;

	if (labs(tvp->tv_usec) > 1000000)
	    return SR_DOM ;

	    if (pip->open.logsys) {
		const int	logpri = LOG_NOTICE ;
		logsys_printf(&pip->ls,logpri,"time-off sec=%ld usec=%ld",
	    		tvp->tv_sec,tvp->tv_usec) ;
	    }

	if (labs(tvp->tv_sec) > 0) {
	    if (pip->open.logsys) {
		const int	logpri = LOG_NOTICE ;
		logsys_printf(&pip->ls,logpri,"adjusting=%u",(! pip->f.test)) ;
	    }
	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: adjusting=%u",(! pip->f.test)) ;
	    }
	    if (! pip->f.test) {
	        rs = u_adjtime(tvp,NULL) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progadjust: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progadjust) */


