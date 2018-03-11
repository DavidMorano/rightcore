/* progpid */

/* PID lock management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2008-10-10, David A­D­ Morano
	This was adapted from the BACKGROUND program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manage the main process PID lock.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<lfm.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdecti(const char *,int,int *) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* forward references */


/* local variables */


/* exported subroutines */


int progpidbegin(PROGINFO *pip,int to)
{
	int		rs = SR_OK ;
	int		f ;

	f = (pip->f.named || pip->f.passfd) ;
	if (! f) {
	    int		cl = -1 ;
	    cchar	*cp = pip->pidfname ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    if ((cp == NULL) || (cp[0] == '+')) {
	        cp = pip->searchname ;
	        cl = -1 ;
	    }

	    if (cp[0] != '-') {

	        if (cp[0] != '/') {
	            if (strchr(cp,'/') != NULL) {
	                cl = mkpath2(tmpfname,pip->pr,cp) ;
	            } else {
	                cl = mkpath3(tmpfname,pip->pr,RUNDNAME,cp) ;
	            }
	            cp = tmpfname ;
	        }

	        if (cp != NULL) {
	            cchar	**vpp = &pip->pidfname ;
	            rs = proginfo_setentry(pip,vpp,cp,cl) ;
	        }

	        if (rs >= 0) {
	            LFM		*lmp = &pip->pidlock ;
	            const int	lt = LFM_TRECORD ;
	            const char	*pf = pip->pidfname ;
	            const char	*nn = pip->nodename ;
	            const char	*un = pip->username ;
	            const char	*bn = pip->banner ;
	            if ((rs = lfm_start(lmp,pf,lt,to,NULL,nn,un,bn)) >= 0) {
	                pip->open.pidlock = TRUE ;
	            }
	        }

	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (progpidbegin) */


int progpidcheck(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.pidlock) {
	    LFM			*lmp = &pip->pidlock ;
	    LFM_CHECK		lc ;
	    const time_t	dt = pip->daytime ;
	    rs = lfm_check(lmp,&lc,dt) ;
	    if (rs < 0) {
	        cchar	*fmt ;
	        char	tbuf[TIMEBUFLEN + 1] ;
	        timestr_logz(dt,tbuf) ;
	        if (pip->open.logprog) {
	            fmt = "%s lost PIDLOCK other PID=%d\n" ;
	            proglog_printf(pip,fmt,tbuf,lc.pid) ;
	        }
	        if (pip->debuglevel > 0) {
	            fmt = "%s: %s lost PIDLOCK other PID=%d\n" ;
	            printf(pip->efp,tbuf,pip->progname,tbuf,lc.pid) ;
	        }
	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (progpidcheck) */


int progpidend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.pidlock) {
	    LFM		*lmp = &pip->pidlock ;
	    pip->open.pidlock = FALSE ;
	    rs1 = lfm_finish(lmp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (progpidend) */


