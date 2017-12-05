/* progacc */

/* handle some service processing */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was borrowed and modified from previous generic
        front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Prepare to do some servicing.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<acctab.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;

extern int	securefile(const char *,uid_t,gid_t) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

#ifdef	COMMENT

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

#endif /* COMMENT */

/* non-'conf' ETC stuff for all regular programs */
static const char	*sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	NULL
} ;

#ifdef	COMMENT
/* non-'conf' ETC stuff for local searching */
static const char	*sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL
} ;
#endif /* COMMENT */


/* exported subroutines */


int progaccopen(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		cl ;
	int		f_secreq = FALSE ;
	int		f_open = FALSE ;

	char		tmpfname[MAXPATHLEN + 1] ;
	cchar		*cp ;

	rs1 = SR_NOENT ;
	f_secreq = (! pip->f.proglocal) ;

	cp = pip->accfname ;
	cl = -1 ;

	if ((cp == NULL) || (cp[0] == '+')) {
		cp = ACCFNAME ;
		cl = -1 ;
	}

	if (strchr(cp,'/') == NULL) {

	    f_secreq = FALSE ;
	    rs1 = permsched(sched2,&pip->svars,
	        tmpfname,MAXPATHLEN, cp,R_OK) ;
	    cl = rs1 ;

#ifdef	COMMENT
	    if (rs1 == SR_NOENT)
	        rs1 = permsched(sched3,&pip->svars,
	            tmpfname,MAXPATHLEN, cp,R_OK) ;
		cl = rs1 ;
#endif /* COMMENT */

	    if ((rs1 >= 0) && (cl > 0))
		cp = tmpfname ;

	} else {

	    if (cp[0] == '/')
	        rs = mkpath1(tmpfname,cp) ;
	    else
	        rs = mkpath2(tmpfname,pip->pr,cp) ;
	    if (rs >= 0) {
	        rs1 = SR_OK ;
		cp = tmpfname ;
	    }

	} /* end if */

	if ((rs >= 0) && (rs1 >= 0) && (cp != NULL))
	    rs = proginfo_setentry(pip,&pip->accfname,cp,cl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("progaccopen: search cp=%t\n",cp,cl) ;
	debugprintf("progaccopen: search rs=%d\n", rs) ;
	debugprintf("progaccopen: accfname=%s\n",pip->accfname) ;
	debugprintf("progaccopen: f_secreq=%u\n",f_secreq) ;
	}
#endif

	if (rs < 0)
	    goto ret0 ;

	if (rs1 < 0)
	    goto ret0 ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: acc=%s\n",
		pip->progname,
		((pip->accfname != NULL) ? pip->accfname : "")) ;

	if (pip->open.logprog)
	    proglog_printf(pip,"acc=%s\n",
		((pip->accfname != NULL) ? pip->accfname : "")) ;

	rs1 = perm(pip->accfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progaccopen: perm() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {
	    if (pip->fromconf.accfname)
	        pip->f.secure_accfile = 
			pip->f.secure_root && pip->f.secure_conf ;
	    else
	        pip->f.secure_accfile = pip->f.secure_root ;

	    if (f_secreq || (! pip->f.secure_accfile)) {

	            rs1 = securefile(pip->accfname,pip->euid,pip->egid) ;
	            pip->f.secure_accfile = (rs1 > 0) ;

	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("progaccopen: securefile() rs=%d\n",rs1) ;
	debugprintf("progaccopen: secure_accfile=%u\n",pip->f.secure_accfile) ;
	}
#endif

	if (rs1 >= 0) {
	    rs1 = acctab_open(&pip->atab,pip->accfname) ;
	}

	if (rs1 >= 0) {
	        pip->open.accfname = TRUE ;
		f_open = TRUE ;
	}

	if (rs1 < 0) {

	    if (pip->open.logprog)
	        proglog_printf(pip,"acctab unavailable (%d)\n",rs1) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: acctab unavailable (%d)\n",
	            pip->progname,rs1) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4) && pip->open.accfname) {
	    ACCTAB_CUR	ac ;
	    ACCTAB_ENT	*ep ;
	    debugprintf("progaccopen: netgroup machine user password\n") ;
	    acctab_curbegin(&pip->atab,&ac) ;
	    while (acctab_enum(&pip->atab,&ac,&ep) >= 0) {
	        if (ep == NULL) continue ;
	        debugprintf("progaccopen: %-20s %-20s\n",
	            ep->netgroup.std,ep->machine.std) ;
	        debugprintf("progaccopen: %-8s %-8s\n",
	            ep->username.std,ep->password.std) ;
	    }
	    acctab_curend(&pip->atab,&ac) ;
	}
#endif /* CF_DEBUG */

ret0:
	return (rs >= 0) ? f_open : rs ;
}
/* end subroutine (progaccopen) */


int progaccclose(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.accfname) {
	    pip->open.accfname = FALSE ;
	    rs = acctab_close(&pip->atab) ;
	}

	return rs ;
}
/* end subroutine (progaccclose) */


int progacccheck(PROGINFO *pip)
{
	int	rs = SR_OK ;
	int	f_changed = FALSE ;
	char	timebuf[TIMEBUFLEN + 1] ;

	if (pip->open.accfname) {
	    if ((rs = acctab_check(&pip->atab)) > 0) {
	        f_changed = TRUE ;
	        if (pip->open.logprog && f_changed) {
		    cchar	*fmt = "%s access-permissions changed\n" ;
	            timestr_logz(pip->daytime,timebuf) ;
	            proglog_printf(pip,fmt,timebuf) ;
	        }
	    }
	}

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (progacccheck) */


