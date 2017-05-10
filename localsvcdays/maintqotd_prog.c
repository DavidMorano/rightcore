/* maintqotd_prog */

/* PROGRAM dialer for MAINTQOTD */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_BACKGROUND	1		/* put program in background */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is called for the "prog" dialer for MAINTQOTD.

	Synopsis:

	int maintqotd_prog(MAINTQOTD *sip,cchar *qfname,cchar *sep)

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
#include	<time.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ascii.h>
#include	<spawner.h>
#include	<filebuf.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"maintqotd.h"


/* local defines */

#define	CHECKER		struct checker

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif


/* external subroutines */

extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	mkpath3w(char *,cchar *,cchar *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	matkeystr(cchar **,char *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optvalue(cchar *,int) ;
extern int	prgetprogpath(cchar *,char *,cchar *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
static int	debugoutput(cchar *,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;
extern cchar	*strsigabbr(int) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnwcpy(char *,int,cchar *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct checker {
	VECSTR		stores ;
	MAINTQOTD	*sip ;
	cchar		**envv ;	/* remains NULL */
	cchar		*pr ;
	cchar		*sep ;
	cchar		*progfname ;
	cchar		**argv ;
	cchar		*a ;		/* allocation */
	int		intcheck ;	/* interval-check */
	int		an ;
} ;


/* forward references */

static int checker_start(CHECKER *,MAINTQOTD *,cchar *) ;
static int checker_finish(CHECKER *) ;
static int checker_setentry(CHECKER *,cchar **,cchar *,int) ;
static int checker_argbegin(CHECKER *,cchar *) ;
static int checker_argend(CHECKER *) ;
static int checker_findprog(CHECKER *,char *,cchar *,int) ;
static int checker_progrun(CHECKER *,cchar *) ;
static int checker_proglog(CHECKER *,int,pid_t,int) ;

#ifdef	COMMENT
static int mksfname(char *,cchar *,cchar *,cchar *) ;
#endif /* COMMENT */


/* local variables */


/* exported subroutines */


int maintqotd_prog(MAINTQOTD *sip,cchar qfname[],cchar *sep)
{
	CHECKER		c ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("maintqotd_prog: ent\n") ;
#endif

	if (qfname == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;

	if ((rs = checker_start(&c,sip,sep)) >= 0) {
	    int		pl = -1 ;
	    cchar	*pp = sep ;
	    cchar	*ap = sep ;
	    cchar	*tp ;
	    char	rbuf[MAXPATHLEN+1] ;
	    if ((tp = strchr(pp,CH_FS)) != NULL) {
	        pl = (tp-pp) ;
	        ap = (tp+1) ;
	    }
	    if ((rs = checker_findprog(&c,rbuf,pp,pl)) > 0) {
	        if ((rs = checker_argbegin(&c,ap)) >= 0) {

	            rs = checker_progrun(&c,qfname) ;
	            fd = rs ;

#if	CF_DEBUGS
	            debugprintf("maintqotd_prog: _progrun() rs=%d\n",rs) ;
#endif
	            rs1 = checker_argend(&c) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (arg) */
	    } /* end if (findprog) */
#if	CF_DEBUGS
	    debugprintf("maintqotd_prog: findprog-out rs=%d\n",rs) ;
#endif

	    rs1 = checker_finish(&c) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) {
		uc_unlink(qfname) ;
	        u_close(fd) ;
	        fd = -1 ;
	    }
	} /* end if (checker) */

#if	CF_DEBUGS
	debugprintf("maintqotd_prog: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (maintqotd_prog) */


/* local subroutines */


static int checker_start(CHECKER *chp,MAINTQOTD *sip,cchar *sep)
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


static int checker_finish(CHECKER *chp)
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


int checker_setentry(CHECKER *chp,cchar **epp,cchar *vp,int vl)
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


static int checker_argbegin(CHECKER *chp,cchar *ap)
{
	int		rs ;
	int		avsize ;
	int		size = 0 ;
	int		na = 0 ;
	cchar		*sp = ap ;
	cchar		*tp ;
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

	avsize = ((na+1) * sizeof(cchar **)) ;
	size += avsize ;

	chp->an = na ;
	if ((rs = uc_malloc(size,&a)) >= 0) {
	    int		c = 1 ;
	    cchar	**argv = (cchar **) a ;
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


static int checker_findprog(CHECKER *chp,char *rbuf,cchar *pp,int pl)
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

	if ((rs = prgetprogpath(chp->pr,rbuf,pp,pl)) >= 0) {
	    cchar	**vpp = &chp->progfname ;
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


static int checker_progrun(CHECKER *chp,cchar *qfname)
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
	    cchar	*pf = chp->progfname ;
	    cchar	**ev = chp->envv ;
	    cchar	**av = chp->argv ;
	    cchar	*zp ;
	    if ((zl = sfbasename(pf,-1,&zp)) > 0) {
		const mode_t	om = 0664 ;
		const int	of = (O_RDWR|O_CREAT|O_TRUNC) ;
	        cchar		*ap = av[0] ;
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
	                cchar	*varpr = MAINTQOTD_PRNAME ;
	                if (getourenv(ev,varpr) == NULL) {
	                    rs = spawner_envset(&s,varpr,chp->pr,-1) ;
#if	CF_DEBUGS
	    	    debugprintf("checker_progrun: spawner_envset() rs=%d\n",
			rs) ;
#endif
	                }
#if	CF_DEBUGS
	    	    debugprintf("checker_progrun: about rs=%d\n",rs) ;
#endif
	                if (rs >= 0) {
	                    int	i ;
#if	CF_BACKGROUND
	                    spawner_sigignores(&s) ;
	                    spawner_setsid(&s) ;
#endif
	                    for (i = 0 ; i < 3 ; i += 1) {
	                        spawner_fdclose(&s,i) ;
			    }
	                    spawner_fdnull(&s,O_RDONLY) ;
	                    spawner_fddup(&s,fd) ;
	                    spawner_fdnull(&s,O_WRONLY) ;
	                    if ((rs = spawner_run(&s)) >= 0) {
				pid_t	pid = rs ;
				int	cs = 0 ;

	                	rs1 = spawner_wait(&s,&cs,0) ;
	                	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	                        debugprintf("checker_progrun: w rs=%d cs=%d\n",
				    rs,cs) ;
#endif

				if (rs >= 0) {
				    rs = checker_proglog(chp,fd,pid,cs) ;
				}

	                        if (rs >= 0) u_rewind(fd) ;
			    } /* end if (spawner_run) */

#if	CF_DEBUGS
	                    debugprintf("checker_progrun: "
				"spawner_run() rs=%d\n", rs) ;
	                    debugoutput("checker_progrun: =",fd) ;
#endif

	                } /* end if (ok) */
#if	CF_DEBUGS
	    	    debugprintf("checker_progrun: ok-out rs=%d\n",rs) ;
#endif

	                rs1 = spawner_finish(&s) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (spawner) */
#if	CF_DEBUGS
	    	    debugprintf("checker_progrun: spawner-out rs=%d\n",rs) ;
#endif
	            if (rs < 0) {
			uc_unlink(qfname) ;
	                u_close(fd) ;
	                fd = -1 ;
	            }
	        } /* end if (u_open) */
#if	CF_DEBUGS
	    	    debugprintf("checker_progrun: open-out rs=%d\n",rs) ;
#endif
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


static int checker_proglog(CHECKER *chp,int fd,pid_t pid,int cs)
{
	MAINTQOTD	*sip = chp->sip ;
	int		rs = SR_OK ;

	if (sip->open.logsub) {
	    LOGFILE	*lhp = sip->logsub ;
	    uint	v = pid ;
	    cchar	*fmt ;
	    if (WIFEXITED(cs)) {
		int	ex = WEXITSTATUS(cs) ;
		fmt = "program (%u) exited normally ex=%u" ;
		logfile_printf(lhp,fmt,v,ex) ;
	    } else if (WIFSIGNALED(cs)) {
		int	sig = WTERMSIG(cs) ;
		cchar	*ss ;
		char	sigbuf[20+1] ;
		if ((ss = strsigabbr(sig)) == NULL) {
		     rs = ctdeci(sigbuf,20,sig) ;
		     ss = sigbuf ;
		}
		if (rs >= 0) {
		    fmt = "program (%u) exited w/ sig=%s" ;
		    logfile_printf(lhp,fmt,v,ss) ;
		}
	    } else {
		fmt = "program (%u) exited weirdly cs=\\x%08x" ;
		logfile_printf(lhp,fmt,v,cs) ;
	    }
	    if (rs >= 0) {
		if ((rs = uc_fsize(fd)) >= 0) {
		    logfile_printf(lhp,"quote size=%u",rs) ;
		}
	    }
	} /* end if (logging) */

	return rs ;
}
/* end subroutine (checker_proglog) */


#ifdef	COMMENT
static int mksfname(rbuf,pr,sdname,sname)
char		rbuf[] ;
cchar		*pr ;
cchar		*sdname ;
cchar		*sname ;
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
static int debugoutput(cchar *ids,int fd)
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


