/* msu-config */

/* handle MSU configuration functions */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines form part of the MSU program (yes, getting
	a little bit more complicated every day now).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"b_msu.h"
#include	"msuconfig.h"
#include	"msulocinfo.h"
#include	"defs.h"


/* local typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


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
#define	DIGBUFLEN	45		/* can hold long128_t in decimal */
#endif

#ifndef	IPCBUFLEN
#define	IPCBUFLEN	MSGBUFLEN
#endif

#define	CMSGBUFLEN	256
#define	NIOVECS		1

#define	DEBUGFNAME	"/tmp/msu.deb"

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif

#define	MSU_REQCNAME	"req"

#undef	TMPDMODE
#define	TMPDMODE	0777


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
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
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	strwcmp(const char *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
#endif

extern char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_loga(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	config_addcooks(CONFIG *) ;

static int	setfname(PROGINFO *,char *,const char *,int,
			int,const char *,const char *,const char *) ;


/* local variables */

static const char	*params[] = {
	"cmd",
	"logsize",
	"msfile",
	"pidfile",
	"runtime",
	"runint",
	"mspoll",
	"pollint",
	"markint",
	"lockint",
	"speedint",
	"logfile",
	"reqfile",
	"speedname",
	NULL
} ;

enum params {
	param_cmd,
	param_logsize,
	param_msfile,
	param_pidfile,
	param_runtime,
	param_intrun,
	param_mspoll,
	param_intpoll,
	param_intmark,
	param_intlock,
	param_intspeed,
	param_logfile,
	param_reqfile,
	param_speedname,
	param_overlast
} ;


/* exported subroutines */


int config_start(cfp,pip,cfname,intcheck)
CONFIG		*cfp ;
PROGINFO	*pip ;
const char	*cfname ;
int		intcheck ;
{
	EXPCOOK		*ckp ;
	int		rs = SR_OK ;

	if (cfp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;
	if (cfname == NULL) return SR_FAULT ;

	if (cfname[0] == '\0') return SR_INVALID ;

/* start in */

	memset(cfp,0,sizeof(CONFIG)) ;
	cfp->pip = pip ;
	cfp->intcheck = intcheck ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("config_start: cfname=%s\n",cfname) ;
	    debugprintf("config_start: nodename=%s\n",pip->nodename) ;
	    debugprintf("config_start: domainname=%s\n",pip->domainname) ;
	}
#endif

	ckp = &cfp->cooks ;
	if ((rs = paramfile_open(&cfp->p,pip->envv,cfname)) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_start: paramfile_open rs=%d\n",rs) ;
#endif
	    if ((rs = expcook_start(ckp)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_start: expcook_start rs=%d\n",rs) ;
#endif
	        if ((rs = config_addcooks(cfp)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_start: config_addcooks rs=%d\n",
				rs) ;
#endif
	            cfp->f.p = TRUE ;
	            rs = config_read(cfp) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("config_start: config_read rs=%d\n",rs) ;
#endif
	        }
	    }
	    if (rs < 0)
	        expcook_finish(ckp) ;
	}
	if (rs < 0)
	    expcook_finish(ckp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


int config_finish(CONFIG *cfp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("config_finish: ent\n") ;
#endif

	if (cfp == NULL) return SR_FAULT ;

	pip = cfp->pip ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_finish: ent %c\n",
		((cfp->f.p) ? '¥' : '_')) ;
#endif

	if (cfp->f.p) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_finish: fins\n") ;
#endif

	    rs1 = expcook_finish(&cfp->cooks) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = paramfile_close(&cfp->p) ;
	    if (rs >= 0) rs = rs1 ;

	} else
	    rs = SR_NOTOPEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_finish) */


int config_check(CONFIG *cfp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (cfp == NULL)
	    return SR_FAULT ;

	pip = cfp->pip ;
	if (cfp->f.p) {
	    time_t	daytime = pip->daytime ;
	    int		intcheck = cfp->intcheck ;
	    int		f_check = FALSE ;

	    f_check = f_check && (intcheck >= 0) ;
	    f_check = f_check && ((daytime - cfp->ti_lastcheck) >= intcheck) ;
	    if (f_check) {
	        cfp->ti_lastcheck = daytime ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("msumain/config_check: paramfile_check()\n") ;
#endif

	        if ((rs = paramfile_check(&cfp->p,daytime)) > 0) {
	            f_changed = TRUE ;
	            rs = config_read(cfp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("msumain/config_check: "
	                    "config_read() rs=%d\n",rs) ;
#endif

	        } /* end if (parameter file changed) */
	    } /* end if (needed a check) */

	} else
	    rs = SR_NOTOPEN ;

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (config_check) */


int config_read(CONFIG *cfp)
{
	PROGINFO	*pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp = &cfp->p ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	const int	elen = EBUFLEN ;
	int		rs = SR_NOTOPEN ;
	int		rs1 ;
	int		pi ;
	int		kl ;
	int		ml, vl, el ;
	int		v ;
	char		pbuf[PBUFLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;
	const char	*ccp ;
	const char	*kp, *vp ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",cfp->f.p) ;
#endif

	if (cfp == NULL)
	    return SR_FAULT ;

	pip = cfp->pip ;
	lip = pip->lip ;
	if (! cfp->f.p)
	    goto ret0 ;

	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,PBUFLEN) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs < 0) break ;

	        kp = pe.key ;
	        vp = pe.value ;
	        vl = pe.vlen ;

	        pi = matostr(params,2,kp,kl) ;
	        if (pi < 0) continue ;

	        ebuf[0] = '\0' ;
	        el = 0 ;
	        if (vl > 0) {
	            el = expcook_exp(&cfp->cooks,0,ebuf,elen,vp,vl) ;
	            if (el >= 0)
	                ebuf[el] = '\0' ;
	        } /* end if */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
	            debugprintf("config_read: param=%s(%u)\n",
	                params[pi],pi) ;
	        }
#endif

	        if (el < 0)
	            continue ;

	        switch (pi) {

	        case param_logsize:
	            if (el > 0) {
	                rs1 = cfdecmfi(ebuf,el,&v) ;
	                if (rs1 >= 0)
	                    pip->logsize = v ;
	            }
	            break ;

	        case param_intrun:
	        case param_intpoll:
	        case param_intmark:
	        case param_intlock:
	        case param_intspeed:
	            rs1 = cfdecti(ebuf,el,&v) ;

	            if ((rs1 >= 0) && (v >= 0)) {

	                switch (pi) {

	                case param_intrun:
	                    if (! pip->final.intrun)
	                        pip->intrun = v ;
	                    break ;

	                case param_mspoll:
	                case param_intpoll:
	                    if (! pip->final.intpoll)
	                        pip->intpoll = v ;
	                    break ;

	                case param_intmark:
	                    if (! pip->final.intmark)
	                        pip->intmark = v ;
	                    break ;

	                case param_intlock:
	                    if (! pip->final.intlock)
	                        pip->intlock = v ;
	                    break ;

	                case param_intspeed:
			    if (! lip->final.intspeed)
	                        lip->intspeed = v ;
	                    break ;

	                } /* end switch */

	            } /* end if (valid number) */
	            break ;

	        case param_pidfile:
	            if (! pip->final.pidfname) {
	                pip->have.pidfname = TRUE ;
	                rs1 = setfname(pip,tbuf,ebuf,el,TRUE,
	                    RUNDNAME,pip->nodename,PIDFNAME) ;
	                ccp = pip->pidfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    const char	**vpp = &pip->pidfname ;
	                    pip->changed.pidfname = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                }
	            }
	            break ;

	        case param_msfile:
	            if (! lip->final.msfname) {
	                lip->have.msfname = TRUE ;
	                rs1 = setfname(pip,tbuf,ebuf,el,TRUE,
	                    MSDNAME,MSFNAME,"") ;
	                ccp = lip->msfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    const char	**vpp = &lip->msfname ;
	                    lip->changed.msfname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,tbuf,rs1) ;
	                }
	            }
	            break ;

	        case param_logfile:
	            if (! pip->final.logprog) {
	                pip->have.logprog = TRUE ;
	                rs1 = setfname(pip,tbuf,ebuf,el,TRUE,
	                    LOGDNAME,pip->searchname,"") ;
	                ccp = pip->lfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    const char	**vpp = &pip->lfname ;
	                    pip->changed.logprog = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                }
	            } /* end if */
	            break ;

	        case param_reqfile:
	            if (! lip->final.reqfname) {
	                lip->have.reqfname = TRUE ;
#ifdef	COMMENT
	                rs1 = setfname(pip,tbuf,ebuf,el,TRUE,
	                    LOGDNAME,pip->searchname,"") ;
#else			    
	                rs1 = mkpath1w(tbuf,ebuf,el) ;
#endif /* COMMENT */
	                ccp = lip->reqfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    const char	**vpp = &lip->reqfname ;
	                    lip->changed.reqfname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,tbuf,rs1) ;
	                }
	            } /* end if */
	            break ;

	        case param_cmd:
	            ml = MIN(LOGIDLEN,el) ;
	            if (ml && (lip->cmd[0] == '\0'))
	                strwcpy(lip->cmd,ebuf,ml) ;
	            break ;

	        case param_speedname:
	            if (! lip->final.speedname) {
	                lip->have.speedname = TRUE ;
	                ccp = lip->speedname ;
	                if ((ccp == NULL) ||
	                    (strwcmp(ccp,ebuf,el) != 0)) {
			    const char	**vpp = &lip->speedname ;
	                    lip->changed.speedname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                }

	            } /* end if */
	            break ;

	        } /* end switch */

	    } /* end while (fetching) */

	    rs1 = paramfile_curend(&cfp->p,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (parameters) */

ret0:
	return rs ;
}
/* end subroutine (config_read) */


/* private subroutines */


static int config_addcooks(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	EXPCOOK		*ckp = &cfp->cooks ;
	int		rs = SR_OK ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"P",pip->progname ,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"S",pip->searchname,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"N",pip->nodename,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"D",pip->domainname,-1) ;

	if (rs >= 0) {
	    int		hnl ;
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    char	hostname[MAXHOSTNAMELEN + 1] ;
	    hnl = snsds(hostname,MAXHOSTNAMELEN,nn,dn) ;
	    rs = expcook_add(ckp,"H",hostname,hnl) ;
	}

	if (rs >= 0)
	    rs = expcook_add(ckp,"R",pip->pr,-1) ;

	if (rs >= 0)
	    rs = expcook_add(ckp,"U",pip->username,-1) ;

	return rs ;
}
/* end subroutine (config_addcooks) */


/* calculate a file name */
static int setfname(pip,fname,ebuf,el,f_def,dname,name,suf)
PROGINFO	*pip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int		rs = 0 ;
	int		ml ;
	int		fl = 0 ;
	const char	*np ;
	char		tmpname[MAXNAMELEN + 1] ;

	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {

	        np = tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;

	    }

	    if (np[0] != '/') {

	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pip->pr,dname,np) ;
	        } else
	            rs = mkpath2(fname, pip->pr,np) ;

	    } else
	        rs = mkpath1(fname, np) ;

	    fl = rs ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = ebuf ;
	    if (el >= 0) {

	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;

	    }

	    if (ebuf[0] != '/') {

	        if (strchr(np,'/') != NULL) {

	            rs = mkpath2(fname,pip->pr,np) ;

	        } else {

	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,pip->pr,dname,np) ;
	            } else
	                rs = mkpath2(fname,pip->pr,np) ;

	        }

	    } else
	        rs = mkpath1(fname,np) ;

	    fl = rs ;

	} /* end if */

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (setfname) */


