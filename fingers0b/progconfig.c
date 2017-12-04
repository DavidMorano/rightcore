/* progconfig */

/* program configuration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_LISTEN	1		/* try to listen */


/* revision history:

	= 2008-10-10, David A­D­ Morano
	This was adapted from the BACKGROUND program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines that manage program configuration.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<paramfile.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<listenspec.h>
#include	<localmisc.h>

#include	"prsetfname.h"
#include	"config.h"
#include	"defs.h"


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
extern int	strwcmp(const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;

extern int	securefile(const char *,uid_t,gid_t) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* forward references */

int		progconfigread(PROGINFO *) ;

static int	progconfigreader(PROGINFO *,vecobj *,char *,int) ;

static int	proclistenadd(PROGINFO *,vecobj *,const char *,int) ;
static int	proclistenmerge(PROGINFO *,vecobj *) ;
static int	proclistenpresent(PROGINFO *,vecobj *,LISTENSPEC *) ;
static int	proclistentmpdel(PROGINFO *,vecobj *,int) ;
static int	proclistenfins(PROGINFO *,vecobj *) ;


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


int progconfigstart(pip,sched,cfname)
PROGINFO	*pip ;
const char	*sched[] ;
const char	cfname[] ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_secreq ;
	const char	**schedp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (cfname == NULL)
	    return SR_FAULT ;

/* look for configuration file */

	f_secreq = (! pip->f.proglocal) ;
	schedp = (sched != NULL) ? sched : schedpconf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	i ;
	    debugprintf("progconfigstart: search-schedule:\n") ;
	    for (i = 0 ; schedp[i] != NULL ; i += 1)
	        debugprintf("progconfigstart: sched%u=%s\n",i,schedp[i]) ;
	}
#endif /* CF_DEBUG */

	rs1 = SR_NOENT ;
	tmpfname[0] = '\0' ;
	if (strchr(cfname,'/') == NULL) {

	    f_secreq = FALSE ;
	    rs1 = permsched(schedp,&pip->svars,
	        tmpfname,MAXPATHLEN, cfname,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progconfigstart: permsched() rs=%d \n",rs1) ;
	        debugprintf("progconfigstart: tmpfname=%s\n", tmpfname) ;
	    }
#endif

	    if (rs1 == 0)
	        rs = mkpath1(tmpfname,cfname) ;

	} else {
	    rs1 = SR_OK ;
	    if (cfname[0] == '/') {
	        rs = mkpath1(tmpfname,cfname) ;
	    } else {
	        rs = mkpath2(tmpfname,pip->pr,cfname) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigstart: mid rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigstart: cfname=%s\n",tmpfname) ;
#endif

	if ((rs1 >= 0) && (tmpfname[0] != '\0')) {
	    const char	**vpp = &pip->cfname ;
	    if ((rs = proginfo_setentry(pip,vpp,tmpfname,-1)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progconfigstart: paramfile_open() rs=%d\n",
	                rs) ;
	            debugprintf("progconfigstart: f=%s\n",tmpfname) ;
	        }
#endif

	        rs1 = paramfile_open(&pip->params,pip->envv, tmpfname) ;
	        pip->open.params = (rs1 >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progconfigstart: paramfile_open() rs=%d\n",
	                rs1) ;
#endif

	    } /* end if */

	} /* end if */

	if (rs >= 0) {
	    pip->f.secure_conf = pip->f.secure_root ;
	    if (f_secreq || (! pip->f.secure_conf)) {
	        rs1 = securefile(pip->cfname,pip->euid,pip->egid) ;
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
	    debugprintf("progconfigstart: ret rs=%d f_pc=%u\n",rs,pip->f.pc) ;
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
/* end subroutine (progconfigstart) */


int progconfigfinish(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigfinish: ent f_pc=%u\n",pip->f.pc) ;
#endif

	if (pip->f.pc) {
	    LISTENSPEC	*lsp ;
	    vecobj	*llp = &pip->listens ;
	    int		i ;
	    for (i = 0 ; vecobj_get(llp,i,&lsp) >= 0 ; i += 1) {
		if (lsp != NULL) {
		    rs1 = listenspec_finish(lsp) ;
		    if (rs >= 0) rs = rs1 ;
		}
	    }
	    if (pip->open.params) {
	        pip->open.params = FALSE ;
	        rs1 = paramfile_close(&pip->params) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (active) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigfinish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progconfigfinish) */


int progconfigcheck(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (pip->f.pc && pip->open.params) {
	    time_t	dt = pip->daytime ;
	    if ((rs = paramfile_check(&pip->params,dt)) > 0) {
	        f = TRUE ;
	        rs = progconfigread(pip) ;
	    }
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progconfigcheck) */


int progconfigread(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.pc && pip->open.params) {
	    vecobj	tmplistens ;
	    int		size = sizeof(LISTENSPEC) ;
	    pip->changed.pc = TRUE ;
	    if ((rs = vecobj_start(&tmplistens,size,6,0)) >= 0) {
	        const int	plen = PBUFLEN ;
	        char		*pbuf ;
	        size = (plen+1) ;
	        if ((rs = uc_malloc(size,&pbuf)) >= 0) {
		    rs = progconfigreader(pip,&tmplistens,pbuf,plen) ;
		    uc_free(pbuf) ;
	        } /* end if (memory-allocation) */
	        if (rs >= 0) rs = proclistenmerge(pip,&tmplistens) ;
		rs1 = proclistenfins(pip,&tmplistens) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_finish(&tmplistens) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecobj) */
	} /* end if (activated) */

	return rs ;
}
/* end subroutine (progconfigread) */


static int progconfigreader(PROGINFO *pip,vecobj *tlp,char *pbuf,int plen)
{
	PARAMFILE	*pfp = &pip->params ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	EXPCOOK		*ckp ;
	const int	elen = EBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		pi ;
	int		kl, vl ;
	int		el, tl ;
	int		v ;
	int		f ;
	const char	*pr = pip->pr ;
	const char	*kp, *vp, *ep ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		ebuf[EBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigread: f_pc=%u open_params=%u\n",
	        pip->f.pc,pip->open.params) ;
#endif

	ckp = &pip->cooks ;
	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {

	    while (rs >= 0) {
		kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	            if (kl == SR_NOTFOUND) break ;
	            rs = kl ;
	            if (rs < 0) break ;

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
	                el = expcook_exp(ckp,0,ebuf,elen,vp,vl) ;
	                if (el >= 0) ebuf[el] = '\0' ;
	            } /* end if */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("progconfigread: ebuf=>%t<\n",ebuf,el) ;
	                debugprintf("progconfigread: switch=%s(%u)\n",
	                    params[pi],pi) ;
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
	                if (el > 0) {
	                    rs1 = cfdecti(ebuf,el,&v) ;
		        }
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
	                if (! pip->final.reqfname) {
	                    char	dname[MAXPATHLEN + 1] ;
	                    pip->have.reqfname = TRUE ;
	                    mkpath2(dname,VARDNAME,pip->searchname) ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        dname,pip->nodename,REQFEXT) ;
	                    f = (pip->reqfname == NULL) ;
	                    f = f || (strcmp(pip->reqfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->reqfname ;
	                        pip->changed.reqfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_pidfile:
	                if (! pip->final.pidfname) {
	                    pip->have.pidfname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        RUNDNAME,pip->nodename,pip->searchname) ;
	                    f = (pip->pidfname == NULL) ;
	                    f = f || (strcmp(pip->pidfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->pidfname ;
	                        pip->changed.pidfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_logfile:
	                if (! pip->final.lfname) {
	                    pip->have.lfname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        LOGDNAME,pip->searchname,"") ;
	                    f = (pip->lfname == NULL) ;
	                    f = f || (strcmp(pip->lfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->lfname ;
	                        pip->fromconf.lfname = TRUE ;
	                        pip->changed.lfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_svcfile:
	                if (! pip->final.svcfname) {
	                    pip->have.svcfname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        ETCDNAME,pip->searchname,SVCFEXT) ;
	                    f = (pip->svcfname == NULL) ;
	                    f = f || (strcmp(pip->svcfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->svcfname ;
	                        pip->fromconf.svcfname = TRUE ;
	                        pip->changed.svcfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_accfile:
	                if (! pip->final.accfname) {
	                    pip->have.accfname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        ETCDNAME,pip->searchname,"") ;
	                    f = (pip->accfname == NULL) ;
	                    f = f || (strcmp(pip->accfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->accfname ;
	                        pip->fromconf.accfname = TRUE ;
	                        pip->changed.accfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_passfile:
	                if (! pip->final.passfname) {
	                    pip->have.passfname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        ETCDNAME,pip->searchname,"") ;
	                    f = (pip->passfname == NULL) ;
	                    f = f || (strcmp(pip->passfname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->passfname ;
	                        pip->fromconf.passfname = TRUE ;
	                        pip->changed.passfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
	                    }
	                }
	                break ;
	            case param_usersrv:
	                if (! pip->final.usersrv) {
	                    pip->have.usersrv = TRUE ;
	                    f = (pip->usersrv == NULL) ;
	                    f = f || (strwcmp(pip->usersrv,ep,el) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->usersrv ;
	                        pip->fromconf.usersrv = TRUE ;
	                        pip->changed.usersrv = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,ep,el) ;
	                    }
	                }
	                break ;
	            case param_stampdir:
	                if (! pip->final.stampdname) {
	                    pip->have.stampdname = TRUE ;
	                    tl = prsetfname(pr,tmpfname,ebuf,el,TRUE,
	                        VARDNAME,STAMPDNAME,"") ;
	                    f = (pip->stampdname == NULL) ;
	                    f = f || (strcmp(pip->stampdname,tmpfname) != 0) ;
	                    if (f) {
				cchar	**vpp = &pip->stampdname ;
	                        pip->changed.stampdname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tmpfname,tl) ;
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
#if	CF_LISTEN
	                if (el > 0) {
	                    rs = proclistenadd(pip,tlp,ebuf,el) ;
		        }
#endif
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
	                    strwcpy(pip->orgcode,ebuf,MIN(ORGCODELEN,el)) ;
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

	            if (rs < 0) break ;
	        } /* end while (enumerating) */

	    paramfile_curend(pfp,&cur) ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutine (progconfigreader) */


/* local subroutines */


/* add a listener-specification to a temporary listen list */
static int proclistenadd(PROGINFO *pip,vecobj *tlp,cchar *ebuf,int elen)
{
	LISTENSPEC	ls ;
	VECSTR		al, *alp = &al ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		n ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenadd: ebuf=>%t<\n",
	        ebuf,elen) ;
#endif

	if ((rs = vecstr_start(alp,5,0)) >= 0) {
	    const char	*sp = ebuf ;
	    const char	*tp ;
	    int		sl = elen ;
	    int		c = 0 ;

	    while ((tp = strnchr(sp,sl,CH_FS)) != NULL) {
	        c += 1 ;
	        rs = vecstr_add(alp,sp,(tp-sp)) ;
	        if (rs < 0) break ;
	        sl -= (tp+1)-sp ;
	        sp = (tp+1) ;
	    } /* end while */
	    if ((rs >= 0) && (sl > 0)) {
	        c += 1 ;
	        rs = vecstr_add(alp,sp,sl) ;
	    }

	    if (rs >= 0) {
	        const char	**av ;
	        if ((rs = vecstr_getvec(alp,&av)) >= 0) {
	            if ((rs = listenspec_start(&ls,c,av)) >= 0) {
	                n = rs ;

	                if (n > 0) {
	                    rs = vecobj_add(tlp,&ls) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progconfig/proclistenadd: "
				    "vecobj_add() rs=%d\n",
	                            rs) ;
#endif

	                }

	                if ((n == 0) || (rs < 0)) {
	                    listenspec_finish(&ls) ;
			}

	            } /* end if */
	        } /* end if */
	    } /* end if */

	    rs1 = vecstr_finish(alp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (proclistenadd) */


static int proclistenmerge(PROGINFO *pip,vecobj *tlp)
{
	LISTENSPEC	*lsp ;
	LISTENSPEC	*tlsp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfig/proclistenmerge: ent\n") ;
#endif

/* phase-1 */

	for (i = 0 ; vecobj_get(&pip->listens,i,&lsp) >= 0 ; i += 1) {
	    if (lsp != NULL) {
	        if ((rs1 = proclistenpresent(pip,tlp,lsp)) >= 0) {
	            proclistentmpdel(pip,tlp,rs1) ;
	        } else {
	            listenspec_delset(lsp,TRUE) ;
	        }
	    }
	} /* end for */

/* phase-2 */

	if (rs >= 0) {
	for (i = 0 ; vecobj_get(tlp,i,&tlsp) >= 0 ; i += 1) {
	    if (tlsp != NULL) {

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

	    }
	    if (rs < 0) break ;
	} /* end for */
	} /* end if */

	return rs ;
}
/* end subroutine (proclistenmerge) */


static int proclistenpresent(PROGINFO *pip,vecobj *tlp,LISTENSPEC *lsp)
{
	LISTENSPEC	*tlsp ;
	int		rs ;
	int		rs1 ;
	int		i = 0 ;

	for (i = 0 ; (rs = vecobj_get(tlp,i,&tlsp)) >= 0 ; i += 1) {
	    if (tlsp != NULL) {
	       rs1 = listenspec_issame(lsp,tlsp) ;
	       if (rs1 > 0) break ;
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (proclistenpresent) */


static int proclistentmpdel(PROGINFO *pip,vecobj *tlp,int ei)
{
	LISTENSPEC	*tlsp = NULL ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_get(tlp,ei,&tlsp)) >= 0) {
	    if (tlsp != NULL) {
	        rs1 = listenspec_finish(tlsp) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(tlp,ei) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (proclistentmpdel) */


static int proclistenfins(PROGINFO *pip,vecobj *tlp)
{
	LISTENSPEC	*lsp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(tlp,i,&lsp) >= 0 ; i += 1) {
	    if (lsp != NULL) {
	        rs1 = listenspec_finish(lsp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (proclistenfins) */


