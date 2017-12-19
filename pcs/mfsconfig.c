/* mfs-config */

/* handle MFS configuration functions */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines form part of the MFS program (yes, getting a little
        bit more complicated every day now).


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
#include	<prsetfname.h>
#include	<localmisc.h>

#include	"mfsmain.h"
#include	"mfsconfig.h"
#include	"mfslocinfo.h"
#include	"mfslisten.h"
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
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	DEBUGFNAME	"/tmp/mfs.deb"

#define	MFS_REQCNAME	"req"


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
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	strwcmp(const char *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	config_reader(CONFIG *,MFSLISTEN_ACQ *,char *,char *,char *) ;


/* local variables */

static cchar	*params[] = {
	"cmd",
	"logsize",
	"msfile",
	"pidfile",
	"logfile",
	"reqfile",
	"builtin",
	"svctab",
	"acctab",
	"runtime",
	"runint",
	"mspoll",
	"pollint",
	"markint",
	"lockint",
	"speedint",
	"speedname",
	"listen",
	"svctype",
	"users",
	NULL
} ;

enum params {
	param_cmd,
	param_logsize,
	param_msfile,
	param_pidfile,
	param_logfile,
	param_reqfile,
	param_builtin,
	param_svctab,
	param_acctab,
	param_runtime,
	param_intrun,
	param_mspoll,
	param_intpoll,
	param_intmark,
	param_intlock,
	param_intspeed,
	param_speedname,
	param_listen,
	param_svctype,
	param_users,
	param_overlast
} ;


/* exported subroutines */


int config_start(CONFIG *cfp,PROGINFO *pip,cchar *cfname,int intcheck)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ent cfname=%s intcheck=%d\n",
			cfname,intcheck) ;
#endif

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

	if (lip->open.cooks) {
	    PARAMFILE	*pfp = &cfp->p ;
	    if ((rs = paramfile_open(pfp,pip->envv,cfname)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_start: paramfile_open rs=%d\n",rs) ;
#endif
	        cfp->f.p = TRUE ;
	        rs = config_read(cfp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_start: config_read rs=%d\n",rs) ;
#endif
		if (rs < 0) {
		    cfp->f.p = FALSE ;
		    paramfile_close(pfp) ;
		}
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d\n",rs) ;
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
	    PARAMFILE	*pfp = &cfp->p ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_finish: fins\n") ;
#endif
	    rs1 = paramfile_close(pfp) ;
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

	if (cfp == NULL) return SR_FAULT ;
	pip = cfp->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfsmain/config_check: ent f_p=%u\n",cfp->f.p) ;
#endif

	if (cfp->f.p) {
	    const time_t	dt = pip->daytime ;
	    const int		intcheck = cfp->intcheck ;
	    int			f_check = TRUE ;

	    f_check = f_check && (intcheck > 0) ;
	    f_check = f_check && ((dt - cfp->ti_lastcheck) >= intcheck) ;
	    if (f_check) {
	        cfp->ti_lastcheck = dt ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mfsmain/config_check: paramfile_check()\n") ;
#endif

	        if ((rs = paramfile_check(&cfp->p,dt)) > 0) {
	            f_changed = TRUE ;
	            rs = config_read(cfp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("mfsmain/config_check: "
	                    "config_read() rs=%d\n",rs) ;
#endif

	        } /* end if (parameter file changed) */
	    } /* end if (needed a check) */

	} else {
	    rs = SR_NOTOPEN ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfsmain/config_check: ret rs=%d f=%u\n",
		rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (config_check) */


int config_read(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (cfp->f.p) {
	    MFSLISTEN_ACQ	acq ;
	    if ((rs = mfslisten_acqbegin(pip,&acq)) >= 0) {
	        int	size = 0 ;
	        char	*a ;
	        size += (PBUFLEN+1) ;
	        size += (EBUFLEN+1) ;
	        size += (MAXPATHLEN+1) ;
	        if ((rs = uc_malloc(size,&a)) >= 0) {
		    {
			char	*bp = a ;
		        char	*pbuf ;
		        char	*ebuf ;
		        char	*tbuf ;
			pbuf = bp ; bp += (PBUFLEN+1) ;
			ebuf = bp ; bp += (EBUFLEN+1) ;
			tbuf = bp ; bp += (MAXPATHLEN+1) ;
		        rs = config_reader(cfp,&acq,pbuf,ebuf,tbuf) ;
	            }
		    rs1 = uc_free(a) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (m-a-f) */
	        rs1 = mfslisten_acqend(pip,&acq) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mfslisten-acq) */
	} /* end if (active) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (config_read) */


/* private subroutines */


int config_reader(CONFIG *cfp,MFSLISTEN_ACQ *acp,
		char *pbuf,char *ebuf,char *tbuf)
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO		*lip ;
	PARAMFILE	*pfp = &cfp->p ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_reader: ent f_p=%u\n",cfp->f.p) ;
#endif

	lip = pip->lip ;
	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    EXPCOOK	*ecp = &lip->cooks ;
	    const int	elen = EBUFLEN ;
	    const int	plen = PBUFLEN ;
	    int		pi ;
	    int		kl ;
	    int		ml, vl, el ;
	    int		v ;
	    cchar	*pr = pip->pr ;
	    cchar	*ccp ;
	    cchar	*kp, *vp ;

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
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
	            if ((el = expcook_exp(ecp,0,ebuf,elen,vp,vl)) >= 0) {
	                ebuf[el] = '\0' ;
		    }
	        } /* end if */
	        if (el < 0) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
	            debugprintf("config_read: param=%s(%u)\n",
	                params[pi],pi) ;
	        }
#endif


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
	            if ((rs = cfdecti(ebuf,el,&v)) >= 0) {
	                switch (pi) {
	                case param_intrun:
	                    if (! pip->final.intrun) pip->intrun = v ;
	                    break ;
	                case param_mspoll:
	                case param_intpoll:
	                    if (! pip->final.intpoll) pip->intpoll = v ;
	                    break ;
	                case param_intmark:
	                    if (! pip->final.intmark) pip->intmark = v ;
	                    break ;
	                case param_intlock:
	                    if (! pip->final.intlock) pip->intlock = v ;
	                    break ;
	                case param_intspeed:
			    if (! lip->final.intspeed) lip->intspeed = v ;
	                    break ;
	                } /* end switch */
	            } /* end if (cfdectinumber) */
	            break ;
	        case param_msfile:
	            if (! lip->final.msfname) {
	                lip->have.msfname = TRUE ;
	                rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                    MSDNAME,MSFNAME,"") ;
	                ccp = lip->msfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    cchar	**vpp = &lip->msfname ;
	                    lip->changed.msfname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,tbuf,rs1) ;
	                }
	            }
	            break ;
	        case param_pidfile:
	            if (! pip->final.pidfname) {
	                pip->have.pidfname = TRUE ;
	                rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                    RUNDNAME,pip->nodename,PIDFNAME) ;
	                ccp = pip->pidfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    cchar	**vpp = &pip->pidfname ;
	                    pip->changed.pidfname = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                }
	            }
	            break ;
	        case param_logfile:
	            if (! pip->final.logprog) {
	                pip->have.logprog = TRUE ;
	                rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                    LOGDNAME,pip->searchname,"") ;
	                ccp = pip->lfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    cchar	**vpp = &pip->lfname ;
	                    pip->changed.logprog = TRUE ;
	                    rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                }
	            } /* end if */
	            break ;
	        case param_reqfile:
	            if (! lip->final.reqfname) {
	                lip->have.reqfname = TRUE ;
#ifdef	COMMENT
	                rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,
	                    LOGDNAME,pip->searchname,"") ;
#else			    
	                rs1 = mkpath1w(tbuf,ebuf,el) ;
#endif /* COMMENT */
	                ccp = lip->reqfname ;
	                if ((ccp == NULL) ||
	                    (strcmp(ccp,tbuf) != 0)) {
			    cchar	**vpp = &lip->reqfname ;
	                    lip->changed.reqfname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,tbuf,rs1) ;
	                }
	            } /* end if */
	            break ;
	        case param_svctab:
	            if (! lip->final.svcfname) {
			cchar	**vpp = &lip->svcfname ;
	                lip->final.svcfname = TRUE ;
	                rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	            }
	            break ;
	        case param_acctab:
	            if (! lip->final.accfname) {
			cchar	**vpp = &lip->accfname ;
	                lip->final.accfname = TRUE ;
	                rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	            }
	            break ;
	        case param_cmd:
	            ml = MIN(LOGIDLEN,el) ;
	            if (ml && (lip->cmd[0] == '\0')) {
	                strwcpy(lip->cmd,ebuf,ml) ;
		    }
	            break ;
	        case param_speedname:
	            if (! lip->final.speedname) {
	                lip->have.speedname = TRUE ;
	                ccp = lip->speedname ;
	                if ((ccp == NULL) ||
	                    (strwcmp(ccp,ebuf,el) != 0)) {
			    cchar	**vpp = &lip->speedname ;
	                    lip->changed.speedname = TRUE ;
	                    rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                }
	            } /* end if */
	            break ;
	       case param_listen:
	            if (pip->f.daemon && (el > 0)) {
	                rs = mfslisten_acqadd(pip,acp,ebuf,el) ;
		    }
		    break ;
	       case param_svctype:
	            if (pip->f.daemon && (el > 0)) {
			if (! lip->final.svctype) {
	                    rs = locinfo_svctype(lip,ebuf,el) ;
			}
		    }
		    break ;
	       case param_users:
	            if (! lip->final.users) {
			lip->final.users = TRUE ;
			lip->f.users = TRUE ;
			if (el > 0) {
	                    rs = optbool(ebuf,el) ;
			    lip->f.users = (rs > 0) ;
			}
		    }
		    break ;
	        } /* end switch */

	    } /* end while (fetching) */

	    rs1 = paramfile_curend(&cfp->p,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramfile-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_reader: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_reader) */


