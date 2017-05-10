/* progconfig */

/* program configuration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2008-10-10, David A­D­ Morano

	This was adapted from the BACKGROUND program.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This module contains the subroutines that manage program
	configuration.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<paramfile.h>
#include	<expcook.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"stat32.h"
#include	"listenspec.h"



/* local defines */

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

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
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecmfu(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;

extern int	securefile(const char *,uid_t,gid_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		progconfigread(struct proginfo *) ;

static int	proclistenadd(struct proginfo *,vecobj *,const char *,int) ;
static int	proclistenmerge(struct proginfo *,vecobj *) ;
static int	proclistenpresent(struct proginfo *,vecobj *,LISTENSPEC *) ;
static int	proclistentmpdel(struct proginfo *,vecobj *,int) ;
static int	proclistenfree(struct proginfo *,vecobj *) ;

static int	setfname(struct proginfo *,char *,const char *,int,int,
			const char *,const char *,const char *) ;


/* local variables */

static const char	*schedpconf[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"stampdir",
	"logsize",
	"reqfile",
	"pidfile",
	"logfile",
	"svcfile",
	"accfile",
	"passfile",
	"usersrv",
	"listen",
	"runint",
	"pollint",
	"markint",
	"lockint",
	"cmd",
	"orgcode",
	"defsvc",
	"torecvfd",
	"tosendfd",
	"useracct",
	NULL
} ;

enum params {
	param_stampdir,
	param_logsize,
	param_reqfile,
	param_pidfile,
	param_logfile,
	param_svcfile,
	param_accfile,
	param_passfile,
	param_usersrv,
	param_listen,
	param_runint,
	param_pollint,
	param_markint,
	param_lockint,
	param_cmd,
	param_orgcode,
	param_defsvc,
	param_torecvfd,
	param_tosendfd,
	param_useracct,
	param_overlast
} ;


/* exported subroutines */


int progconfiginit(pip,sched,configfname)
struct proginfo	*pip ;
const char	*sched[] ;
const char	configfname[] ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	f_secreq ;

	const char	**schedp ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (configfname == NULL)
	    return SR_FAULT ;

/* look for configuration file */

	f_secreq = (! pip->f.proglocal) ;
	schedp = (sched != NULL) ? sched : schedpconf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	i ;
	    debugprintf("progconfiginit: search-schedule:\n") ;
	    for (i = 0 ; schedp[i] != NULL ; i += 1)
	        debugprintf("progconfiginit: sched%u=%s\n",i,schedp[i]) ;
	}
#endif /* CF_DEBUG */

	rs1 = SR_NOENT ;
	tmpfname[0] = '\0' ;
	if (strchr(configfname,'/') == NULL) {

	    f_secreq = FALSE ;
	    rs1 = permsched(schedp,&pip->svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progconfiginit: permsched() rs=%d \n",rs1) ;
	        debugprintf("progconfiginit: tmpfname=%s\n", tmpfname) ;
	    }
#endif

	    if (rs1 == 0)
	        rs = mkpath1(tmpfname,configfname) ;

	} else {
	    rs1 = SR_OK ;
	    if (configfname[0] == '/') {
	        rs = mkpath1(tmpfname,configfname) ;
	    } else
	        rs = mkpath2(tmpfname,pip->pr,configfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfiginit: mid rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfiginit: configfname=%s\n",tmpfname) ;
#endif

	if ((rs1 >= 0) && (tmpfname[0] != '\0')) {

	    rs = proginfo_setentry(pip,&pip->configfname,tmpfname,-1) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progconfiginit: paramfile_open() rs=%d\n",
			rs) ;
	            debugprintf("progconfiginit: f=%s\n",tmpfname) ;
	        }
#endif

	        rs1 = paramfile_open(&pip->params,pip->envv, tmpfname) ;
	        pip->open.params = (rs1 >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progconfiginit: paramfile_open() rs=%d\n",
			rs1) ;
#endif

	    } /* end if */

	} /* end if */

	if (rs >= 0) {
	    pip->f.secure_conf = pip->f.secure_root ;
	    if (f_secreq || (! pip->f.secure_conf)) {

	        rs1 = securefile(pip->configfname,pip->euid,pip->egid) ;
	        pip->f.secure_conf = (rs1 > 0) ;

	    }
	}

	if (rs >= 0)
	    pip->f.pc = TRUE ;

	if ((rs >= 0) && pip->open.params)
	    rs = progconfigread(pip) ;

	if (rs < 0)
	    goto bad4 ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfiginit: ret rs=%d f_pc=%u\n",rs,pip->f.pc) ;
#endif

	return rs ;

/* bad stuff */
bad4:
bad3:
bad2:
	if (pip->open.params) {
	    pip->open.params = FALSE ;
	    paramfile_close(&pip->params) ;
	}

bad1:
	pip->f.pc = FALSE ;
	pip->open.params = FALSE ;

bad0:
	goto ret0 ;
}
/* end subroutine (progconfiginit) */


int progconfigcheck(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	f = FALSE ;


	if (pip->f.pc && pip->open.params) {

	    rs = paramfile_check(&pip->params,pip->daytime) ;
	    if (rs > 0) {
	        f = TRUE ;
	        rs = progconfigread(pip) ;
	    }

	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progconfigcheck) */


int progconfigfree(pip)
struct proginfo	*pip ;
{
	int	rs = SR_NOTOPEN ;


	if (! pip->f.pc)
	    goto ret0 ;

	rs = SR_OK ;
	if (pip->open.params) {
	    pip->open.params = FALSE ;
	    rs = paramfile_close(&pip->params) ;
	}

ret0:
	return rs ;
}
/* end subroutine (progconfigfree) */


int progconfigread(pip)
struct proginfo	*pip ;
{
	PARAMFILE_CUR	cur ;

	PARAMFILE_ENT		pe ;

	vecobj	tmplistens ;

	const int	elen = EBUFLEN ;

	int	rs = SR_OK ;
	int	rs1 = 0 ;
	int	size ;
	int	pi ;
	int	kl ;
	int	vl ;
	int	el ;
	int	tl ;
	int	v ;
	int	f ;

	const char	*kp, *vp, *ep ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	pbuf[PBUFLEN + 1] ;
	char	ebuf[EBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigread: f_pc=%u open_params=%u\n",
	        pip->f.pc,pip->open.params) ;
#endif

	if (! pip->f.pc)
	    goto ret0 ;

	if (! pip->open.params)
	    goto ret0 ;

	pip->changed.pc = TRUE ;
	size = sizeof(LISTENSPEC) ;
	rs = vecobj_start(&tmplistens,size,6,0) ;
	if (rs < 0)
	    goto ret0 ;

	paramfile_curbegin(&pip->params,&cur) ;

	while (rs >= 0) {

	    kl = paramfile_enum(&pip->params,&cur,&pe,pbuf,PBUFLEN) ;
	    if (kl == SR_NOTFOUND)
		break ;

	    rs = kl ;
 	    if (rs < 0)
		break ;

	    kp = pe.key ;
	    vp = pe.value ;
	    vl = pe.vlen ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progconfigread: enum k=%t\n",kp,kl) ;
#endif

	    pi = matpstr(params,2,kp,kl) ;

	    if (pi < 0) continue ;

	    ebuf[0] = '\0' ;
	    ep = ebuf ;
	    el = 0 ;
	    if (vl > 0) {
	        el = expcook_exp(&pip->cooks,0,ebuf,elen,vp,vl) ;
	        if (el >= 0) {
	            ebuf[el] = '\0' ;
		}
	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progconfigread: ebuf=>%t<\n",ebuf,el) ;
	        debugprintf("progconfigread: switch=%s(%u)\n",
	            params[i],i) ;
	    }
#endif

	    if (el < 0)
	        continue ;

	    switch (pi) {

	    case param_logsize:
	        if ((elen > 0) && (! pip->final.logsize)) {
	            rs1 = cfdecmfi(ebuf,el,&v) ;
	            if ((rs1 >= 0) && (v >= 0)) {
	                pip->have.logsize = TRUE ;
	                pip->changed.logsize = TRUE ;
	                pip->logsize = v ;
	            }
	        }
	        break ;

	    case param_runint:
	    case param_pollint:
	    case param_markint:
	    case param_lockint:
	    case param_torecvfd:
	    case param_tosendfd:
	        v = -1 ;
	        if (el > 0)
	            rs1 = cfdecti(ebuf,el,&v) ;

	        if ((rs1 >= 0) && (v >= 0)) {

	            switch (pi) {

	            case param_pollint:
	                if (! pip->final.intpoll) {
	                    pip->have.intpoll = TRUE ;
	                    pip->changed.intpoll = TRUE ;
	                    pip->intpoll = v ;
	                }
	                break ;

	            case param_lockint:
	                if (! pip->final.intlock) {
	                    pip->have.intlock = TRUE ;
	                    pip->changed.intlock = TRUE ;
	                    pip->intlock = v ;
	                }
	                break ;

	            case param_markint:
	                if (! pip->final.intmark) {
	                    pip->have.intmark = TRUE ;
	                    pip->changed.intmark = TRUE ;
	                    pip->intmark = v ;
	                }
	                break ;

	            case param_runint:
	                if (! pip->final.intrun) {
	                    pip->have.intrun = TRUE ;
	                    pip->changed.intrun = TRUE ;
	                    pip->intrun = v ;
	                }
	                break ;

	            case param_torecvfd:
	                if (! pip->final.torecvfd) {
	                    pip->have.torecvfd = TRUE ;
	                    pip->changed.torecvfd = TRUE ;
	                    pip->to_recvfd = v ;
	                }
	                break ;

	            case param_tosendfd:
	                if (! pip->final.tosendfd) {
	                    pip->have.tosendfd = TRUE ;
	                    pip->changed.tosendfd = TRUE ;
	                    pip->to_sendfd = v ;
	                }
	                break ;

	            } /* end switch */

	        } /* end if (valid number) */

	        break ;

	    case param_reqfile:
	        if (! pip->final.reqfile) {
	            char	dname[MAXPATHLEN + 1] ;
	            pip->have.reqfile = TRUE ;
	            mkpath2(dname,VARDNAME,pip->searchname) ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                dname,pip->nodename,REQFEXT) ;
	            f = (pip->reqfname == NULL) ;
	            f = f || (strcmp(pip->reqfname,tmpfname) != 0) ;
	            if (f) {
	                pip->changed.reqfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->reqfname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_pidfile:
	        if (! pip->final.pidfile) {
	            pip->have.pidfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                RUNDNAME,pip->nodename,pip->searchname) ;
	            f = (pip->pidfname == NULL) ;
	            f = f || (strcmp(pip->pidfname,tmpfname) != 0) ;
	            if (f) {
	                pip->changed.pidfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->pidfname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_logfile:
	        if (! pip->final.logfile) {
	            pip->have.logfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                LOGDNAME,pip->searchname,"") ;
	            f = (pip->logfname == NULL) ;
	            f = f || (strcmp(pip->logfname,tmpfname) != 0) ;
	            if (f) {
	                pip->fromconf.logfile = TRUE ;
	                pip->changed.logfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->logfname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_svcfile:
	        if (! pip->final.svcfile) {
	            pip->have.svcfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                ETCDNAME,pip->searchname,SVCFEXT) ;
	            f = (pip->svcfname == NULL) ;
	            f = f || (strcmp(pip->svcfname,tmpfname) != 0) ;
	            if (f) {
	                pip->fromconf.svcfile = TRUE ;
	                pip->changed.svcfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->svcfname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_accfile:
	        if (! pip->final.accfile) {
	            pip->have.accfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                ETCDNAME,pip->searchname,"") ;
	            f = (pip->accfname == NULL) ;
	            f = f || (strcmp(pip->accfname,tmpfname) != 0) ;
	            if (f) {
	                pip->fromconf.accfile = TRUE ;
	                pip->changed.accfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->accfname,
	                    tmpfname,tl) ;
	            }
	        }
		break ;

	    case param_passfile:
	        if (! pip->final.passfile) {
	            pip->have.passfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                ETCDNAME,pip->searchname,"") ;
	            f = (pip->passfname == NULL) ;
	            f = f || (strcmp(pip->passfname,tmpfname) != 0) ;
	            if (f) {
	                pip->fromconf.passfile = TRUE ;
	                pip->changed.passfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->passfname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_usersrv:
	        if (! pip->final.usersrv) {
	            pip->have.usersrv = TRUE ;
	            f = (pip->usersrv == NULL) ;
	            f = f || (strwcmp(pip->usersrv,ep,el) != 0) ;
	            if (f) {
	                pip->fromconf.usersrv = TRUE ;
	                pip->changed.usersrv = TRUE ;
	                rs = proginfo_setentry(pip,&pip->usersrv,
	                    ep,el) ;
	            }
	        }
	        break ;

	    case param_stampdir:
	        if (! pip->final.stampdir) {
	            pip->have.stampdir = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,el,TRUE,
	                VARDNAME,STAMPDNAME,"") ;
	            f = (pip->stampdname == NULL) ;
	            f = f || (strcmp(pip->stampdname,tmpfname) != 0) ;
	            if (f) {
	                pip->changed.stampdir = TRUE ;
	                rs = proginfo_setentry(pip,&pip->stampdname,
	                    tmpfname,tl) ;
	            }
	        }
	        break ;

	    case param_listen:

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progconfigread: listen \n") ;
	            debugprintf("progconfigread: listen ebuf=>%t<\n",
	                ebuf,el) ;
	        }
#endif

	        if (el > 0)
	            rs = proclistenadd(pip,&tmplistens,ebuf,el) ;

	        break ;

	    case param_cmd:
	        if ((el > 0) && (pip->cmd[0] == '\0')) {
	            pip->have.cmd = TRUE ;
	            pip->changed.cmd = TRUE ;
	            strwcpy(pip->cmd,ebuf,MIN(LOGIDLEN,el)) ;
	        }
	        break ;

	    case param_orgcode:
	        pip->have.orgcode = TRUE ;
	        if (el > 0) {
	            pip->changed.orgcode = TRUE ;
	            strwcpy(pip->orgcode,ebuf,MIN(LOGIDLEN,el)) ;
	        }
	        break ;

	    case param_defsvc:
	        pip->have.defsvc = TRUE ;
	        if (el > 0) {
	            pip->changed.defsvc = TRUE ;
	            strwcpy(pip->defsvc,ebuf,MIN(SVCNAMELEN,el)) ;
	        }
	        break ;

	    case param_useracct:
	        pip->have.useracct = TRUE ;
		if (vl > 0) {
		    rs = optbool(ep,el) ;
		    pip->f.useracct = (rs > 0) ;
		}
		break ;

	    } /* end switch */

	    if (rs < 0)
	        break ;

	} /* end while (enumerating) */

	paramfile_curend(&pip->params,&cur) ;

ret3:
	if (rs >= 0)
	    rs = proclistenmerge(pip,&tmplistens) ;

ret2:
	proclistenfree(pip,&tmplistens) ;

ret1:
	vecobj_finish(&tmplistens) ;

ret0:
	return rs ;
}
/* end subroutine (progconfigread) */


/* local subroutines */


/* add a listener-specification to a temporary listen */
static int proclistenadd(pip,tlp,ebuf,elen)
struct proginfo	*pip ;
vecobj		*tlp ;
const char	ebuf[] ;
int		elen ;
{
	LISTENSPEC	ls ;

	int	rs = SR_OK ;
	int	n ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenadd: ebuf=>%t<\n",
	        ebuf,elen) ;
#endif

	rs = listenspec_init(&ls,ebuf,elen) ;
	n = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenadd: listenspec_init() rs=%d\n",
	        rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (n > 0) {
	    rs = vecobj_add(tlp,&ls) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenadd: vecobj_add() rs=%d\n",
		rs) ;
#endif

	}

	if ((n == 0) || (rs < 0))
	    listenspec_free(&ls) ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (proclistenadd) */


static int proclistenmerge(pip,tlp)
struct proginfo	*pip ;
vecobj		*tlp ;
{
	LISTENSPEC	*lsp ;
	LISTENSPEC	*tlsp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenmerge: entered\n") ;
#endif

/* phase-1 */

	for (i = 0 ; vecobj_get(&pip->listens,i,&lsp) >= 0 ; i += 1) {
	    if (lsp == NULL) continue ;

	    rs1 = proclistenpresent(pip,tlp,lsp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenmerge: "
		"proclistenpresent() rs1=%d\n",rs1) ;
#endif

	    if (rs1 >= 0)
	        proclistentmpdel(pip,tlp,rs1) ;
	    else
	        listenspec_delset(lsp,TRUE) ;

	} /* end for */

/* phase-2 */

	for (i = 0 ; vecobj_get(tlp,i,&tlsp) >= 0 ; i += 1) {
	    if (tlsp == NULL) continue ;

	    rs1 = listenspec_delmarked(tlsp) ; /* cannot happen! */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenmerge: "
		"listenspec_delmarked() rs1=%d\n",rs1) ;
#endif

	    if (rs1 <= 0) {
	        rs = vecobj_add(&pip->listens,tlsp) ;
	        vecobj_del(tlp,i--) ;
	    }
	    if (rs < 0)
	        break ;

	} /* end for */

	return rs ;
}
/* end subroutine (proclistenmerge) */


static int proclistenpresent(pip,tlp,lsp)
struct proginfo	*pip ;
vecobj		*tlp ;
LISTENSPEC	*lsp ;
{
	LISTENSPEC	*tlsp ;

	int	rs ;
	int	rs1 ;
	int	i = 0 ;


	for (i = 0 ; (rs = vecobj_get(tlp,i,&tlsp)) >= 0 ; i += 1) {
	    if (tlsp == NULL) continue ;
	    rs1 = listenspec_issame(lsp,tlsp) ;
	    if (rs1 > 0)
	        break ;
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (proclistenpresent) */


static int proclistentmpdel(pip,tlp,ei)
struct proginfo	*pip ;
vecobj		*tlp ;
int		ei ;
{
	LISTENSPEC	*tlsp = NULL ;

	int	rs ;


	rs = vecobj_get(tlp,ei,&tlsp) ;

	if ((rs >= 0) && (tlsp != NULL)) {
	    listenspec_free(tlsp) ;
	    vecobj_del(tlp,ei) ;
	} /* end if */

	return rs ;
}
/* end subroutine (proclistentmpdel) */


static int proclistenfree(pip,tlp)
struct proginfo	*pip ;
vecobj		*tlp ;
{
	LISTENSPEC	*lsp ;

	int	i ;


	for (i = 0 ; vecobj_get(tlp,i,&lsp) >= 0 ; i += 1) {
	    if (lsp == NULL) continue ;
	    listenspec_free(lsp) ;
	}

	return SR_OK ;
}
/* end subroutine (proclistenfree) */


/* calculate a file name */
static int setfname(pip,fname,ebuf,el,f_def,dname,name,suf)
struct proginfo	*pip ;
char		fname[] ;
const char	ebuf[] ;
const char	dname[], name[], suf[] ;
int		el ;
int		f_def ;
{
	int	rs = 0 ;
	int	ml ;

	char	tmpname[MAXNAMELEN + 1], *np ;


	if ((f_def && (ebuf[0] == '\0')) ||
	    (strcmp(ebuf,"+") == 0)) {

	    np = (char *) name ;
	    if ((suf != NULL) && (suf[0] != '\0')) {

	        np = (char *) tmpname ;
	        mkfnamesuf1(tmpname,name,suf) ;

	    }

	    if (np[0] != '/') {

	        if ((dname != NULL) && (dname[0] != '\0'))
	            rs = mkpath3(fname,pip->pr,dname,np) ;

	        else
	            rs = mkpath2(fname, pip->pr,np) ;

	    } else
	        rs = mkpath1(fname, np) ;

	} else if (strcmp(ebuf,"-") == 0) {

	    fname[0] = '\0' ;

	} else if (ebuf[0] != '\0') {

	    np = (char *) ebuf ;
	    if (el >= 0) {

	        np = tmpname ;
	        ml = MIN(MAXPATHLEN,el) ;
	        strwcpy(tmpname,ebuf,ml) ;

	    }

	    if (ebuf[0] != '/') {

	        if (strchr(np,'/') != NULL) {

	            rs = mkpath2(fname,pip->pr,np) ;

	        } else {

	            if ((dname != NULL) && (dname[0] != '\0'))
	                rs = mkpath3(fname,pip->pr,dname,np) ;

	            else
	                rs = mkpath2(fname,pip->pr,np) ;

	        }

	    } else
	        rs = mkpath1(fname,np) ;

	} /* end if */

	return rs ;
}
/* end subroutine (setfname) */



