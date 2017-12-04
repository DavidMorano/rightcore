/* progsvc */

/* handle some service processing */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGN	0		/* special (live) debugging */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module handles initializing, checking, and freeing the service-file
        object.


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
#include	<logfile.h>
#include	<svcfile.h>
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

#undef	DEBFNAME
#define	DEBFNAME	"svcfile.deb"

#ifndef	SVCBUFLEN
#define	SVCBUFLEN	128
#endif

#define	SVBUFLEN	100


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
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;

extern int	securefile(const char *,uid_t,gid_t) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

int	progsvcopen(PROGINFO *) ;
int	progsvcclose(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
static int	proclist(PROGINFO *,const char *) ;
#endif


/* local variables */

/* 'conf' for most regular programs */
static const char	*sched_system[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*sched_user[] = {
	"%h/%e/%n/%n.%f",
	"%h/%e/%n/%f",
	"%h/%e/%n.%f",
	"%h/%n.%f",
	NULL
} ;


/* exported subroutines */


int progsvcopen(pip)
PROGINFO	*pip ;
{
	const int	tlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		f_secreq = TRUE ;
	const char	*cp ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progsvcopen: ent\n") ;
	    debugprintf("progsvcopen: pr=%s\n",pip->pr) ;
	    debugprintf("progsvcopen: hd=%s\n",pip->homedname) ;
	}
#endif

	rs1 = SR_NOENT ;
	f_secreq = (! pip->f.proglocal) ;

	cp = pip->svcfname ;
	cl = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: 0 svcfname=%t\n",cp,cl) ;
#endif

	if ((cp == NULL) || (cp[0] == '+')) {
	    cp = SVCFNAME ;
	    cl = -1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: 1 svcfname=%t\n",cp,cl) ;
#endif

	if (strchr(cp,'/') == NULL) {
	    VECSTR	*plp = &pip->svars ;
	    cchar	**sched ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progsvcopen: permsched() svcfname=%s\n",cp) ;
	    debugprintf("progsvcopen: tmptype=%u\n",pip->tmptype) ;
	}
#endif

	    f_secreq = FALSE ;
	    switch (pip->tmptype) {
	    case tmptype_system:
	    default:
		sched = sched_system ;
		break ;
	    case tmptype_user:
		sched = sched_user ;
		break ;
	    } /* end switch */
	        rs1 = permsched(sched,plp,tbuf,tlen,cp,R_OK) ;
	        cl = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: permsched() rs=%d\n",rs1) ;
#endif

	    if ((rs1 >= 0) && (cl > 0))
	        cp = tbuf ;

	} else {

	    if ((rs1 = getfname(pip->pr,cp,TRUE,tbuf)) >= 0) {
	        cl = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: getfname() rs1=%d\n",rs1) ;
#endif

	        cp = tbuf ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progsvcopen: mid rs=%d rs1=%d\n",rs,rs1) ;
	    debugprintf("progsvcopen: cl=%d\n",cl) ;
	    debugprintf("progsvcopen: c=%t\n",cp,cl) ;
	}
#endif

	if ((rs >= 0) && (rs1 >= 0) && (cp != NULL)) {
	    cchar	**vpp = &pip->svcfname ;
	    rs = proginfo_setentry(pip,vpp,cp,cl) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progsvcopen: _setentry() rs=%d\n",rs) ;
	    debugprintf("progsvcopen: svcfname=%s\n", pip->svcfname) ;
	}
#endif
	}

	if ((rs < 0) || (rs1 < 0))
	    goto ret0 ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: svc=%s\n",
	        pip->progname,pip->svcfname) ;

	if (pip->open.logprog)
	    proglog_printf(pip,"svc=%s\n",pip->svcfname) ;

	if ((rs = perm(pip->svcfname,-1,-1,NULL,R_OK)) >= 0) {
	    if (pip->fromconf.svcfname) {
	        pip->f.secure_svcfile = 
	            pip->f.secure_root && pip->f.secure_conf ;
	        } else
	            pip->f.secure_svcfile = pip->f.secure_root ;

	    if (f_secreq || (! pip->f.secure_svcfile)) {

	        rs = securefile(pip->svcfname,pip->euid,pip->egid) ;
	        pip->f.secure_svcfile = (rs > 0) ;

	    } /* end if */
	}

/* try to actually open the service file */

	if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: svcfile_open() svcfname=%s\n",
		pip->svcfname) ;
#endif

	    rs = svcfile_open(&pip->stab,pip->svcfname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        SVCFILE_CUR	cur ;
	        int	nlen ;
	        int	c = 0 ;
	        char	nbuf[SVCNAMELEN + 1] ;
	        debugprintf("progsvcopen: svcfile_open() rs=%d\n",rs) ;
	        svcfile_curbegin(&pip->stab,&cur) ;
	        while (rs >= 0) {
	            nlen = svcfile_enumsvc(&pip->stab,&cur,nbuf,SVCNAMELEN) ;
	            if (nlen < 0)
	                break ;
	            c += 1 ;
	            debugprintf("progsvcopen: 1 svc=%s\n",nbuf) ;
	        }
	        svcfile_curend(&pip->stab,&cur) ;
	        debugprintf("progsvcopen: svcfile_open() end c=%u\n",c) ;
	    }
#endif /* CF_DEBUG */

	} /* end if (ok) */

	if ((rs < 0) && pip->open.logprog) {

	    proglog_printf(pip,
	        "srvfile=%s",pip->svcfname) ;

	    proglog_printf(pip,
	        "file unavailable (%d)",rs) ;

	    bprintf(pip->efp,"%s: svc=%s\n",
	        pip->progname,pip->svcfname) ;

	    bprintf(pip->efp,"%s: file unavailable (%d)\n",
	        pip->progname,rs) ;

	} /* end if (log bad file) */

	if (rs < 0)
	    goto ret0 ;

	pip->open.svcfname = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    SVCFILE_CUR	cur ;
	    int		kl ;
	    int		c = 0 ;
	    char	kbuf[KBUFLEN + 1] ;
	    debugprintf("progsvcopen: svcs start\n") ;
	    svcfile_curbegin(&pip->stab,&cur) ;
/* CONSTCOND */
	    while (TRUE) {
	        kl = svcfile_enumsvc(&pip->stab,&cur,kbuf,KBUFLEN) ;
	        if (kl < 0)
	            break ;
	        c += 1 ;
	        debugprintf("progsvcopen: 2 svc=%s\n",kbuf) ;
	    } /* end for */
	    svcfile_curend(&pip->stab,&cur) ;
	    debugprintf("progsvcopen: svcs end c=%u\n",c) ;
	}
#endif /* CF_DEBUG */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsvcopen) */


int progsvcclose(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;
	int	rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcclose: ent\n") ;
#endif

	if (pip->open.svcfname) {
	    pip->open.svcfname = FALSE ;
	    rs1 = svcfile_close(&pip->stab) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsvcclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsvcclose) */


int progsvccheck(pip)
PROGINFO	*pip ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if (! pip->open.svcfname)
	    goto ret0 ;

	rs = svcfile_check(&pip->stab,pip->daytime) ;
	c = rs ;

#if	CF_DEBUG && CF_DEBUGN && defined(DEBFNAME)
	nprintf(DEBFNAME,"progsvccheck: svcfile_check() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (pip->open.logprog && c) {
	    proglog_printf(pip,"%s services changed (%u)\n",
	        timestr_logz(pip->daytime,timebuf),c) ;
	}

#if	CF_DEBUG && CF_DEBUGN && defined(DEBFNAME)
	if ((rs >= 0) && c) {
	    SVCFILE_ENT	sv ;
	    const int	svlen = SVBUFLEN ;
	    int		rs1 ;
	    int		i ;
	    const char	*sn = "helloworld" ;
	    char	svbuf[SVBUFLEN + 1] ;
	    rs1 = svcfile_fetch(&pip->stab,sn,NULL,&sv,svbuf,svlen) ;
	    nprintf(DEBFNAME,"progsvccheck: svcfile_fetch() rs=%d\n",rs1) ;
	    if (rs1 >= 0) {
	        nprintf(DEBFNAME,"progsvccheck: svc=%s\n",sv.svc) ;
	        for (i = 0 ; sv.keyvals[i][0] != NULL ; i += 1) {
	            nprintf(DEBFNAME,"progsvccheck: k=%s v=>%s<\n",
	                sv.keyvals[i][0],sv.keyvals[i][1]) ;
		}
	    }
#endif /* CF_DEBUG */
#if	CF_DEBUG 
	if (DEBUGLEVEL(5)) {
	    if ((rs >= 0) && c) proclist(pip,"check") ;
	}
#endif /* CF_DEBUG */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progsvccheck) */


#if	CF_DEBUGS || CF_DEBUG

static int proclist(pip,s)
PROGINFO	*pip ;
const char	*s ;
{
	SVCFILE		*svcp = &pip->stab ;
	SVCFILE_CUR	cur ;
	const int	svclen = SVCBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl ;
	int		i = 0 ;
	char		svcbuf[SVCBUFLEN + 1] ;

	if (! pip->open.svcfname)
	    goto ret0 ;

	if (s == NULL) s = "" ;

	if ((rs = svcfile_curbegin(svcp,&cur)) >= 0) {

	    while (rs >= 0) {
	        rs1 = svcfile_enumsvc(svcp,&cur,svcbuf,svclen) ;
	        sl = rs1 ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;
	        if (rs < 0) break ;

	        debugprintf("proclist: %s svc[%u]=%s\n",
			s,i++,svcbuf) ;

	    } /* end while */

	    svcfile_curend(svcp,&cur) ;
	} /* end if (cursor) */

ret0:
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (proclist) */

#endif /* CF_DEBUGS */


