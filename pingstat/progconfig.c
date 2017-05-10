/* progconfig */

/* program configuration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2008-10-10, David A­D­ Morano

	This was adapted from the FINGERS program.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines that manage program
	configuration.


*******************************************************************************/


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
#include	<localmisc.h>

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

#define	SUMFEXT		"sum"


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
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;

extern int	securefile(const char *,uid_t,gid_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		progconfigread(struct proginfo *) ;

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

static const char	*configopts[] = {
	"pidfile",
	"logfile",
	"sumfile",
	"logsize",
	"markint",
	"minpingint",
	"minupdateint",
	"pingto",
	NULL
} ;

enum configopts {
	configopt_pidfile,
	configopt_logfile,
	configopt_sumfile,
	configopt_logsize,
	configopt_markint,
	configopt_minpingint,
	configopt_minupdateint,
	configopt_pingto,
	configopt_overlast
} ;


/* exported subroutines */


int progconfigbegin(pip,sched,configfname)
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
	    debugprintf("progconfigbegin: search-schedule:\n") ;
	    for (i = 0 ; schedp[i] != NULL ; i += 1)
	        debugprintf("progconfigbegin: sched%u=%s\n",i,schedp[i]) ;
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
	        debugprintf("progconfigbegin: permsched() rs=%d \n",rs1) ;
	        debugprintf("progconfigbegin: tmpfname=%s\n", tmpfname) ;
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
	    debugprintf("progconfigbegin: mid rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progconfigbegin: configfname=%s\n",tmpfname) ;
#endif

	if ((rs1 >= 0) && (tmpfname[0] != '\0')) {

	    rs = proginfo_setentry(pip,&pip->configfname,tmpfname,-1) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progconfigbegin: paramfile_open() rs=%d\n",
			rs) ;
	            debugprintf("progconfigbegin: f=%s\n",tmpfname) ;
	        }
#endif

	        rs1 = paramfile_open(&pip->params,pip->envv, tmpfname) ;
	        pip->open.params = (rs1 >= 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progconfigbegin: paramfile_open() rs=%d\n",
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
	    debugprintf("progconfigbegin: ret rs=%d f_pc=%u\n",rs,pip->f.pc) ;
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
/* end subroutine (progconfigbegin) */


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


int progconfigend(pip)
struct proginfo	*pip ;
{
	int	rs = SR_NOTOPEN ;

	if (pip->f.pc) {
	    rs = SR_OK ;
	    if (pip->open.params) {
	        pip->open.params = FALSE ;
	        rs = paramfile_close(&pip->params) ;
	    }
	}

	return rs ;
}
/* end subroutine (progconfigend) */


int progconfigread(pip)
struct proginfo	*pip ;
{
	PARAMFILE_CUR	cur ;

	PARAMFILE_ENT		pe ;

	int	rs = SR_OK ;
	int	rs1 = 0 ;
	int	oi ;
	int	kl ;
	int	vl ;
	int	elen ;
	int	tl ;
	int	v ;
	int	f ;

	const char	*kp, *vp ;

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

	rs1 = 0 ;
	if ((rs = paramfile_curbegin(&pip->params,&cur)) >= 0) {

	while (rs >= 0) {

	    kl = paramfile_enum(&pip->params,&cur,&pe,pbuf,PBUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progconfigread: paramfile_enum() rs=%d\n",
	            kl) ;
#endif

	    if (kl <= 0)
	        break ;

	    kp = pe.key ;
	    vp = pe.value ;
	    vl = pe.vlen ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progconfigread: enum k=%t\n",kp,kl) ;
#endif

	    oi = matpstr(configopts,2,kp,kl) ;

	    if (oi < 0) continue ;

	    ebuf[0] = '\0' ;
	    elen = 0 ;
	    if (vl > 0) {

	        elen = expcook_exp(&pip->cooks,0,ebuf,EBUFLEN,vp,vl) ;

	        if (elen >= 0)
	            ebuf[elen] = '\0' ;

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("progconfigread: ebuf=>%t<\n",ebuf,elen) ;
	        debugprintf("progconfigread: switch=%s(%u)\n",
	            configopts[i],i) ;
	    }
#endif

	    if (elen < 0)
	        continue ;

	    switch (oi) {

	    case configopt_logsize:
	        if ((elen > 0) && (! pip->final.logsize)) {

	            rs1 = cfdecmfi(ebuf,elen,&v) ;
	            if ((rs1 >= 0) && (v >= 0)) {
	                pip->have.logsize = TRUE ;
	                pip->changed.logsize = TRUE ;
	                pip->logsize = v ;
	            }

	        }
	        break ;

	    case configopt_markint:
	case configopt_minpingint:
	case configopt_minupdateint:
	case configopt_pingto:
	        v = -1 ;
	        if (elen > 0)
	            rs1 = cfdecti(ebuf,elen,&v) ;

	        if ((rs1 >= 0) && (v >= 0)) {

	            switch (oi) {

	            case configopt_markint:
	                if (! pip->final.intmark) {
	                    pip->have.intmark = TRUE ;
	                    pip->changed.intmark = TRUE ;
	                    pip->intmark = v ;
	                }

	                break ;

	            case configopt_minpingint:
	                if (! pip->final.intminping) {
	                    pip->have.intminping = TRUE ;
	                    pip->changed.intminping = TRUE ;
	                    pip->intminping = v ;
	                }

	                break ;

	            case configopt_minupdateint:
	                if (! pip->final.intminupdate) {
	                    pip->have.intminupdate = TRUE ;
	                    pip->changed.intminupdate = TRUE ;
	                    pip->intminupdate = v ;
	                }

	                break ;

	            case configopt_pingto:
	                if (! pip->final.toping) {
	                    pip->have.toping = TRUE ;
	                    pip->changed.toping = TRUE ;
	                    pip->toping = v ;
	                }

	                break ;

	            } /* end switch */

	        } /* end if (valid number) */

	        break ;

	    case configopt_sumfile:
	        if (! pip->final.sumfile) {

	            char	dname[MAXPATHLEN + 1] ;

	            pip->have.sumfile = TRUE ;
	            mkpath2(dname,VARDNAME,pip->searchname) ;
	            tl = setfname(pip,tmpfname,ebuf,elen,TRUE,
	                dname,pip->nodename,SUMFEXT) ;

	            f = (pip->sumfname == NULL) ;
	            f = f || (strcmp(pip->sumfname,tmpfname) != 0) ;
	            if (f) {
	                pip->changed.sumfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->sumfname,
	                    tmpfname,tl) ;
	            }

	        }

	        break ;

	    case configopt_pidfile:
	        if (! pip->final.pidfile) {

	            pip->have.pidfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,elen,TRUE,
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

	    case configopt_logfile:
	        if (! pip->final.logfile) {

	            pip->have.logfile = TRUE ;
	            tl = setfname(pip,tmpfname,ebuf,elen,TRUE,
	                LOGDNAME,pip->searchname,"") ;

	            f = (pip->logfname == NULL) ;
	            f = f || (strcmp(pip->logfname,tmpfname) != 0) ;
	            if (f) {
	                pip->changed.logfile = TRUE ;
	                rs = proginfo_setentry(pip,&pip->logfname,
	                    tmpfname,tl) ;
	            }

	        }

	        break ;

	    } /* end switch */

	    if (rs < 0)
	        break ;

	} /* end while (enumerating) */

	paramfile_curend(&pip->params,&cur) ;
	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (progconfigread) */


/* local subroutines */


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
	        if ((dname != NULL) && (dname[0] != '\0')) {
	            rs = mkpath3(fname,pip->pr,dname,np) ;
	        } else
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
	            if ((dname != NULL) && (dname[0] != '\0')) {
	                rs = mkpath3(fname,pip->pr,dname,np) ;
	            } else
	                rs = mkpath2(fname,pip->pr,np) ;
	        }

	    } else
	        rs = mkpath1(fname,np) ;

	} /* end if */

	return rs ;
}
/* end subroutine (setfname) */



