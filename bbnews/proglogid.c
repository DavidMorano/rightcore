/* proglogid */

/* process the service names given us */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_SERIAL	1		/* serial or PID */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine creates a log-id for use in logging entries to a
        log-file.

	Synopsis:

	int proglogid(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		pointer to PROGINFO object

	Returns:

	<0		error
	>=0		OK

	Notes:

        For programs that are actually built-in SHELL commands, the compile-time
        switch flag CF_SERIAL must always be enabled (set to '1') so that the
        process PID is not used. This is because the process PID will always be
        the same for calls to built-in commands.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ugetpid.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	getserial(const char *) ;
extern int	isprintlatin(int) ;


/* externals variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int proglogid(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		v = -1 ;
	int		ll = 0 ;

	if (pip->pid < 0) pip->pid = ugetpid() ;

#if	CF_SERIAL
	{
	    const char	*sfname = SERIALFNAME ;
	    const char	*tdn ;
	    char	tbuf[MAXPATHLEN + 1] ;

	    tbuf[0] = '\0' ;
	    rs = mkpath3(tbuf,pip->pr,VCNAME,sfname) ;
	    if ((rs >= 0) && (tbuf[0] != '\0')) {
	        v = getserial(tbuf) ;
	    }

	    if ((rs >= 0) && (v < 0)) {
	        tdn = pip->tmpdname ;
	        if ((tdn != NULL) && (tdn[0] != '0')) {
	            if ((rs = mkpath2(tbuf,pip->tmpdname,sfname)) >= 0) {
	                if (tbuf[0] != '\0') {
	                    v = getserial(tbuf) ;
			}
		    }
	        }
	    }
	} /* end block */
#endif /* CF_SERIAL */

	if ((rs >= 0) && (v < 0)) {
	    v = ((int) pip->pid) ;
	}

	if (rs >= 0) {
	    const int	llen = LOGIDLEN ;
	    cchar	*nn = pip->nodename ;
	    char	lbuf[LOGIDLEN + 1] ;
	    if ((rs = mklogid(lbuf,llen,nn,-1,v)) > 0) {
		const char	**vpp = &pip->logid ;
		ll = rs ;
	        rs = proginfo_setentry(pip,vpp,lbuf,ll) ;
	    }

	} /* end block (logid) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (proglogid) */


