/* proglogfname */

/* program log-fname-name calculation */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine calculates the log filename for a program.

	Synopsis:

	int proglogfname(pip,tmpfname,logcname,logfname)
	PROGINFO	*pip ;
	char		tmpfname[] ;
	const char	logcname[] ;
	const char	*logfname ;

	Arguments:

	pip		program-information pointer
	tmpfname	buffer to receive result
	logcname	directory component name for log-directory
	logfname	the name of the (supposed) log file

	Returns:

	<0		error
	>=0		length of created file-name


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int proglogfname(PROGINFO *pip,char *rbuf,cchar *logcname,cchar *lfname)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("proglogfname: ent lfname=%s\n",lfname) ;
#endif

	rbuf[0] = '\0' ;
	if ((lfname == NULL) || (lfname[0] == '+'))
	    lfname = pip->searchname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("proglogfname: adj lfname=%s\n",lfname) ;
#endif

	if ((lfname != NULL) && (lfname[0] != '\0') && (lfname[0] != '-')) {

	    if (lfname[0] != '/') {

	        if (strchr(lfname,'/') != NULL) {
	            rs = mkpath2(rbuf,pip->pr,lfname) ;
	        } else {
		    const char	*logdname ;
		    char	tmpdname[MAXPATHLEN+1] ;
	            if (logcname == NULL) logcname = LOGCNAME ;
		    logdname = logcname ;
		    if (logdname[0] != '/') {
			rs = mkpath2(tmpdname,pip->pr,logcname) ;
		        logdname = tmpdname ;
		    }
		    if (rs >= 0)
	                rs = mkpath2(rbuf,logdname,lfname) ;
	        }
	        pl = rs ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("proglogfname: ret rbuf=%s\n",rbuf) ;
	debugprintf("proglogfname: ret rs=%d pl=%u\n",rs,pl) ;
	}
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (proglogfname) */


