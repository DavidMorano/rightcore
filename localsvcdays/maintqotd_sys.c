/* maintqotd_sys */

/* SYSTEM dialer for MAINTQOTD */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_BACKGROUND	1		/* put program in background */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is called for the "system" dialer for MAINTQOTD.

	Synopsis:

	int maintqotd_sys(MAINTQOTD *sip,const char *qfname,const char *sep)

	Arguments:

	sip		pointer to local state
	qfname		QOTD-file name
	sep		source entry pointer

	Returns:

	<0		some error
	>=0		resulting FD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ascii.h>
#include	<spawner.h>
#include	<filebuf.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"maintqotd.h"
#include	"systems.h"
#include	"sysdialer.h"
#include	"cm.h"


/* local defines */

#define	CHECKER		struct checker

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif


/* external subroutines */

extern int	snwcpyclean(char *,int,int,const char *,int) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optvalue(const char *,int) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
static int	debugoutput(const char *,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;
extern const char	*strsigabbr(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern const char	**environ ;


/* local structures */

struct checker {
	VECSTR		stores ;
	MAINTQOTD	*sip ;
	const char	**envv ;	/* remains NULL */
	const char	*pr ;
	const char	*sep ;
	const char	*progfname ;
	const char	**argv ;
	const char	*a ;		/* allocation */
	int		intcheck ;	/* interval-check */
	int		an ;
} ;


/* forward references */

static int checker_start(CHECKER *,MAINTQOTD *,const char *) ;
static int checker_finish(CHECKER *) ;
static int checker_setentry(CHECKER *,const char **,const char *,int) ;
static int checker_argbegin(CHECKER *,const char *) ;
static int checker_argend(CHECKER *) ;
static int checker_findprog(CHECKER *,char *,const char *,int) ;
static int checker_progrun(CHECKER *,const char *) ;
static int checker_proglog(CHECKER *,int) ;

#ifdef	COMMENT
static int mksfname(char *,const char *,const char *,const char *) ;
#endif /* COMMENT */


/* local variables */


/* exported subroutines */


int maintqotd_sys(sip,qfname,sep)
MAINTQOTD	*sip ;
const char	qfname[] ;
const char	*sep ;
{
	CHECKER		c ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("maintqotd_sys: ent\n") ;
#endif

	if (qfname == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;

	if ((rs = checker_start(&c,sip,sep)) >= 0) {
	        if ((rs = checker_argbegin(&c,sep)) >= 0) {

	            rs = checker_progrun(&c,qfname) ;
	            fd = rs ;

#if	CF_DEBUGS
	            debugprintf("maintqotd_sys: _progrun() rs=%d\n",rs) ;
#endif
	            rs1 = checker_argend(&c) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (arg) */
	    rs1 = checker_finish(&c) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) {
	        u_close(fd) ;
	        fd = -1 ;
	    }
	} /* end if (checker) */

#if	CF_DEBUGS
	debugprintf("maintqotd_sys: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (maintqotd_sys) */


/* local subroutines */


static int procsystems(PROGINFO *pip,void *ofp,const char *sfname)
{
	struct locinfo	*lip = pip->lip ;
	SYSDIALER	d ;
	int		rs ;

	if ((rs = sysdialer_start(&d,pip->pr,NULL,NULL)) >= 0) {
	    CM_ARGS		ca ;
	    SYSTEMS		sysdb ;
	    int		al ;
	    const char	*ap ;

	    memset(&ca,0,sizeof(CM_ARGS)) ;
	    ca.pr = pip->pr ;
	    ca.prn = pip->rootname ;
	    ca.searchname = pip->searchname ;
	    ca.nodename = pip->nodename ;
	    ca.domainname = pip->domainname ;
	    ca.username = pip->username ;
	    ca.sp = &sysdb ;
	    ca.dp = &d ;
	    ca.timeout = lip->to ;
	    ca.options = (SYSDIALER_MFULL | SYSDIALER_MCO) ;

/* do it */

	    if ((rs = systems_open(&sysdb,sfname)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("b_rfinger: systems_open() rs=%d\n",rs) ;
#endif

	        if (sfname == NULL) {
	            rs = loadsysfiles(pip,&sysdb) ;

#if	CF_DEBUGS
	                debugprintf("b_rfinger: loadsysfiles() rs=%d\n",
	                    rs) ;
#endif

	        } /* end if (loadfiles) */

#if	CF_DEBUGS && 0
	        {
	            SYSTEMS_CUR		cur ;
	            SYSTEMS_ENT	*sep ;
	            debugprintf("b_rfinger: sysnames: \n") ;
	            systems_curbegin(&sysdb,&cur) ;
	            while (systems_enum(&sysdb,&cur,&sep) >= 0) {
	                debugprintf("b_rfinger: sysname=%s\n",sep->sysname) ;
	            }
	            systems_curend(&sysdb,&cur) ;
	        }
#endif /* CF_DEBUGS */

	        if (rs >= 0) {
	            int	i = 0 ;
	            while (rs >= 0) {
	                al = locinfo_argenum(lip,i++,&ap) ;
	                if (al == SR_NOTFOUND) break ;
	                rs = al ;

	                if (rs >= 0) {
	                    if (ap == NULL) continue ;
	                    rs = procsystem(pip,ofp,&ca,ap) ;
	                }

	            } /* end while */
	        } /* end if */

	        systems_close(&sysdb) ;
	    } /* end if (systems) */

	    sysdialer_finish(&d) ;
	} /* end if (sysdialer) */

#if	CF_DEBUGS
	    debugprintf("b_rfinger/procsystems: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsystems) */


static int procsystem(pip,ofp,cap,ap)
PROGINFO	*pip ;
void		*ofp ;
CM_ARGS		*cap ;
const char	*ap ;
{
	struct locinfo	*lip = pip->lip ;
	struct query	q ;
	CM		con ;
	const int	llen = LINEBUFLEN ;
	const int	clen = LINEBUFLEN ;
	int		rs ;
	int		to ;
	int		ll ;
	int		cl ;
	int		ql = -1 ;
	int		ropts = 0 ;
	int		wlen = 0 ;
	int		f_long ;
	const char	**av ;
	const char	*qp ;
	char		lbuf[LINEBUFLEN + 1], *lp = lbuf ;
	char		cbuf[LINEBUFLEN+1] ;

#if	CF_DEBUGS
	{
	    debugprintf("main/procsystem: ap=>%s<\n",ap) ;
	    debugprintf("main/procsystem: svc=%s\n",lip->svcspec) ;
	}
#endif

	f_long = lip->f.longer ;
	av = lip->av ;
	to = lip->to ;

#if	CF_DEBUGS
	{
	    debugprintf("main/procsystem: av=%p\n",av) ;
	    if (av != NULL) {
	        int	i ;
	        for (i = 0 ; av[i] != NULL ; i += 1)
	            debugprintf("main/procsystem: a[%u]=%s\n",i,av[i]) ;
	    }
	}
#endif /* CF_DBEBUGS */

	rs = query_parse(&q,ap) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	{
	    debugprintf("main/procsystem: hpart=>%s<\n",q.hpart) ;
	    debugprintf("main/procsystem: upart=>%s<\n",q.upart) ;
	}
#endif

	rs = mkfingerquery(lbuf,llen,f_long,q.upart,av) ;
	ql = rs ;
	if (rs < 0) goto ret0 ;
	qp = lbuf ;

#if	CF_DEBUGS
	{
	    debugprintf("main/procsystem: mkfingerquery() rs=%d\n",rs) ;
	    debugprintf("main/procsystem: q=>%t<\n",qp,ql) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = cm_open(&con,cap,q.hpart,lip->svcspec,NULL)) >= 0) {
	    const char	*tmpdname = pip->tmpdname ;

#if	CF_DEBUGS
	        debugprintf("main/procsystem: cm_open() rs=%d\n",rs) ;
#endif

/* debug information (if requested) */

	    if (pip->debuglevel > 0) {
	        CM_INFO	ci ;

	        rs = cm_info(&con,&ci) ;

#if	CF_DEBUGS
	            debugprintf("main/procsystem: cm_info() rs=%d\n",rs) ;
#endif

	        if (rs >= 0)
	            shio_printf(pip->efp,"%s: selected dialer=%s\n",
	                pip->progname,ci.dname) ;

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"dialer=%s",
	                ((rs >= 0) ? ci.dname : "*")) ;

	    } /* end if (dialer information) */

/* write the query */

	    if (rs >= 0) {
	        if (ql < 0) ql = strlen(qp) ;
	        rs = cm_write(&con,qp,ql) ;
	    }

#if	CF_SHUTDOWN
	    if ((rs >= 0) && lip->f.shutdown)
	        cm_shutdown(&con,SHUT_WR) ;
#endif /* CF_SHUTDOWN */

/* read the response */

	    if ((rs >= 0) && ((rs = opentmp(tmpdname,O_RDWR,0664)) >= 0)) {
	        int	fd = rs ;

	        while (rs >= 0) {
	            rs = cm_reade(&con,lbuf,llen,to,ropts) ;
	            ll = rs ;
#if	CF_DEBUGS
	                debugprintf("main/procsystem: cm_reade() rs=%d\n",
	                    rs) ;
#endif
	            if (rs <= 0) break ;
	            rs = u_write(fd,lbuf,ll) ;
	        } /* end while */

	        if (rs >= 0) rs = u_rewind(fd) ;

	        if (rs >= 0) {
	            FILEBUF	b ;
	            const int	opts = 0 ;

	            if ((rs = filebuf_start(&b,fd,0L,512,opts)) >= 0) {

	                while (rs >= 0) {
	                    rs = filebuf_readlines(&b,lbuf,llen,to,NULL) ;
	                    ll = rs ;
	                    if (rs <= 0) break ;

	                    if ((ll > 0) && (lp[ll-1] == '\n')) ll -= 1 ;

	                    if (if_exit) rs = SR_EXIT ;
	                    if ((rs >= 0) && if_int) rs = SR_INTR ;

	                    if (rs >= 0) {
	                        rs = snwcpyclean(cbuf,clen,'¿',lp,ll) ;
	                        cl = rs ;
	                        if (lip->open.outer && (cl > 0)) {
	                            rs = locinfo_termoutprint(lip,ofp,cbuf,cl) ;
	                            wlen += rs ;
	                        } else {
	                            rs = shio_printline(ofp,cbuf,cl) ;
	                            wlen += rs ;
	                        }
	                    }

	                } /* end while (reading lines) */

	                filebuf_finish(&b) ;
	            } /* end if (filebuf) */

	        } /* end if */

	        u_close(fd) ;
	    } /* end if (opentmp) */

	    cm_close(&con) ;
	} /* end if (cm) */

ret0:

#if	CF_DEBUGS
	    debugprintf("main/procsystem: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsystem) */


static int loadsysfiles(pip,sdbp)
PROGINFO	*pip ;
SYSTEMS		*sdbp ;
{
	SCHEDVAR	sf ;
	int		rs ;
	int		rs1 ;
	int		i, j ;
	int		n = 0 ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((rs = schedvar_start(&sf)) >= 0) {

	    schedvar_add(&sf,"p",pip->pr,-1) ;

	    schedvar_add(&sf,"n",pip->searchname,-1) ;

	    for (j = 0 ; j < 2 ; j += 1) {

	        if (j == 0) {
	            schedvar_add(&sf,"f",SYSFNAME1,-1) ;
	        } else
	            schedvar_add(&sf,"f",SYSFNAME2,-1) ;

	        for (i = 0 ; sysfiles[i] != NULL ; i += 1) {

	            rs = schedvar_expand(&sf,tmpfname,MAXPATHLEN,
	                sysfiles[i],-1) ;

	            rs1 = SR_NOENT ;
	            if (rs >= 0)
	                rs1 = u_access(tmpfname,R_OK) ;

	            if (rs1 >= 0) {

	                n += 1 ;
	                rs = systems_fileadd(sdbp,tmpfname) ;

#if	CF_DEBUGS
	                {
	                    debugprintf("b_rfinger: "
	                        "systems_fileadd() rs=%d\n",
	                        rs) ;
	                    debugprintf("b_rfinger: fname=%s\n", tmpfname) ;
	                }
#endif /* CF_DEBUGS */

	            } /* end if */

	            if (rs < 0) break ;
	        } /* end for */

	        if (rs < 0) break ;
	    } /* end for */

	    schedvar_finish(&sf) ;
	} /* end if (schedvar) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (loadsysfiles) */


static int checker_start(chp,sip,sep)
CHECKER		*chp ;
MAINTQOTD	*sip ;
const char	*sep ;
{
	int		rs ;

	memset(chp,0,sizeof(CHECKER)) ;
	chp->pr = sip->pr ;
	chp->sip = sip ;
	chp->sep = sep ;
	chp->envv = environ ;

	rs = vecstr_start(&chp->stores,1,0) ;

#if	CF_DEBUGS
	debugprintf("checker_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checker_start) */


static int checker_finish(chp)
CHECKER		*chp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("checker_finish: ent\n") ;
#endif

	rs1 = vecstr_finish(&chp->stores) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("checker_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checker_finish) */


int checker_setentry(chp,epp,vp,vl)
CHECKER		*chp ;
const char	**epp ;
const char	*vp ;
int		vl ;
{
	VECSTR		*slp = &chp->stores ;
	int		rs = SR_OK ;
	int		oi = -1 ;
	int		len = 0 ;

	if (chp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_INVALID ;

	if (*epp != NULL)
	    oi = vecstr_findaddr(slp,*epp) ;

	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(slp,vp,len,epp) ;
	} else if (epp != NULL)
	    *epp = NULL ;

	if ((rs >= 0) && (oi >= 0))
	    vecstr_del(slp,oi) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (checker_setentry) */


static int checker_argbegin(CHECKER *chp,const char *ap)
{
	int		rs ;
	int		avsize ;
	int		size = 0 ;
	int		na = 0 ;
	const char	*sp = ap ;
	const char	*tp ;
	void		*a ;

	while ((tp = strchr(sp,CH_FS)) != NULL) {
	    na += 1 ;
	    size += ((tp-sp)+1) ;
	    sp = (tp+1) ;
	} /* end while */
	if (sp[0] != '\0') {
	    na += 1 ;
	    size += (strlen(sp) + 1) ;
	}

	avsize = ((na+1) * sizeof(const char **)) ;
	size += avsize ;

	chp->an = na ;
	if ((rs = uc_malloc(size,&a)) >= 0) {
	    int		c = 1 ;
	    const char	**argv = (const char **) a ;
	    char	*bp = a ;
	    chp->a = a ;
	    chp->argv = argv ;
	    sp = ap ;
	    bp += avsize ;
	    while ((tp = strchr(sp,CH_FS)) != NULL) {
	        argv[c++] = sp ;
	        bp = (strwcpy(bp,sp,(tp-sp)) + 1) ;
	    }
	    if (sp[0] != '\0') {
	        argv[c++] = sp ;
	        bp = (strwcpy(bp,sp,-1) + 1) ;
	    }
	    argv[c] = NULL ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("checker_argbegin: ret rs=%d n=%u\n",rs,na) ;
#endif

	return (rs >= 0) ? na : rs ;
}
/* end subroutine (checker_argbegin) */


static int checker_argend(CHECKER *chp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (chp->a != NULL) {
	    rs1 = uc_free(chp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    chp->a = NULL ;
	    chp->argv= NULL ;
	}

	return rs ;
}
/* end subroutine (checker_argend) */


static int checker_findprog(CHECKER *chp,char *rbuf,const char *pp,int pl)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("checker_findprog: ent pn=%t\n",pp,pl) ;
#endif

	rbuf[0] = '\0' ;
	if (pp[0] != '/') {
	    if ((pp[0] == '\0') || (pp[0] == '-') || (pp[0] == '+')) {
	        pp = MAINTQOTD_PROGMKQOTD ;
	        pl = -1 ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("checker_findprog: pn=%t\n",pp,pl) ;
#endif

	rs = prgetprogpath(chp->pr,rbuf,pp,pl) ;

#if	CF_DEBUGS
	debugprintf("checker_findprog: prgetprogpath() rs=%d rbuf=%s\n",
	    rs,rbuf) ;
#endif

	if (rs >= 0) {
	    const char	**vpp = &chp->progfname ;
	    rl = (rs > 0) ? rs : strlen(rbuf) ;
	    rs = checker_setentry(chp,vpp,rbuf,rl) ;
	} /* end if */

	if (rs >= 0) {
	    MAINTQOTD	*sip = chp->sip ;
	    if (sip->open.logsub) {
		LOGFILE	*lhp = sip->logsub ;
		logfile_printf(lhp,"svc=prog") ;
		logfile_printf(lhp,"pf=%s",rbuf) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("checker_findprog: ret rbuf=%s\n",rbuf) ;
	debugprintf("checker_findprog: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (checker_findprog) */


static int checker_progrun(CHECKER *chp,const char *qfname)
{
	SPAWNER		s ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("checker_progrun: ent\n") ;
#endif

	if (chp == NULL) return SR_FAULT ;

	if (chp->progfname != NULL) {
	    int		zl ;
	    const char	*pf = chp->progfname ;
	    const char	**ev = chp->envv ;
	    const char	**av = chp->argv ;
	    const char	*zp ;
	    if ((zl = sfbasename(pf,-1,&zp)) > 0) {
		const mode_t	om = 0664 ;
		const int	of = (O_RDWR|O_CREAT|O_TRUNC) ;
	        const char	*ap = av[0] ;
	        char		argz[MAXNAMELEN+1] ;
	        if (ap != NULL) {
	            if ((ap[0] == '+') || (ap[0] == '-') || (ap[0] == '\0')) {
	                char	*bp = argz ;
	                if (ap[0] == '-') *bp++ = '-' ;
	                bp = strnwcpy(bp,(MAXNAMELEN-1),zp,zl) ;
	                av[0] = argz ;
	            }
	        }
	        if ((rs = u_open(qfname,of,om)) >= 0) {
	            fd = rs ;
	            if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
	                const char	*varpr = MAINTQOTD_PRNAME ;
	                if (getourenv(ev,varpr) == NULL) {
	                    rs = spawner_envset(&s,varpr,chp->pr,-1) ;
	                }
	                if (rs >= 0) {
	                    int	i ;
#if	CF_BACKGROUND
	                    spawner_sigignores(&s) ;
	                    spawner_setsid(&s) ;
#endif
	                    for (i = 0 ; i < 3 ; i += 1)
	                        spawner_fdclose(&s,i) ;
	                    spawner_fdnull(&s,O_RDONLY) ;
	                    spawner_fddup(&s,fd) ;
	                    spawner_fdnull(&s,O_WRONLY) ;
	                    if ((rs = spawner_run(&s)) >= 0) {
				int	cs = 0 ;

	                	rs1 = spawner_wait(&s,&cs,0) ;
	                	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	                        debugprintf("checker_progrun: w rs=%d cs=%d\n",
				    rs,cs) ;
#endif

				if (rs >= 0) {
				    rs = checker_proglog(chp,cs) ;
				}

	                        if (rs >= 0) u_rewind(fd) ;
			    } /* end if (spawner_run) */

#if	CF_DEBUGS
	                    debugprintf("checker_progrun: "
				"spawner_run() rs=%d\n", rs) ;
	                    debugoutput("checker_progrun: ¬",fd) ;
#endif

	                } /* end if */
	                spawner_finish(&s) ;
	            } /* end if (spawner) */
	            if ((rs < 0) && (fd >= 0)) {
	                u_close(fd) ;
	                fd = -1 ;
	            }
	        } /* end if (piper) */
	    } else
	        rs = SR_NOENT ;
	} else
	    rs = SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("checker_progrun: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (checker_progrun) */


static int checker_proglog(CHECKER *chp,int cs)
{
	MAINTQOTD	*sip = chp->sip ;
	int		rs = SR_OK ;

	if (sip->open.logsub) {
	   LOGFILE	*lhp = sip->logsub ;
	    if (WIFEXITED(cs)) {
		int	ex = WEXITSTATUS(cs) ;
		logfile_printf(lhp,"program exited normally ex=%u",ex) ;
	    } else if (WIFSIGNALED(cs)) {
		int	sig = WTERMSIG(cs) ;
		const char	*ss ;
		char		sigbuf[20+1] ;
		if ((ss = strsigabbr(sig)) == NULL) {
		     ctdeci(sigbuf,20,sig) ;
		     ss = sigbuf ;
		}
		logfile_printf(lhp,"program exited w/ sig=%s",ss) ;
	    } else {
		logfile_printf(lhp,"program exited weirdly cs=\\x%08x",cs) ;
	    }
	} /* end if (logging) */

	return rs ;
}
/* end subroutine (checker_proglog) */


#ifdef	COMMENT
static int mksfname(rbuf,pr,sdname,sname)
char		rbuf[] ;
const char	*pr ;
const char	*sdname ;
const char	*sname ;
{
	int		rs ;

	if (sdname[0] != '/') {
	    rs = mkpath3(rbuf,pr,sdname,sname) ;
	} else
	    rs = mkpath2(rbuf,sdname,sname) ;

	return rs ;
}
/* end subroutine (mksfname) */
#endif /* COMMENT */

#if	CF_DEBUGS
static int debugoutput(const char *ids,int fd)
{
	FILEBUF		b ;
	int		rs ;
	int		wlen = 0 ;
	debugprintf("%t\n",ids,strlinelen(ids,80,60)) ;
	sleep(2) ;
	if ((rs = uc_fsize(fd)) >= 0) {
	    debugprintf("%t fsize=%u\n",ids,strlinelen(ids,80,60),rs) ;
	    if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN+1] ;
	        while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	            debugprintf("o> %t\n",
	                lbuf,strlinelen(lbuf,rs,70)) ;
	        } /* end while */
	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */
	} /* end if (fsize) */
	if (rs >= 0) u_rewind(fd) ;
	return (rs >= 0) ? wlen : rs ;
}
#endif /* CF_DEBUGS */


