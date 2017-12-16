/* msu-log */

/* utilies to support logging */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2011-01-25, David A­D­ Morano
        This code was seperated out for for more modularity. This was in turn
        needed to fix the AST-code sockets library definition problems (see
        notes elsewhere).

*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This modeule contains code to support logging of various bit of
        information.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"msumain.h"
#include	"msulocinfo.h"
#include	"msulog.h"
#include	"defs.h"


/* local defines */

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MSGBUFLEN
#endif

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;
extern char	*strval(char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	logfind(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int logbegin(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->f.logprog) {
	    if ((rs = logfind(pip)) >= 0) {
	        const char	*li = pip->logid ;
	        const char	*lf = pip->lfname ;
	        if ((lf != NULL) && (lf[0] != '\0')) {
	            const mode_t	om = 0666 ;
	            if ((rs = logfile_open(&pip->lh,lf,0,om,li)) >= 0) {
	                pip->open.logprog = TRUE ;
	                rs = logprogname(pip) ;
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
	            }
	        } /* end if (non-empty) */
	    } /* end if (logfind) */
	} /* end if (logging specified) */

	return rs ;
}
/* end subroutine (logbegin) */


int logend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    rs1 = logfile_close(&pip->lh) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (logend) */


int logflush(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    rs = logfile_flush(&pip->lh) ;
	}

	return rs ;
}
/* end subroutine (logflush) */


int logcheck(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    rs = logfile_check(&pip->lh,pip->daytime) ;
	}

	return rs ;
}
/* end subroutine (logflush) */


int logprintf(PROGINFO *pip,const char *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = logfile_vprintf(&pip->lh,fmt,ap) ;
	    wlen = rs ;
	    va_end(ap) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (logprintf) */


int logprogname(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    cchar	*a = getourenv(pip->envv,VARARCHITECTURE) ;
	    cchar	*s = getourenv(pip->envv,VARSYSNAME) ;
	    cchar	*r = getourenv(pip->envv,VARRELEASE) ;
	    cchar	*n = pip->name ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    if (pip->logsize > 0) {
	        logfile_checksize(&pip->lh,pip->logsize) ;
	    }

	    fmt = "%s %-14s %s" ;
	    timestr_logz(pip->daytime,timebuf) ;
	    logfile_printf(&pip->lh,fmt,timebuf,pn,pip->version) ;

	    if ((s != NULL) && (r != NULL)) {
	        cchar	*dn = pip->domainname ;

	        if (a != NULL) {
	            fmt = "a=%s os=%s(%s) d=%s" ;
	            rs = logfile_printf(&pip->lh,fmt,a,s,r,dn) ;
	        } else {
	            fmt = "os=%s(%s) d=%s" ;
	            rs = logfile_printf(&pip->lh,fmt,s,r,dn) ;
	        }

	    } /* end if (OS system information) */

	    if (rs >= 0) {
		cchar	*nn = pip->nodename ;
		cchar	*un = pip->username ;
	        fmt = (n != NULL) ? "%s!%s (%s)" : "%s!%s" ;
	        rs = logfile_printf(&pip->lh,fmt,nn,un,n) ;
	    }

	} /* end if (logprog) */

	return rs ;
}
/* end subroutine (logprogname) */


int logmark(PROGINFO *pip,int rem)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/logmark: open.logprog=%u\n",pip->open.logprog) ;
#endif

	if (pip->open.logprog) {
	    cchar	*nn = pip->nodename ;
	    cchar	*fmt ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    if (rs >= 0) {
	        fmt = "%s mark> %s" ;
	        timestr_logz(pip->daytime,timebuf) ;
	        rs = logfile_printf(&pip->lh,fmt,timebuf,nn) ;
	    }

	    if (rs >= 0) {
		cchar	*un = pip->username ;
	        cchar	*n = pip->name ;
	        fmt = (n != NULL) ? "%s!%s (%s)" : "%s!%s" ;
	        rs = logfile_printf(&pip->lh,fmt,nn,un,n) ;
	    }

	    if (rs >= 0) {
		int	v = pip->pid ;
		fmt = "pid=%d" ;
	        rs = logfile_printf(&pip->lh,fmt,v) ;
	    }

	    if (rs >= 0) {
		time_t	rtime = rem ;
 		fmt = "remaining=%s" ;
		if (rem <= 0) {
		    strcpy(timebuf,"·") ;
		} else {
	            timestr_elapsed(rtime,timebuf) ;
		}
	        rs = logfile_printf(&pip->lh,fmt,timebuf) ;
	    }

	} /* end if (log-prog) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/logmark: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (logmark) */


int logreport(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (pip->open.logprog) {

	    timestr_logz(pip->daytime,timebuf) ;
	    if ((rs = logfile_printf(&pip->lh, "%s report",timebuf)) >= 0) {
	        rs = logfile_printf(&pip->lh, "narkint=%u",pip->intmark) ;
	    }

	    if (rs >= 0) {
	        timestr_logz(lip->ti_marklog,timebuf) ;
	        rs = logfile_printf(&pip->lh, "marktime=%s",timebuf) ;
	    }

	} /* end if (logprog) */

	return rs ;
}
/* end subroutine (logreport) */


int loginvalidcmd(PROGINFO *pip,cchar *cmd)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    const int	cl = strnlen(cmd,40) ;
	    const char	*fmt = "%s invalid cmd=%t" ;
	    char	timebuf[TIMEBUFLEN + 1] ;
	    timestr_logz(pip->daytime,timebuf) ;
	    rs = logfile_printf(&pip->lh,fmt,timebuf,cmd,cl) ;
	}

	return rs ;
}
/* end subroutine (loginvalidcmd) */


int loginfo(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    long		lw ;
	    char		dbuf[DIGBUFLEN + 1] ;
	    char		timebuf[TIMEBUFLEN + 1] ;

	    if (pip->pidfname != NULL) {
	        logfile_printf(&pip->lh,"pid=%s",pip->pidfname) ;
	    }

	    logfile_printf(&pip->lh, "daemon pid=%u",((uint) pip->pid)) ;

	    logfile_printf(&pip->lh, "intpoll=%s", strval(dbuf,pip->intpoll)) ;

	    logfile_printf(&pip->lh, "intmark=%s", strval(dbuf,pip->intmark)) ;

	    lw = pip->intrun ;
	    if ((lw >= 0) && (lw < INT_MAX)) {
	        timestr_elapsed(lw,timebuf) ;
	    } else {
	        sncpy1(timebuf,TIMEBUFLEN,"max") ;
	    }
	    logfile_printf(&pip->lh,"intrun=%s", timebuf) ;

	    logfile_flush(&pip->lh) ;

	} /* end if (logprog) */

	return rs ;
}
/* end subroutine (loginfo) */


int loglock(PROGINFO *pip,LFM_CHECK *lcp,cchar *lfname,cchar *np)
{
	int		rs = SR_OK ;
	char		timebuf[TIMEBUFLEN + 1] ;

	timestr_logz(pip->daytime,timebuf) ;
	logfile_printf(&pip->lh, "%s lock %s\n",timebuf, np) ;

	logfile_printf(&pip->lh,"lf=%s",lfname) ;

	logfile_printf(&pip->lh, "other_pid=%d\n", lcp->pid) ;

	if (lcp->nodename != NULL) {
	    logfile_printf(&pip->lh, "other_node=%s\n", lcp->nodename) ;
	}

	if (lcp->username != NULL) {
	    logfile_printf(&pip->lh, "other_user=%s\n", lcp->username) ;
	}

	if (lcp->banner != NULL) {
	    logfile_printf(&pip->lh, "other_banner=>%s<\n", lcp->banner) ;
	}

	return rs ;
}
/* end subroutine (loglock) */


/* local subroutines */


static int logfind(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->lfname == NULL) {
	    cchar	*sn = pip->searchname ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(tbuf,pip->pr,LOGDNAME,sn)) >= 0) {
	        cchar	**vpp = &pip->lfname ;
	        rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	    } /* end if (mkpath) */
	}
	return rs ;
}
/* end subroutine (logfind) */


