/* progprocess */

/* handle some service processing */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_LOGID	1		/* use a special LOGID */
#define	CF_SETRUID	1		/* use 'setreuid(2)' */
#define	CF_SETEUID	0		/* already done in 'main()' */


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
#include	<baops.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<getax.h>
#include	<svcfile.h>
#include	<acctab.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	GETFNAME_TYPELOCAL
#define	GETFNAME_TYPELOCAL	0	/* search locally first */
#define	GETFNAME_TYPEROOT	1	/* search programroot area first */
#endif

#ifndef	VARPATHEXEC
#define	VARPATHEXEC	"PATH"
#endif

#ifndef	VARPATHLIB
#define	VARPATHLIB	"LD_LIBRARY_PATH"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
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
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getserial(const char *) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	isNotPresent(int) ;

extern int	progsvcopen(PROGINFO *) ;
extern int	progsvcclose(PROGINFO *) ;
extern int	progaccopen(PROGINFO *) ;
extern int	progaccclose(PROGINFO *) ;

extern int	proglog_begin(PROGINFO *,USERINFO *) ;
extern int	proglog_end(PROGINFO *) ;
extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

extern int	progwatch(PROGINFO *,vecstr *) ;

extern int	securefile(const char *,uid_t,gid_t) ;
extern int	mkplogid(char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG 
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	progexports(PROGINFO *,const char *) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	proglog_extra(PROGINFO *) ;

static int	procsecurity(PROGINFO *) ;

static int	procaa(PROGINFO *,ARGINFO *) ;

static int	loadserial(PROGINFO *) ;
static int	loadpath(PROGINFO *,vecstr *,cchar *,cchar **,cchar *) ;
static int	loadpathpr(PROGINFO *,vecstr *,const char **) ;
static int	loadpathprdir(PROGINFO *,vecstr *,const char *) ;
static int	loadpathcomp(PROGINFO *,vecstr *,const char *) ;
static int	loadpather(PROGINFO *,vecstr *,const char *,int) ;


/* local variables */

static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static const char	*prlibs[] = {
	"lib",
	NULL
} ;


/* exported subroutines */


int progprocess(PROGINFO *pip,ARGINFO *aip,USERINFO *uip)
{
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progprocess: ent\n") ;
#endif

	if ((rs = loadserial(pip)) >= 0) {
	    if ((rs = proglog_begin(pip,uip)) >= 0) {
		if ((rs = proglog_extra(pip)) >= 0) {
	                if ((rs = progsvcopen(pip)) >= 0) {
	                    if ((rs = procsecurity(pip)) >= 0) {
	                        if ((rs = progaccopen(pip)) >= 0) {
				    {
	                                rs = procaa(pip,aip) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progprocess: procaa() rs=%d\n",rs) ;
#endif
				    }
	                            rs1 = progaccclose(pip) ;
	                            if (rs >= 0) rs = rs1 ;
	                        } /* end if (progacc) */
	                    } /* end if (procsecutiry) */
	                    rs1 = progsvcclose(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } else {
	                    if (pip->debuglevel > 0) {
				cchar	*pn = pip->progname ;
				cchar	*fmt ;
				fmt = "%s: no service table\n" ;
	        		bprintf(pip->efp,fmt,pn) ;
			    }
		        } /* end if (progsvc) */
		} /* end if (proglog_extra) */
		rs1 = proglog_end(pip) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (proglog) */
	} /* end if (loadserial) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progprocess: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progprocess) */


/* local subroutines */


static int proglog_extra(PROGINFO *pip)
{
	const int	f = (pip->f.named || pip->f.passfd) ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	    if ((pip->cfname != NULL) && (rs1 >= 0)) {
	        rs1 = proglog_printf(pip,"cf=%s\n",pip->cfname) ;
	    }
	    if ((! f) && (pip->pidfname != NULL) && (rs1 >= 0)) {
	        rs1 = proglog_printf(pip,"pid=%s\n",pip->pidfname) ;
	    }
	return rs ;
}
/* end subroutine (proglog_extra) */


static int procsecurity(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = TRUE ;

	f = f && pip->f.secure_root ;
	f = f && pip->f.secure_conf ;
	f = f && pip->f.secure_svcfile ;
	f = f && pip->f.secure_path ;
	pip->f.secure = f ;

#if	CF_SETRUID
	if (pip->f.secure) {
	    if (pip->uid != pip->euid) {
	        u_setreuid(pip->euid,-1) ;
		if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: set RUID to uid=%d\n",
	            pip->progname,pip->euid) ;
	    }
	    if (pip->gid != pip->egid)
	        u_setreuid(pip->egid,-1) ;
	}
#endif /* CF_SETRUID */

#if	CF_SETEUID
	if (! pip->f.secure) {
	    if (pip->uid != pip->euid)
	        u_seteuid(pip->uid) ;
	    if (pip->gid != pip->egid)
	        u_setegid(pip->gid) ;
	}
#endif /* CF_SETEUID */

	return rs ;
}
/* end subroutine (procsecurity) */


static int procaa(PROGINFO *pip,ARGINFO *aip)
{
	vecstr		snames ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ai ;
	int		opts ;
	int		count = 0 ;
	int		f ;
	const char	*varpathexec = VARPATHEXEC ;
	const char	*varpathlib = VARPATHLIB ;
	const char	*defpath ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprocess/procaa: ent\n") ;
#endif

	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&pip->pathexec,40,opts) ;
	if (rs < 0)
	    goto ret0 ;

	defpath = pip->defpath ;
	rs = loadpath(pip,&pip->pathexec,varpathexec,prbins,defpath) ;
	if (rs < 0)
	    goto ret1 ;

	rs = vecstr_start(&pip->pathlib,40,opts) ;
	if (rs < 0)
	    goto ret1 ;

	defpath = "/usr/preroot/lib:/usr/extra/lib" ;
	rs = loadpath(pip,&pip->pathlib,varpathlib,prlibs,defpath) ;
	if (rs < 0)
	    goto ret2 ;

	rs = vecstr_start(&snames,0,0) ;
	if (rs < 0)
	    goto ret2 ;

/* load up all of the service names that we have so far */

	if (pip->f.named) {

	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(&aip->pargs,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (! f) continue ;

	        cp = aip->argv[ai] ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progprocess/procaa: svc=%s\n",cp) ;
#endif

	        rs1 = svcfile_fetch(&pip->stab,cp,NULL,NULL,NULL,0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progprocess/procaa: svcfile_fetch() rs=%d\n",
	                rs1) ;
#endif

	        if (rs1 >= 0) {

	            rs = vecstr_adduniq(&snames,cp,-1) ;
	            if (rs != INT_MAX) count += 1 ;

	        } else if (! pip->f.quiet) {

	            rs = SR_NOTFOUND ;
	            bprintf(pip->efp,
	                "%s: unavailable service=%s\n",
	                pip->progname,cp) ;

	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progprocess/procaa: had_svc rs=%d\n",rs1) ;
#endif

	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progprocess/procaa: specified svcs=%u\n",
	            count) ;
#endif

	} /* end if (named services) */

/* do some things if we are running in daemon mode */

	if ((rs >= 0) && (pip->f.daemon || pip->f.background)) {

	    proglog_flush(pip) ;

	    bflush(pip->efp) ;

	    if (pip->debuglevel == 0) {

	        rs = uc_fork() ;
	 	if (rs == 0) {

		    if (pip->f.caf) {
		        int	i ;
			bclose(pip->efp) ;
			pip->efp = NULL ;
			for (i = 0 ; i < 3 ; i += 1)
			    u_close(i) ;
		    }

	            u_setsid() ;

	            pip->pid = getpid() ;

		    pip->spid = pip->pid ;
	            proglog_printf(pip,"backgrounded pid=%d\n",
				pip->pid) ;
	
		} else if (rs > 0)
		    uc_exit(EX_OK) ;

	    } /* end if (backgrounding) */

	} /* end if (daemon mode) */

	if ((rs >= 0) && ((! pip->f.named) || (count > 0))) {

	    rs = progwatch(pip,&snames) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progprocess/procaa: progwatch() rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
	        if (pip->debuglevel > 0) {
	            bprintf(pip->efp,
	                "%s: exceptional (%d)\n",
	                pip->progname,rs) ;
		}
	        proglog_printf(pip,"exceptional (%d)\n",rs) ;
	    } /* end if */

	} /* end if */

	vecstr_finish(&snames) ;

ret2:
	vecstr_finish(&pip->pathlib) ;

ret1:
	vecstr_finish(&pip->pathexec) ;

ret0:
	return (rs >= 0) ? count : rs ;
}
/* end subroutine (procaa) */


static int loadserial(pip)
PROGINFO	*pip ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		serial = -1 ;
	const char	*sfn = SERIALFNAME ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (sfn[0] != '/') {

	    if (serial < 0) {
	        rs1 = mkpath3(tmpfname,pip->pr,VARDNAME,sfn) ;
	        if (rs1 > 0)
	            serial = getserial(tmpfname) ;
	    }

	    if (serial < 0) {
	        rs1 = mkpath3(tmpfname,pip->tmpdname,pip->rootname,sfn) ;
	        if (rs1 > 0)
	            serial = getserial(tmpfname) ;
	    } /* end if */

	    if (serial < 0) {
	        rs1 = mkpath2(tmpfname,pip->tmpdname,sfn) ;
	        if (rs1 > 0)
	            serial = getserial(tmpfname) ;
	    } /* end if */

	} else
	    serial = getserial(sfn) ;

	if (serial >= 0) {
	    pip->serial = (serial % (PID_MAX+1)) ;
	} else
	    pip->serial = pip->pid ;

#if	CF_LOGID
	{
	    const int	llen = LOGIDLEN ;
	    cchar	*nn = pip->nodename ;
	    int		serial = pip->serial ;
	    char	lbuf[LOGIDLEN + 1] ;
	    if ((rs = mkplogid(lbuf,llen,nn,serial)) >= 0) {
		cchar	**vpp = &pip->logid ;
	        rs = proginfo_setentry(pip,vpp,lbuf,rs) ;
	    }
	}
#endif /* CF_LOGID */

	return rs ;
}
/* end subroutine (loadserial) */


static int loadpath(pip,plp,varname,prdirs,defpath)
PROGINFO	*pip ;
vecstr		*plp ;
const char	*varname ;
const char	**prdirs ;
const char	*defpath ;
{
	VECSTR		*elp = &pip->exports ;
	int		rs ;
	int		c = 0 ;
	const char	*pp ;

/* system-administrative environment */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progprocess/loadpath: exports> \n") ;
	    progexports(pip,"loadpath0") ;
	}
#endif

	if ((rs = vecstr_search(elp,varname,vstrkeycmp,&pp)) >= 0) {
	    const char	*tp ;

	    if ((tp = strchr(pp,'=')) != NULL) {
	        rs = loadpathcomp(pip,plp,(tp + 1)) ;
	        c += rs ;
	    }

/* our program root */

	    if (rs >= 0) {
	        rs = loadpathpr(pip,plp,prdirs) ;
	        c += rs ;
	    }

/* system-default path */

	    if ((rs >= 0) && (defpath != NULL)) {
	        rs = loadpathcomp(pip,plp,defpath) ;
	        c += rs ;
	    }

/* process environment */

	    if ((rs >= 0) && ((tp = getenv(varname)) != NULL)) {
	        rs = loadpathcomp(pip,plp,tp) ;
	        c += rs ;
	    }

	} /* end if (search-found) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progprocess/loadpath: ret exports> \n") ;
	    progexports(pip,"loadpath1") ;
	}
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprocess/loadpath: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int loadpathpr(pip,plp,prdirs)
PROGINFO	*pip ;
vecstr		*plp ;
const char	**prdirs ;
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	for (i = 0 ; prdirs[i] != NULL ; i += 1) {
	    rs = loadpathprdir(pip,plp,prdirs[i]) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathpr) */


static int loadpathprdir(pip,plp,bname)
PROGINFO	*pip ;
vecstr		*plp ;
const char	bname[] ;
{
	int		rs ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = mkpath2(tbuf,pip->pr,bname)) >=0) {
	    rs = vecstr_adduniq(plp,tbuf,rs) ;
	    if (rs < INT_MAX) c += 1 ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathprdir) */


static int loadpathcomp(pip,plp,pp)
PROGINFO	*pip ;
vecstr		*plp ;
const char	*pp ;
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*tp ;

	while ((tp = strpbrk(pp,":;")) != NULL) {
	    rs = loadpather(pip,plp,pp,(tp - pp)) ;
	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {
	    rs = loadpather(pip,plp,pp,-1) ;
	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


static int loadpather(pip,plp,pbuf,plen)
PROGINFO	*pip ;
vecstr		*plp ;
const char	pbuf[] ;
int		plen ;
{
	int		rs ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = pathclean(tbuf,pbuf,plen)) > 0) {
	    int	pl = rs ;
	    if ((rs = vecstr_findn(plp,tbuf,pl)) == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(plp,tbuf,pl) ;
	    }
	} /* end if (pathclean) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpather) */


