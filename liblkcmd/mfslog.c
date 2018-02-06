/* mfs-log */

/* utility to support logging */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2011-01-25, David A­D­ Morano
        This code was seperated out for for more modularity. This was in turn
        needed to fix the AST-code sockets library definition problems (see
        notes elsewhere).

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2011,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This modeule contains code to support logging of various bit of
        information.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"proglog.h"
#include	"mfsmain.h"
#include	"mfslocinfo.h"
#include	"mfslisten.h"
#include	"mfslog.h"
#include	"defs.h"


/* local defines */

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

extern int	proglogout(PROGINFO *,cchar *,cchar *) ;

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
/* end subroutine (logbegin) */


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


int logssprint(PROGINFO *pip,cchar *lid,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    rs = proglog_ssprint(pip,lid,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (logssprint) */


int logssprintf(PROGINFO *pip,cchar *id,cchar *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	if (pip->open.logprog) {
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = proglog_ssvprintf(pip,id,fmt,ap) ;
	    wlen = rs ;
	    va_end(ap) ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (logssprintf) */


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
		    strcpy(timebuf,"·") ;
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


int loglisteners(PROGINFO *pip)
{
	MFSLISTEN_INST	inst ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;
	for (i = 0 ; mfslisten_getinst(pip,&inst,i) >= 0 ; i += 1) {
	    MFSLISTEN_INST	*lp = &inst ;
	    cchar		*fmt ;
	    if (lp->type[0] != '\0') {
	        const int	ls = lp->state ;
	        cchar		*sn ;
	        if (ls & LISTENSPEC_MDELPEND) {
	            sn = "D" ;
	        } else if (ls & LISTENSPEC_MBROKEN) {
	            sn = "B" ;
	        } else if (ls & LISTENSPEC_MACTIVE) {
	            sn = "A" ;
	        } else if (ls == 0) {
	            sn = "C" ;
	        } else {
	            sn = "U" ;
	        }
	        fmt = "i=%u %s type=%s addr=%s (%d)\n" ;
	        rs = logprintf(pip,fmt,i,sn,lp->type,lp->addr,ls) ;
	        wlen += rs ;
	    } else {
	        fmt = "i=%u no-name\n" ;
	        rs = logprintf(pip,fmt,i) ;
	        wlen += rs ;
	    } /* end if (name) */
	    if (rs < 0) break ;
	} /* end for */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (loglisteners) */


int logreport(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    char	timebuf[TIMEBUFLEN + 1] ;

	    timestr_logz(pip->daytime,timebuf) ;
	    if ((rs = proglog_printf(pip,"%s report",timebuf)) >= 0) {
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


int loginvalidcmd(PROGINFO *pip,cchar *cmd)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    const int	cl = strnlen(cmd,40) ;
	    cchar	*fmt = "%s invalid cmd=%t" ;
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
	    char	dbuf[DIGBUFLEN + 1] ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    if (pip->pidfname != NULL) {
	        proglog_printf(pip,"pid=%s",pip->pidfname) ;
	    }

	    if (pip->f.daemon) {
		const int	v = pip->pid ;
	        proglog_printf(pip,"daemon pid=%u",v) ;
	    }

	    proglog_printf(pip, "intpoll=%s",strval(dbuf,pip->intpoll)) ;

	    proglog_printf(pip, "intmark=%s",strval(dbuf,pip->intmark)) ;

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


int logoutfile(PROGINFO *pip,cchar *lid,cchar *ms,cchar *fn)
{
	int		rs = SR_OK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("logoutfile: ent ms=%s fn=%s\n",ms,fn) ;
#endif
	if ((pip->open.logprog) && (fn != NULL)) {
	    if ((rs = proglog_setid(pip,lid,-1)) >= 0) {
		if ((rs = proglogout(pip,ms,fn)) >= 0) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(5))
		    debugprintf("logoutfile: proglogout() rs=%d\n",rs) ;
#endif
		    rs = proglog_setid(pip,pip->logid,-1) ;
		}
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("logoutfile: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (logoutfile) */


int loglock(PROGINFO *pip,LFM_CHECK *lcp,cchar *lfname,cchar *np)
{
	int		rs = SR_OK ;
	char		timebuf[TIMEBUFLEN + 1] ;

	timestr_logz(pip->daytime,timebuf) ;
	proglog_printf(pip,"%s lock %s\n", timebuf, np) ;

	proglog_printf(pip,"lf=%s",lfname) ;

	proglog_printf(pip,"other_pid=%d\n", lcp->pid) ;

	if (lcp->nodename != NULL) {
	    proglog_printf(pip, "other_node=%s\n", lcp->nodename) ;
	}

	if (lcp->username != NULL) {
	    proglog_printf(pip, "other_user=%s\n", lcp->username) ;
	}

	if (lcp->banner != NULL) {
	    proglog_printf(pip, "other_banner=>%s<\n", lcp->banner) ;
	}

	return rs ;
}
/* end subroutine (loglock) */


/* local subroutines */


