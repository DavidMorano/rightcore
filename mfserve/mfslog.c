/* mfs-log */

/* utilies to support logging */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2011-01-25, David A�D� Morano
        This code was seperated out for for more modularity. This was in turn
        needed to fix the AST-code sockets library definition problems (see
        notes elsewhere).

*/

/* Copyright � 2011 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

        This modeule contains code to support logging of various bit of
        information.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"proglog.h"
#include	"mfsmain.h"
#include	"mfslocinfo.h"
#include	"mfslog.h"
#include	"defs.h"


/* local defines */

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif

#ifndef	TO_POLLMULT
#define	TO_POLLMULT	1000
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

#define	DEBUGFNAME	"/tmp/mfs.deb"


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

extern char	*getourenv(const char **,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static char	*strval(char *,int) ;


/* local variables */


/* exported subroutines */


int logbegin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("logbegin: ent\n") ;
#endif

	if (pip->f.logprog) {
	    rs = proglog_begin(pip,uip) ;
	} /* end if (logging specified) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("logbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (logstart) */


int logend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = proglog_end(pip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (logend) */


int logflush(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    rs = proglog_flush(pip) ;
	}

	return rs ;
}
/* end subroutine (logflush) */


int logcheck(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    rs = proglog_check(pip) ;
	}

	return rs ;
}
/* end subroutine (logcheck) */


int logprint(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    rs = proglog_print(pip,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (logprint) */


int logprintf(PROGINFO *pip,cchar *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = proglog_vprintf(pip,fmt,ap) ;
	    wlen = rs ;
	    va_end(ap) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (logprintf) */


int logmark(PROGINFO *pip,int rem)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mfs/logmark: open.logprog=%u\n",pip->open.logprog) ;
#endif

	if (pip->open.logprog) {
	    cchar	*nn = pip->nodename ;
	    cchar	*fmt ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    if (rs >= 0) {
	        fmt = "%s mark> %s" ;
	        timestr_logz(pip->daytime,timebuf) ;
	        rs = proglog_printf(pip,fmt,timebuf,nn) ;
	    }

	    if (rs >= 0) {
		cchar	*un = pip->username ;
	        cchar	*n = pip->name ;
	        fmt = (n != NULL) ? "%s!%s (%s)" : "%s!%s" ;
	        rs = proglog_printf(pip,fmt,nn,un,n) ;
	    }

	    if (rs >= 0) {
		const int	v = pip->pid ;
		fmt = "pid=%u" ;
	        rs = proglog_printf(pip,fmt,v) ;
	    }

	    if (rs >= 0) {
		const time_t	rtime = rem ;
 		fmt = "remaining=%s" ;
		if (rem <= 0) {
		    strcpy(timebuf,"�") ;
		} else {
	            timestr_elapsed(rtime,timebuf) ;
		}
	        rs = proglog_printf(pip,fmt,timebuf) ;
	    }

	} /* end if (open-logprog) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_mfs/logmark: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (logmark) */


int logreport(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    char	timebuf[TIMEBUFLEN + 1] ;

	    timestr_logz(pip->daytime,timebuf) ;
	    rs = proglog_printf(pip,"%s report",timebuf) ;

	    if (rs >= 0) {
	        rs = proglog_printf(pip,"narkint=%u",pip->intmark) ;
	    }

	    if (rs >= 0) {
	        timestr_logz(lip->ti_marklog,timebuf) ;
	        rs = proglog_printf(pip,"marktime=%s",timebuf) ;
	    }

	} /* end if (logprog) */

	return rs ;
}
/* end subroutine (logreport) */


int loginvalidcmd(PROGINFO *pip,cchar cmd[])
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    const int	cl = strnlen(cmd,40) ;
	    const char	*fmt = "%s invalid cmd=%t" ;
	    char	timebuf[TIMEBUFLEN + 1] ;
	    timestr_logz(pip->daytime,timebuf) ;
	    rs = proglog_printf(pip,fmt,timebuf,cmd,cl) ;
	}

	return rs ;
}
/* end subroutine (loginvalidcmd) */


int loginfo(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    long	lw ;
	    char	digbuf[DIGBUFLEN + 1] ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    if (pip->pidfname != NULL) {
	        proglog_printf(pip,"pid=%s",pip->pidfname) ;
	    }

	    if (pip->f.daemon) {
		const int	v = pip->pid ;
	        proglog_printf(pip,"daemon pid=%u",v) ;
	    }

	    proglog_printf(pip,
	        "intpoll=%s",strval(digbuf,pip->intpoll)) ;

	    proglog_printf(pip,
	        "intmark=%s",strval(digbuf,pip->intmark)) ;

	    lw = pip->intrun ;
	    if ((lw >= 0) && (lw < INT_MAX)) {
	        timestr_elapsed(lw,timebuf) ;
	    } else {
	        sncpy1(timebuf,TIMEBUFLEN,"max") ;
	    }

	    proglog_printf(pip,"intrun=%s",timebuf) ;

	    proglog_flush(pip) ;

	} /* end if (logprog) */

	return rs ;
}
/* end subroutine (loginfo) */


int loglock(PROGINFO *pip,LFM_CHECK *lcp,cchar *lfname,cchar np[])
{
	char		timebuf[TIMEBUFLEN + 1] ;

	timestr_logz(pip->daytime,timebuf) ;
	proglog_printf(pip, "%s lock %s\n", timebuf, np) ;

	proglog_printf(pip,"lf=%s",lfname) ;

	proglog_printf(pip, "other_pid=%d\n", lcp->pid) ;

	if (lcp->nodename != NULL) {
	    proglog_printf(pip,
	        "other_node=%s\n",
	        lcp->nodename) ;
	}

	if (lcp->username != NULL) {
	    proglog_printf(pip,
	        "other_user=%s\n",
	        lcp->username) ;
	}

	if (lcp->banner != NULL) {
	    proglog_printf(pip,
	        "other_banner=>%s<\n",
	        lcp->banner) ;
	}

	return SR_OK ;
}
/* end subroutine (loglock) */


/* local subroutines */


static char *strval(char *rbuf,int val)
{
	const int	rlen = DIGBUFLEN ;
	int		rs1 ;
	if ((val >= 0) && (val < INT_MAX)) {
	    rs1 = ctdeci(rbuf,rlen,val) ;
	    if (rs1 < 0)
	        sncpy1(rbuf,rlen,"bad") ;
	} else {
	    sncpy1(rbuf,rlen,"max") ;
	}
	return rbuf ;
}
/* end subroutine (strval) */


