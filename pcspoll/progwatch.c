/* progwatch */

/* process the service names given us */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGEXPS	0		/* debug exports */
#define	CF_DEBUGSUBS	0		/* debug var-subs */
#define	CF_ALWAYSDEF	0		/* always want the default access? */
#define	CF_FASTQ	1		/* fast Q allocation of SVCENTRYs */
#define	CF_LOGFILECHECK	1		/* logfile check */
#define	CF_SVCFILECHECK	1		/* check SVCFILE for changes */
#define	CF_ACCTABCHECK	1		/* check ACCTAB for changes */
#define	CF_SVCFILEFREE	0		/* free up SVCFILE occassionally? */
#define	CF_ACCTABFREE	0		/* free up ACCTAB occassionally? */
#define	CF_LOGONLY	0		/* log exit only w/ daemon? */
#define	CF_POLL		1		/* use 'poll(2)'? */
#define	CF_SPERM	1		/* use 'sperm(3dam)' */
#define	CF_SLOWGROW	1		/* slowly grow polling interval */
#define	CF_ENVLOCAL	0		/* use only local environment */
#define	CF_MKSUBLOGID	1		/* use 'mksublogid(3dam)' */
#define	CF_JOBDNAME	1		/* use "jobdname" */
#define	CF_PROCFINDPROG	0		/* use |procfindprog()| */


/* revision history:

	= 1999-09-01, David A­D­ Morano

	This subroutine was adopted from the DWD program.


	= 2009-03-05, David A­D­ Morano

	The SRVTAB object has been replaced with the SVCFILE object.
	Some changed were needed to this module to accomplish this.


*/

/* Copyright © 1999,2009 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for processing the jobs that
	have been handed to us from the initialization code.

	Returns:

	OK	may not really matter in the current implementation!
	<0	error


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<limits.h>
#include	<utime.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<field.h>
#include	<ids.h>
#include	<svcfile.h>
#include	<acctab.h>
#include	<cq.h>
#include	<spawner.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"svcentry.h"
#include	"svckey.h"


/* local defines */

#define	W_OPTIONS	(WNOHANG)
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#define	TO_STAMPLOCK	0		/* stamp-file lock */

#define	POLLMULT	1000

#define	JOBFRACTION	10

#ifndef	MAXOUTLEN
#define	MAXOUTLEN	64
#endif

#ifndef	XDEBFILE
#define	XDEBFILE	"/var/tmp/pcspoll.deb"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mksublogid(char *,int,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	prmktmpdir(const char *,char *,const char *,const char *,
			mode_t) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	getpwd(char *,int) ;
extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	isNotPresent(int) ;

extern int	progpidbegin(struct proginfo *,int) ;
extern int	progpidcheck(struct proginfo *) ;
extern int	progpidend(struct proginfo *) ;
extern int	progexec(struct proginfo *,const char *,vecstr *,vecstr *) ;
extern int	proglogout(struct proginfo *,const char *,const char *) ;
extern int	progsvccheck(struct proginfo *) ;
extern int	progacccheck(struct proginfo *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* externals variables */


/* local structures */

struct subinfo {
	varsub		*ssp ;		/* string substitutions */
	vecstr		*elp ;		/* export list */
	SVCFILE		*sfp ;		/* service file */
	ACCTAB		*atp ;		/* access table file */
	CQ		qfree, qcom ;	/* free and compute Qs */
	vechand		trun ;		/* run table */
	SVCENTRY_ARGS	args ;
	time_t		ti_lockcheck ;
	time_t		ti_pidcheck ;
	time_t		ti_stampcheck ;
	time_t		ti_logjobs ;
	time_t		ti_dbdump ;
	int		serial ;	/* serial number */
	int		to_minjob ;	/* interval */
	int		to_logjobs ;
	uint		f_error:1 ;	/* early exit on error */
} ;


/* forward references */

static int	procjobdname(struct proginfo *) ;
static int	procloadnames(struct proginfo *,vecstr *) ;
static int	procname(struct proginfo *,
			SVCENTRY_ARGS *,const char *) ;
static int	procservice(struct proginfo *,
			SVCFILE_ENT *,SVCENTRY_ARGS *) ;
static int	procnewprogentry(struct proginfo *,SVCENTRY **) ;
static int	procfreeprogentry(struct proginfo *,SVCENTRY *) ;
static int	procjobdel(struct proginfo *,int,SVCENTRY *) ;
static int	procruncheck(struct proginfo *) ;
static int	procjobfind(struct proginfo *,pid_t,SVCENTRY **) ;
static int	procjobstart(struct proginfo *,SVCENTRY *) ;
static int	procjobadd(struct proginfo *,SVCENTRY *,const char *) ;
static int	procmorecheck(struct proginfo *) ;
static int	procjobactive(struct proginfo *,
			const char *,SVCENTRY **) ;
static int	procaccess(struct proginfo *,
			const char *) ;

static int	procstampfile(struct proginfo *) ;
#if	CF_PROCFINDPROG
static int	procfindprog(struct proginfo *,const char *,char *,int *) ;
#endif

static int	procfreeall(struct proginfo *) ;
static int	proclogjobs(struct proginfo *) ;
static int	proclogjob(struct proginfo *,SVCENTRY *) ;
static int	proclogsecurity(struct proginfo *,const char *) ;

#if	CF_SVCFILEFREE || CF_ACCTABFREE
static int	procdbdump(struct proginfo *) ;
#endif /* CF_SVCFILEFREE || CF_ACCTABFREE */

#if	CF_ENVLOCAL
static int	loadexports(struct proginfo *,vecstr *) ;
#endif

static int	procxfile(struct proginfo *,const char *) ;

static void	int_all(int) ;


/* local variables */

static volatile int	if_term ;
static volatile int	if_int ;


/* exported subroutines */


int progwatch(pip,snp)
struct proginfo	*pip ;
vecstr		*snp ;
{
	struct sigaction	sigs ;
	struct subinfo	si, *sip = &si ;
	struct pollfd	fds[2] ;
	VARSUB		tabsubs ;
#if	CF_ENVLOCAL
	VECSTR		exports ;
#endif
	SVCENTRY	*pep ;
	sigset_t	sigmaskadds ;
	time_t		ti_start ;
	time_t		ti_checksvc ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	loopcount = 0 ;
	int	opts ;
	int	njobs ;
	int	to_poll ;
	int	to_jobslice ;
	int	to_checksvc ;
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	rs1 ;
	    const char	*cp ;
	    debugprintf("progwatch: entered \n") ;
	    debugprintf("progwatch: workdname=%s\n",pip->workdname) ;
	    rs1 = expcook_findkey(&pip->cooks,VARHOMEDNAME,-1,&cp) ;
	    debugprintf("progwatch: _findkey() rs=%d HOME=%s\n",rs1,cp) ;
	}
#endif /* CF_DEBUG */

	if_int = FALSE ;
	if_term = FALSE ;

#if	CF_DEBUG && CF_DEBUGEXPS
	if (DEBUGLEVEL(4)) {
	    vecstr	*elp = &pip->exports ;
	    int		i ;
	    int		n = 0 ;
	    const char	*cp ;
	    debugprintf("progwatch: elp={%p} exports¬\n",elp) ;
	    if ((rs = vecstr_count(elp)) >= 0) {
	        for (i = 0 ; vecstr_get(elp,i,&cp) >= 0 ; i += 1) {
		    if (cp == NULL) continue ;
		    n += 1 ;
	            debugprintf("progwatch: e=>%t<\n",
			cp,strlinelen(cp,-1,50)) ;
	        } /* end while (vecstr-get) */
	    } /* end if */
	    debugprintf("progwatch: exports rs=%d n=%u\n",rs,n) ;
	}
#endif /* CF_DEBUG */

	if (pip->daytime == 0)
	    pip->daytime = time(NULL) ;

/* before we go too far, are we the only one on this PID mutex? */

	if (! pip->f.named) {
	    const char	*pidmsg ;

	    rs = progpidbegin(pip,TO_PIDLOCK) ;

	    pidmsg = (rs >= 0) ? "PID mutex captured" : "PID mutex busy" ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s\n",pip->progname,pidmsg) ;

	    if (pip->open.logprog)
	        logfile_printf(&pip->lh,pidmsg) ;

	} /* end if (un-named mode) */

	if (rs < 0)
	    goto ret0 ;

	if (pip->f.daemon && pip->open.logprog) {
	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestr_logz(pip->daytime,timebuf)) ;
	    logfile_flush(&pip->lh) ;
	} /* end if (making log entries) */

	ti_start = pip->daytime ;
	ti_checksvc = pip->daytime ;

/* find a "job" directory for job-tmp files */

#if	CF_JOBDNAME
	rs = procjobdname(pip) ;
	if (rs < 0) goto ret0 ;
#endif /* CF_JOBDNAME */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progwatch: jobdname=%s\n",pip->jobdname) ;
#endif

/* initialize our local information */

	pip->sip = sip ;
	memset(sip,0,sizeof(struct subinfo)) ;

	sip->sfp = &pip->stab ;
	sip->atp = &pip->atab ;

	sip->serial = 0 ;
	sip->ti_dbdump = pip->daytime ;

	sip->to_minjob = -1 ;
	sip->to_logjobs = TO_LOGJOBS ;

	sip->ssp = &tabsubs ;
#if	CF_ENVLOCAL
	sip->elp = &exports ;
#else
	sip->elp = &pip->exports ;
#endif

#ifdef	OPTIONAL
	memset(&sip->args,0,sizeof(SVCENTRY_ARGS)) ;
#endif

	sip->args.version = VERSION ;
	sip->args.programroot = pip->pr ;
	sip->args.domainname = pip->domainname ;
	sip->args.nodename = pip->nodename ;
	sip->args.username = pip->username ;
	sip->args.groupname = pip->groupname ;
#if	CF_JOBDNAME
	sip->args.tmpdname = pip->jobdname ;
#else
	sip->args.tmpdname = pip->tmpdname ;
#endif /* CF_JOBDNAME */
	sip->args.hostname = NULL ;
	sip->args.service = NULL ;
	sip->args.interval = NULL ;
	sip->args.daytime = pip->daytime ;

	rs = cq_start(&sip->qfree) ;
	if (rs < 0)
	    goto ret1 ;

	rs = cq_start(&sip->qcom) ;
	if (rs < 0)
	    goto ret2 ;

	opts = (VECHAND_OSWAP | VECHAND_OCONSERVE) ;
	rs = vechand_start(&sip->trun,pip->maxjobs,opts) ;
	if (rs < 0)
	    goto ret3 ;

#if	CF_ENVLOCAL
	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&exports,100,opts) ;
	if (rs < 0)
	    goto ret4 ;

	rs = loadexports(pip,&exports) ;
	if (rs < 0)
	    goto ret5 ;
#endif /* CF_ENVLOCAL */

	rs = varsub_start(&tabsubs,0) ;
	if (rs < 0)
	    goto ret5 ;

	if (rs >= 0) {
	    rs = varsub_addvec(&tabsubs,&pip->exports) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progwatch: varsub_addvec() rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0)
	    rs = varsub_addva(&tabsubs,pip->envv) ;

#if	CF_DEBUG && CF_DEBUGSUBS
	if (DEBUGLEVEL(4)) {
	    VARSUB	*slp = &tabsubs ;
	    VARSUB_CUR	c ;
	    int		n = 0 ;
	    const char	*kp, *vp ;
	    debugprintf("progwatch: varsubs¬\n") ;
	    varsub_curbegin(slp,&c) ;
	    while (varsub_enum(slp,&c,&kp,&vp) >= 0) {
		n += 1 ;
	        debugprintf("progwatch: k=%s v=%t\n",kp,
		    vp,strlinelen(vp,-1,40)) ;
	    } /* end while (enum) */
	    varsub_curend(slp,&c) ;
	    debugprintf("progwatch: varsubs n=%u\n",n) ;
	}
#endif /* CF_DEBUG */

	if (rs < 0)
	    goto ret6 ;

/* screw the stupid signals */

	memset(&sigs,0,sizeof(struct sigaction)) ;

	uc_sigsetempty(&sigmaskadds) ;

	uc_sigignore(SIGPIPE) ;

	uc_sigignore(SIGHUP) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = sigmaskadds ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGQUIT,&sigs,NULL) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = sigmaskadds ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = sigmaskadds ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;

/* more initialization */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	rs1 ;
	    rs1 = vechand_count(&sip->trun) ;
	    debugprintf("progwatch: trun -1 count=%d\n",rs1) ;
	    rs1 = vechand_extent(&sip->trun) ;
	    debugprintf("progwatch: trun -1 extent=%d\n",rs1) ;
	}
#endif /* CF_DEBUG */

/* load up all of the jobs that we may have been given */

	rs = procloadnames(pip,snp) ;
	if (rs < 0)
	    goto ret5 ;

/* top of loop */

	loopcount = 0 ;
	while ((rs >= 0) && (! if_term)) {
	    int	f_log = FALSE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        int	rs1 ;
	        rs1 = vechand_count(&sip->trun) ;
	        debugprintf("progwatch: trun 1 count=%d\n",rs1) ;
	        rs1 = vechand_extent(&sip->trun) ;
	        debugprintf("progwatch: trun 1 extent=%d\n",rs1) ;
	    }
#endif /* CF_DEBUG */

#if	CF_SVCFILEFREE || CF_ACCTABFREE
	    if ((rs >= 0) && ((loopcount % 300) == 0))
		rs = procdbdump(pip,sip) ;
#endif /* CF_SVCFILEFREE || CF_ACCTABFREE */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: mid1 rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procruncheck(pip) ;
	        njobs = rs ;
		f_log = f_log || (njobs > 0) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progwatch: procruncheck() rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && pip->open.logprog) {
		rs = proclogjobs(pip) ;
		f_log = f_log || (rs > 0) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: mid2 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && pip->open.logprog && f_log) {
		logfile_flush(&pip->lh) ;
	    }

	    if (rs < 0) break ;

/* check for the case of no current jobs! */

	    if ((rs >= 0) && (njobs == 0)) {

	        if (! pip->f.daemon)
	            break ;

	        if (pip->intrun > 0) {
	            if (pip->daytime >= (ti_start + pip->intrun)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progwatch: run-interval expired\n") ;
#endif

	                break ;
		    }
	        }

#if	CF_SLOWGROW
	        to_poll = MAX((sip->to_minjob / JOBFRACTION),1) ;
#else
	        to_poll += 1 ;
#endif

	        if (to_poll > TO_POLL)
	            to_poll = TO_POLL ;

	    } else
	        to_poll = 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progwatch: mid3 rs=%d to_minjob=%d to_poll=%u\n",
	            rs,sip->to_minjob,to_poll) ;
#endif

	if (rs >= 0) {
#if	CF_POLL
	    {
		const int	pto = (to_poll * POLLMULT) ;
	        fds[0].fd = -1 ;
	        rs1 = u_poll(fds,0,pto) ;
	    }
#else
	    sleep(to_poll) ;
#endif
	} /* end if (sleep code) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("progwatch: u_poll() rs=%d \n",rs1) ;
	        debugprintf("progwatch: if_term=%u if_int=%u\n",
			if_term,if_int) ;
	}
#endif

	    pip->daytime = time(NULL) ;

	    sip->args.daytime = pip->daytime ;

/* check for new service interval expirations */

	    to_jobslice = MAX((sip->to_minjob / JOBFRACTION),1) ;

	    to_checksvc = MIN(pip->intpoll,to_jobslice) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progwatch: to_checksvc=%d\n",to_checksvc) ;
#endif

	    if (pip->f.daemon && 
		((pip->daytime - ti_checksvc) >= to_checksvc)) {

	        ti_checksvc = pip->daytime ;
	        rs = procmorecheck(pip) ;

	    } /* end if (checking timestamps) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: mid4 rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && if_int) {
		if_int = FALSE ;
		if (pip->debuglevel > 0)
		    bprintf(pip->efp,"%s: interrupt\n",pip->progname) ;
		if (pip->open.logprog)
	            logfile_printf(&pip->lh,"%s interrupt (%d)",
			timestr_logz(pip->daytime,timebuf),rs) ;
	    } /* end if */

/* check up on the log file */

#if	CF_LOGFILECHECK
	    if (pip->open.logprog)
	        logfile_check(&pip->lh,pip->daytime) ;
#endif

	    loopcount += 1 ;

	} /* end while (looping) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: while-out rs=%d if_term=%u\n",
		rs,if_term) ;
#endif

#if	CF_LOGONLY
	f = pip->f.daemon ;
#else
	f = TRUE ;
#endif

	if (f) {

	    const char	*fmt ;
	    const char	*s = (pip->f.daemon) ? "server" : "single" ;

	    pip->daytime = time(NULL) ;

	    if (pip->f.daemon && if_term) {
	        fmt = "%s %s exiting -terminated (%d)\n" ;

	    } else if (sip->f_error) {
	        fmt = "%s %s exiting -error (%d)\n" ;

	    } else if (pip->f.daemon) {
	        fmt = "%s %s exiting -expired (%d)\n" ;

	    } else
	        fmt = "%s %s exiting -completed (%d)\n" ;

	    if (pip->open.logprog)
	        logfile_printf(&pip->lh,fmt,
	            timestr_logz(pip->daytime,timebuf),s,rs) ;

	    if (pip->debuglevel > 0) {
		char	pn[MAXNAMELEN + 20] ;
		sncpy2(pn,(MAXNAMELEN + 10),pip->progname,":") ;
		bprintf(pip->efp,fmt,pn,s,rs) ;
	    }

	} /* end if (daemon mode exit stuff) */

/* early and regular exits */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: cleanup 0\n") ;
#endif

	procfreeall(pip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: cleanup 3\n") ;
#endif

ret6:
	varsub_finish(&tabsubs) ;

ret5:

#if	CF_ENVLOCAL
	vecstr_finish(&exports) ;
#endif

ret4:
	vechand_finish(&sip->trun) ;	/* run table */

ret3:
	cq_finish(&sip->qfree) ;

ret2:
	cq_finish(&sip->qcom) ;

ret1:
	progpidend(pip) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progwatch) */


/* local subroutines */


static int procjobdname(struct proginfo *pip)
{
	int		rs ;

	if (pip->jobdname == NULL) {
	    mode_t	dm ;
	    const char	*pr = pip->pr ;
	    const char	*pn = pip->progname ;
	    const char	*sn = pip->searchname ;
	    const char	*tmpdname = pip->tmpdname ;
	    char	jobdname[MAXPATHLEN+1] ;
	    switch (pip->tmptype) {
	    defualt:
	    case 0:
	        dm = 0777 ;
	        rs = prmktmpdir(pr,jobdname,tmpdname,sn,dm) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progwatch/procjobdname: prmktmpdir() rs=%d\n",
		        rs) ;
#endif
	        break ;
	    case 1:
	        {
		    dm = 0775 ;
		    const char	*un = pip->username ;
	            rs = mktmpuserdir(jobdname,un,sn,dm) ;
	        }
	        break ;
	    } /* end switch */
	    if (rs >= 0)
	        rs = proginfo_setentry(pip,&pip->jobdname,jobdname,rs) ;
	    if (pip->debuglevel > 0) {
		const int	ti = pip->tmptype ;
		bprintf(pip->efp,"%s: jobdname(%u)=%s\n",pn,ti,jobdname) ;
	    }
	} else
	    rs = strlen(pip->jobdname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progwatch/procjobdname: ret rs=%d\n",rs) ;
	    debugprintf("progwatch/procjobdname: jobdname=%s\n",pip->jobdname) ;
	}
#endif

	return rs ;
}
/* end subroutine (procjobdname) */


static int procloadnames(pip,snp)
struct proginfo	*pip ;
vecstr		*snp ;
{
	struct subinfo	*pcp = pip->sip ;
	SVCFILE		*sfp ;
	int		rs = SR_OK ;

	sfp = pcp->sfp ;
	if (pip->f.named) {
	    int		i ;
	    const char	*cp ;

	    for (i = 0 ; vecstr_get(snp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;

	        rs = procname(pip,&pcp->args,cp) ;

	        if (rs < 0) break ;
	    } /* end for */

	} else {
	    struct svckey	sk ;
	    SVCFILE_ENT		ste ;
	    SVCFILE_CUR		cur ;
	    char		vbuf[VBUFLEN + 1] ;

	    if ((rs = svcfile_curbegin(sfp,&cur)) >= 0) {

	    while (svcfile_enum(sfp,&cur,&ste,vbuf,VBUFLEN) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progwatch: svc=%s\n",ste.svc) ;
#endif

	        svckey_load(&sk,&ste) ;

	        if (sk.interval == NULL)
	            continue ;

	        if (pip->f.daemon && (sk.interval == NULL))
	            continue ;

	        rs = procservice(pip,&ste,&pcp->args) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progwatch: procservice() 1 rs=%d\n",rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end for */

	    svcfile_curend(sfp,&cur) ;
	} /* end if (cursor) */

	} /* end if (named or not) */

	return rs ;
}
/* end subroutine (procloadnames) */


static int procname(pip,pap,name)
struct proginfo	*pip ;
SVCENTRY_ARGS	*pap ;
const char	name[] ;
{
	struct subinfo	*pcp = pip->sip ;
	SVCFILE_ENT	ste ;
	int		rs ;
	char		vbuf[VBUFLEN + 1] ;

	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

	if ((rs = svcfile_fetch(pcp->sfp,name,NULL,&ste,vbuf,VBUFLEN)) >= 0) {
	    rs = procservice(pip,&ste,pap) ;
	}

	return rs ;
}
/* end subroutine (procname) */


/* get a new SVCENTRY object */
static procnewprogentry(pip,pepp)
struct proginfo	*pip ;
SVCENTRY	**pepp ;
{
	struct subinfo	*pcp = pip->sip ;
	int		rs = SR_OK ;
	int		rs1 = SR_EMPTY ;

#if	CF_FASTQ
	rs1 = cq_rem(&pcp->qfree,(void **) pepp) ;
#endif /* CF_FASTQ */

	if (rs1 == SR_EMPTY) {
	    rs = uc_malloc(sizeof(SVCENTRY),pepp) ;
	}

	return rs ;
}
/* end subroutine (procnewprogentry) */


/* free up or re-Q up (whatever) an old SVCENTRY object */
static procfreeprogentry(pip,pep)
struct proginfo	*pip ;
SVCENTRY	*pep ;
{
	struct subinfo	*pcp = pip->sip ;
	int		rs1 = SR_EMPTY ;

	if (pep == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfreeprogentry: pep=%p\n",pep) ;
#endif

#if	CF_FASTQ
	rs1 = cq_ins(&pcp->qfree,(void *) pep) ;
#endif /* CF_FASTQ */

	if (rs1 < 0)
	    uc_free(pep) ;

	return SR_OK ;
}
/* end subroutine (procfreeprogentry) */


/* do a cycle of work */
static int procruncheck(pip)
struct proginfo	*pip ;
{
	struct subinfo	*pcp = pip->sip ;
	SVCENTRY	*pep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cstat ;
	int		ncom = 0 ;
	int		nrun = 0 ;
	char		timebuf[TIMEBUFLEN + 1] ;

	ncom = cq_count(&pcp->qcom) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procruncheck: cq_count() ncom=%d\n",ncom) ;
#endif

	nrun = vechand_count(&pcp->trun) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procruncheck: vechand_count() nrun=%d\n",nrun) ;
#endif

	if ((nrun >= 0) && (ncom >= 0)) {

	    if ((ncom > 0) && (nrun < pip->maxjobs)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("procruncheck: ncom=%d\n",ncom) ;
#endif

	        rs1 = cq_rem(&pcp->qcom,(void **) &pep) ;
	        if (rs1 >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                int	i ;
	                const char	*cp ;
	                debugprintf("procruncheck: "
	                    "dequeued rs=%d pep=%08lx\n",
	                    rs1,pep) ;
	                debugprintf("procruncheck: "
	                    "dequeued rs=%d svc=%s p=%s\n",
	                    rs1,pep->name,pep->program) ;
#ifdef	COMMENT
	                debugprintf("procruncheck: dumping exports\n") ;
	                for (i = 0 ; vecstr_get(pcp->elp,i,&cp) >= 0 ; i += 1) {
	                    if (cp == NULL) continue ;
	                    debugprintf("procruncheck: CP=%p\n",cp) ;
	                    debugprintf("procruncheck: export> %t\n",
	                        cp,strnlen(cp,50)) ;
	                } /* end for */
#endif /* COMMENT */
	            }
#endif /* CF_DEBUG */

	            ncom -= 1 ;
	            rs1 = procjobstart(pip,pep) ;

	            if (rs1 < 0) {

	                if (pip->open.logprog) {
	                    logfile_setid(&pip->lh,pep->jobid) ;
	                    logfile_printf(&pip->lh,
	                        "could not start job=%s (%d)\n",
	                        pep->name,rs1) ;
	                    logfile_setid(&pip->lh,pip->logid) ;
			}

	                svcentry_finish(pep) ;

	                procfreeprogentry(pip,pep) ;

	            } else
	                nrun += 1 ;

	        } /* end if */

	    } /* end if (starting new jobs) */

	} /* end if */

/* are there any completed jobs yet? */

	if ((rs1 = u_waitpid(-1,&cstat,W_OPTIONS)) > 0) {
	    pid_t	pid = rs1 ;
	    int		ji ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procruncheck: child exit pid=%d stat=%u\n",
	            pid,(cstat & 0xFF)) ;
#endif

	    if ((ji = procjobfind(pip,pid,&pep)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	            debugprintf("procruncheck: found child ji=%d pid=%d\n",
	                ji,pep->pid) ;
#endif

	        if (pip->open.logprog) {

	            logfile_setid(&pip->lh,pep->jobid) ;

	            logfile_printf(&pip->lh, "%s server(%d) exit ex=%u\n",
	                timestr_logz(pip->daytime,timebuf),pid,
	                (cstat & 255)) ;

/* process this guy's termination */

	            if (rs >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("procruncheck: proglogout() stderr\n") ;
#endif
	                rs = proglogout(pip, "stderr", pep->efname) ;
		    }
	            if (rs >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("procruncheck: proglogout() stdout\n") ;
#endif
	                rs = proglogout(pip, "stdout", pep->ofname) ;
		    }

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("procruncheck: proglogout() rs=%d\n",rs) ;
#endif

	            timestr_elapsed((pip->daytime - pep->stime),timebuf) ;
	            logfile_printf(&pip->lh,"elapsed runtime %s\n",timebuf) ;

	        } /* end if (have logging) */

	        procjobdel(pip,ji,pep) ;

	        if (pip->open.logprog)
	            logfile_setid(&pip->lh,pip->logid) ;

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procruncheck: unknown pid (%d)\n",rs1) ;
#endif

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"unknown PID=%d\n",pid) ;

	    } /* end if */

	} /* end if (handle a process exit) */

/* maintenance the PID mutex lock file */

	if (pip->open.pidlock &&
	    ((pip->daytime - pcp->ti_pidcheck) > TO_MAINT)) {

	    pcp->ti_pidcheck = pip->daytime ;
	    rs = progpidcheck(pip) ;
	    if (rs < 0)
	        pcp->f_error = TRUE ;

	} /* end if (maintaining the PID mutex file) */

/* check if the service file (svcfile) has changed */

#if	CF_SVCFILECHECK
	if ((rs >= 0) && (! pip->f.named)) {

	    rs1 = progsvccheck(pip) ;

#ifdef	COMMENT /* already done */
	    if ((rs1 > 0) && pip->open.logprog)
	        logfile_printf(&pip->lh,
	            "%s service file changed\n",
	            timestr_logz(pip->daytime,timebuf)) ;
#endif /* COMMENT */

	}
#endif /* CF_SVCFILECHECK */

/* check if the access file (acctab) has changed */

#if	CF_ACCTABCHECK
	if ((rs >= 0) && (! pip->f.named) && pip->open.accfname) {

	    rs1 = progacccheck(pip) ;

	    if ((rs1 > 0) && pip->open.logprog)
	        logfile_printf(&pip->lh,"%s access table changed\n",
	            timestr_logz(pip->daytime,timebuf)) ;

	} /* end if (ACCTAB) */
#endif /* CF_ACCTABCHECK */

/* maintenance the stamp-file (our own) */

	if ((rs >= 0) && pip->f.stampfname) {
	    const int	to = TO_STAMP ;

	    if (pcp->ti_stampcheck == 0)
	        pcp->ti_stampcheck = pip->daytime ;

	    if ((pip->daytime - pcp->ti_stampcheck) >= to) {
		pcp->ti_stampcheck = pip->daytime ;
	        procstampfile(pip) ;
	    } /* end if */

	} /* end if (stamp-file) */

/* done */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procruncheck: ret rs=%d njobs=%d\n",
	        rs,(ncom + nrun)) ;
#endif

	return (rs >= 0) ? (ncom + nrun) : rs ;
}
/* end subroutine (procruncheck) */


/* spawn a job */
static int procjobstart(pip,pep)
struct proginfo	*pip ;
SVCENTRY	*pep ;
{
	struct subinfo	*pcp = pip->sip ;
	pid_t		pid ;
	int		rs ;
	int		i ;
	const char	*pfname ;
	char		progfname[MAXPATHLEN + 2] ;

/* can we execute this service daemon? */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procjobstart: X-check program=>%s<\n",
	        pep->program) ;
#endif

	progfname[0] = '\0' ;
#if	CF_PROCFINDPROG
	{
	    int	f_secure ;
	    pfname = pep->program ;
	    rs = procfindprog(pip,pep->program,progfname,&f_secure) ;
	}
#else
	pfname = pep->program ;
	rs = prgetprogpath(pip->pr,progfname,pep->program,-1) ;
#endif /* CF_PROCFINDPROG */

	if (rs > 0)
	    pfname = progfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procjobstart: get-find rs=%d p=>%s<\n",
	        rs,pfname) ;
#endif /* CF_DEBUG */

	if (rs < 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procjobstart: could not execute\n") ;
#endif

	    if (pip->open.logprog) {
	        logfile_printf(&pip->lh,"cannot execute server \n") ;
	        logfile_printf(&pip->lh,"program=%s\n",pep->program) ;
	    }

	    rs = SR_NOEXIST ;
	    goto bad0 ;

	} /* end if */

#ifdef	COMMENT
	if (rs > 0) {
	    if ((pep->program[0] != '/') && (progfname[0] != '/'))
	        mkpath2(progfname, pip->pwd,pep->program) ;
	    pfname = progfname ;
	}
#endif /* COMMENT */

/* start 'er up */

	svcentry_stime(pep,pip->daytime) ;

	rs = procjobadd(pip,pep,pfname) ;
	pid = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procjobstart: server pid=%d\n",
	        pid) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procjobstart: job-proc pid=%d\n",pep->pid) ;
#endif

bad0:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procjobstart: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procjobstart) */


static int procjobadd(pip,pep,pfname)
struct proginfo	*pip ;
SVCENTRY	*pep ;
const char	*pfname ;
{
	struct subinfo	*pcp = pip->sip ;
	pid_t		pid = 0 ;
	int		rs ;
	if ((rs = vechand_add(&pcp->trun,pep)) >= 0) {
	    VECSTR	*elp = &pip->exports ;
	    SPAWNER	s ;
	    int		ji = rs ;
	    const char	*pf = pfname ;
	    const char	*sfo = pep->ofname ;
	    const char	*sfe = pep->efname ;
	    const char	**av, **ev ;
	    bflush(pip->efp) ;
	    if ((rs = svcentry_getargs(pep,&av)) >= 0) {
	        if (pip->open.logprog) {
		    logfile_setid(&pip->lh,pep->jobid) ;
		    if ((av != NULL) && (av[0] != NULL)) {
		        int		al ;
		        const char	*ap ;
		        if ((al = sfbasename(av[0],-1,&ap)) > 0)
	                    logfile_printf(&pip->lh,"server=%t\n",ap,al) ;
		    }
	            logfile_flush(&pip->lh) ;
	        } /* end if (log-open) */
	        if ((rs = vecstr_getvec(elp,&ev)) >= 0) {
	    	    if ((rs = u_open(sfe,O_WRONLY,0666)) >= 0) {
			int	fde = rs ;
	    	        if ((rs = u_open(sfo,O_WRONLY,0666)) >= 0) {
			    int	fdo = rs ;
	                    if ((rs = spawner_start(&s,pf,av,ev)) >= 0) {
				int	i ;
	                        if (pip->uid != pip->euid) 
			            spawner_seteuid(&s,pip->uid) ;
			        if (pip->gid != pip->egid)
			            spawner_setegid(&s,pip->gid) ;
			        for (i = 0 ; i < 3 ; i += 1) {
			            spawner_fdclose(&s,i) ;
				}
			        spawner_fdnull(&s,O_RDONLY) ;
			        spawner_fddup(&s,fdo) ;
			        spawner_fddup(&s,fde) ;
		                if ((rs = spawner_run(&s)) >= 0) {
				    pep->pid = rs ;
		                    pid = rs ;
		                }
	                        spawner_finish(&s) ;
	                    } /* end if (spawner) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progwatch/procjobadd: spawner rs=%d\n",rs) ;
#endif
			    u_close(fdo) ;
		        } /* end if (stdout) */
			u_close(fde) ;
		    } /* end if (stderr) */
	        } /* end if (get-envv) */
	    } /* end if (get-argv) */
	    if (rs < 0)
	        vechand_del(&pcp->trun,ji) ;
	    if ((rs >= 0) && pip->open.logprog) {
	        rs = proclogsecurity(pip,pfname) ;
	        logfile_setid(&pip->lh,pip->logid) ;
	    }
	} /* end if (SVC-entry add) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progwatch/procjobadd: ret rs=%d pid=%d\n",rs,pid) ;
#endif
	if (rs >= 0) rs = (pid&INT_MAX) ;
	return rs ;
}
/* end subroutine (procjobadd) */


/* find a job by its PID */
static int procjobfind(pip,pid,pepp)
struct proginfo	*pip ;
pid_t		pid ;
SVCENTRY	**pepp ;
{
	struct subinfo	*pcp = pip->sip ;
	SVCENTRY	*pep ;
	int		rs = SR_OK ;
	int		i ;

	if (pepp == NULL)
	    pepp = &pep ;

	for (i = 0 ; (rs = vechand_get(&pcp->trun,i,pepp)) >= 0 ; i += 1) {
	    if (*pepp != NULL) {
	        if (pid == (*pepp)->pid) break ;
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (procjobfind) */


/* delete a job and free up the progentry */
static int procjobdel(pip,ji,pep)
struct proginfo	*pip ;
int		ji ;
SVCENTRY	*pep ;
{
	struct subinfo	*pcp = pip->sip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = svcentry_finish(pep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_del(&pcp->trun,ji) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = procfreeprogentry(pip,pep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procjobdel) */


/* check on program intervals when in daemon mode */
static int procmorecheck(pip)
struct proginfo	*pip ;
{
	struct subinfo	*pcp = pip->sip ;
	SVCFILE		*sfp ;
	SVCFILE_CUR	cur ;
	SVCFILE_ENT	ste ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progwatch/procmorecheck: ent\n") ;
#endif

	pcp->to_minjob = -1 ;

/* update the time-of-day for newly created SVCENTRY objects */

	pcp->args.daytime = pip->daytime ;

/* loop finding suitable jobs to restart */

	sfp = pcp->sfp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progwatch/procmorecheck: sfp{%p}\n",sfp) ;
#endif

	if ((rs = svcfile_curbegin(sfp,&cur)) >= 0) {
	    struct svckey	sk ;
	    SVCENTRY		*pep ;
	    char		vbuf[VBUFLEN + 1] ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progwatch/procmorecheck: while-before\n") ;
#endif

	    while (svcfile_enum(sfp,&cur,&ste,vbuf,VBUFLEN) >= 0) {

	        svckey_load(&sk,&ste) ;

	        if (sk.interval == NULL) continue ;

/* is this service already active? (if not, check it) */

	    if (procjobactive(pip,ste.svc,&pep) == 0) {

	        rs = procservice(pip,&ste,&pcp->args) ;
	        if (rs > 0)
	            n += 1 ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progwatch/procmorecheck: "
			"procservice() 1 rs=%d\n",rs) ;
#endif

	    } else {
	        int	interval ;

	        if (svcentry_getinterval(pep,&interval) >= 0) {
		    const int	to = pcp->to_minjob ;

	            if ((interval < to) || (to < 0))
	                pcp->to_minjob = interval ;

	        }

	    } /* end if */

	} /* end while (looping through all services) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("progwatch/procmorecheck: while-after\n") ;
#endif

	    rs1 = svcfile_curend(sfp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (svcfile-cur) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progwatch/procmorecheck: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procmorecheck) */


/* check the interval of a service entry */
static int procservice(pip,sep,pap)
struct proginfo	*pip ;
SVCFILE_ENT	*sep ;
SVCENTRY_ARGS	*pap ;
{
	struct subinfo	*pcp = pip->sip ;
	struct ustat	sb ;
	SVCENTRY	*pep ;
	bfile		tsfile ;
	mode_t		oldmask ;
	int		rs ;
	int		interval ;
	int		f_process = FALSE ;
	const char	*access ;
	char		stampfname[MAXPATHLEN + 1] ;
	char		jobid[SVCENTRY_IDLEN + 2] ;
	char		timebuf[TIMEBUFLEN + 1] ;

	if ((pcp == NULL) || (sep == NULL))
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procservice: svc=%s\n", sep->svc) ;
	    debugprintf("procservice: pap->tmpdname=%s\n",pap->tmpdname) ;
	}
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: 1 stampdname=%s\n",
	        pip->stampdname) ;
#endif

/* allocate a new SVCENTRY */

	rs = procnewprogentry(pip,&pep) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: newprogentry() rs=%d pep=%08lx\n",
	        rs,pep) ;
#endif

	if (rs < 0)
	    goto bad0 ;

/* initialize this SVCENTRY object */

	rs = svcentry_start(pep,pcp->ssp,sep,pap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: svcentry_start() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* see if this service is allowed on the host we are running on! */

	rs = svcentry_getaccess(pep,&access) ;

	if ((rs < 0) && (rs != SR_EMPTY))
	    goto bad2 ;

/* check no matter what since there may be a default access restriction! */

	rs = procaccess(pip,access) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: procaccess() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

/* see if the interval has expired */

	rs = svcentry_getinterval(pep,&interval) ;
	if (rs < 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procservice: got a bad interval\n") ;
#endif

	    goto bad2 ;
	}

	if ((interval < 0) && (! pip->f.named)) {
	    rs = 0 ;
	    goto ret1 ;
	}

	if (pip->f.named && (interval < 0))
	    interval = 0 ;

	if ((pcp->to_minjob < 0) || (pcp->to_minjob > interval))
	    pcp->to_minjob = interval ;

/* we are ready for the timestamp check */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: stampdname=%s\n",
	        pip->stampdname) ;
#endif

	mkpath2(stampfname,pip->stampdname,sep->svc) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("procservice: stampfname=%s\n",
	        stampfname) ;
	    debugprintf("procservice: interval=%d\n",
	        interval) ;
	}
#endif /* CF_DEBUG */

	oldmask = umask(0000) ;
	rs = bopen(&tsfile,stampfname,"wc",0666) ;
	umask(oldmask) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procservice: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: created=%s\n",
	        ((rs == SR_CREATED) ? "yes" : "no")) ;
#endif

	f_process = FALSE ;
	if (rs != SR_CREATED) {

	    rs = bcontrol(&tsfile,BC_LOCK,TO_STAMPLOCK) ;
	    if (rs >= 0) {

	        rs = bcontrol(&tsfile,BC_STAT,&sb) ;
	        if (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2)) {
	                debugprintf("procservice: daytime=%s\n",
	                    timestr_logz(pip->daytime,timebuf)) ;
	                debugprintf("procservice: mtime=%s\n",
	                    timestr_logz(sb.st_mtime,timebuf)) ;
	            }
#endif /* CF_DEBUG */

	            if ((pip->daytime - sb.st_mtime) > interval)
	                f_process = TRUE ;

	        }

	    } else
	        f_process = FALSE ;

	} else {

	    f_process = TRUE ;
	    bcontrol(&tsfile,BC_CHMOD,0666) ;

	} /* end if */

	if (rs < 0)
	    goto bad3 ;

/* at this point, if 'f_process' is TRUE, that means to run that job! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procservice: rs=%d should we process ans=%d\n",
	        rs,((rs >= 0) && f_process)) ;
#endif

	if ((rs >= 0) && f_process) {

/* create a job ID for this job */

	    pap->jobid = jobid ;

#if	CF_MKSUBLOGID
	    rs = mksublogid(jobid,SVCENTRY_IDLEN,pip->logid,pcp->serial) ;
#else
	    rs = snsdd(jobid,SVCENTRY_IDLEN,pip->logid,pcp->serial) ;
#endif /* CF_MKSUBLOGID */

	    pcp->serial += 1 ;
	    if (pip->open.logprog)
	        logfile_setid(&pip->lh,jobid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procservice: %s svc=%s\n",
	            timestr_logz(pip->daytime,timebuf),
	            pep->name) ;
#endif /* CF_DEBUG */

	    if (pip->open.logprog)
	        logfile_printf(&pip->lh,"%s svc=%s\n",
	            timestr_logz(pip->daytime,timebuf),
	            pep->name) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s svc=%s\n",
	            pip->progname,
	            timestr_logz(pip->daytime,timebuf),
	            pep->name) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procservice: svcentry_expand()\n") ;
#endif

	    if (rs >= 0)
	        rs = svcentry_expand(pep,sep,pap) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("procservice: svcentry_expand() rs=%d\n", rs) ;
	        debugprintf("procservice: svc=%s p=>%s<\n",
	            pep->name,pep->program) ;
	    }
#endif

	    if (rs >= 0) { /* success */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procservice: cq_ins() pep=%08lx\n",pep) ;
#endif

	        rs = cq_ins(&pcp->qcom,pep) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procservice: cq_ins() rs=%d\n", rs) ;
#endif

	        if (rs < 0)
	            goto bad3 ;

/* update the time stamp by writing to it */

	        bprintf(&tsfile,"%s\n",
	            timestr_logz(pip->daytime,timebuf)) ;

	    } else { /* failure */

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,
	                "could not expand service entry (%d)\n",
	                rs) ;

	    } /* end if */

	    if (pip->open.logprog)
	        logfile_setid(&pip->lh,pip->logid) ;

	} /* end if (processing this service) */

	bclose(&tsfile) ;

/* OK, let's get out but first clean up as necessary */

	if ((rs < 0) || (! f_process)) {

	    svcentry_finish(pep) ;

	    procfreeprogentry(pip,pep) ;

	} /* end if (more failure stuff) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procservice: ret rs=%d f_p=%u\n", 
		rs,f_process) ;
#endif

	return (rs >= 0) ? f_process : rs ;

/* bad things */
bad3:
	bclose(&tsfile) ;

bad2:
ret1:
	svcentry_finish(pep) ;

bad1:
	rs = procfreeprogentry(pip,pep) ;

bad0:
	goto ret0 ;
}
/* end subroutine (procservice) */


/* is a named job active in the system already? */
static int procjobactive(pip,name,pepp)
struct proginfo	*pip ;
const char	name[] ;
SVCENTRY	**pepp ;
{
	struct subinfo	*pcp = pip->sip ;
	CQ_CUR		cur ;
	SVCENTRY	*pep ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; vechand_get(&pcp->trun,i,&pep) >= 0 ; i += 1) {
	    if (pep == NULL) continue ;

	    f = (strcmp(pep->name,name) == 0) ;
	    if (f) break ;

	} /* end for */

	if (! f) {
	    if ((rs = cq_curbegin(&pcp->qcom,&cur)) >= 0) {

	        while (cq_enum(&pcp->qcom,&cur,&pep) >= 0) {
	            f = (strcmp(pep->name,name) == 0) ;
		    if (f) break ;
	        } /* end while */

	        cq_curend(&pcp->qcom,&cur) ;
	    } /* end if (cursor) */
	} /* end if */

	if (pepp != NULL)
	    *pepp = (f) ? pep : NULL ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procjobactive) */


/**** PROGRAM NOTE:

	At this point, we have a processed (variable substituted, et
	cetera) server entry in the SVCENTRY object.  From here
	on, we want to check the server access list to see if this
	current job execution should be allowed.

****/

/* check if this connection is allowed based on the service access */
static int procaccess(pip,access)
struct proginfo	*pip ;
const char	access[] ;
{
	struct subinfo	*pcp = pip->sip ;
	FIELD		af ;
	vecstr		netgroups, names ;
	int		rs = SR_OK ;
	int		i, j ;
	int		fl ;
	int		cl ;
	const char	*fp ;
	const char	*cp ;
	char		hostname[MAXHOSTNAMELEN + 2] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procaccess: access=>%s<\n",
	        access) ;
#endif

	if (access == NULL)
	    goto ret0 ;

	if (pip->defacc == NULL)
	    goto ret0 ;

	rs = vecstr_start(&netgroups,4,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_start(&names,4,0) ;
	if (rs < 0)
	    goto ret1 ;

/* process the server access list */

	if ((rs = field_start(&af,cp,-1)) >= 0) {

	    while ((fl = field_get(&af,NULL,&fp)) >= 0) {

	        if (fl > 0) {
	            int		bl = fl ;
	            const char	*bp = fp ;
	            while ((rs >= 0) && ((cl = nextfield(bp,bl,&cp)) > 0)) {
	                rs = vecstr_add(&netgroups,cp,cl) ;
	                bl -= ((cp + cl) - bp) ;
	                bp = (cp + cl) ;
	            } /* end while */
	        } /* end if */

	    } /* end while */

	    field_finish(&af) ;
	} /* end if */

#if	CF_ALWAYSDEF
	if (vecstr_find(&netgroups,"DEFAULT") < 0)
	    vecstr_add(&netgroups,"DEFAULT",-1) ;
#else
	if (vecstr_count(&netgroups) <= 0)
	    vecstr_add(&netgroups,"DEFAULT",-1) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("procaccess: netgroups:\n") ;
	    for (i = 0 ; vecstr_get(&netgroups,i,&cp) >= 0 ; i+= 1)
	        debugprintf("procaccess: ng=%s\n",cp) ;
	}
#endif

/* our own machine names */

	if (rs >= 0)
	    rs = vecstr_add(&names,pip->nodename,-1) ;

	if ((rs >= 0) && (pip->domainname != NULL)) {

	    rs = snsds(hostname,MAXHOSTNAMELEN,
	        pip->nodename,pip->domainname) ;

	    if (rs > 0)
	        rs = vecstr_add(&names,hostname,rs) ;

	} /* end if (creating our hostname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("procaccess: hostnames rs=%d\n",rs) ;
	    for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1)
	        debugprintf("procaccess: mname=%s\n",cp) ;
	}
#endif

	if (rs < 0)
	    goto ret2 ;

/* try our own netgroups */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procaccess: checking access\n") ;
#endif

	if ((rs >= 0) && pip->open.accfname) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procaccess: checking ACCTAB\n") ;
#endif

	    rs = acctab_anyallowed(pcp->atp,&netgroups,&names,
	        pip->username,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procaccess: acctab rs=%d\n",
	            rs) ;
#endif

	} /* end if (ACCTAB check) */

/* try the system netgroups (UNIX does not have one simple call as above!) */

	if ((! pip->open.accfname) || (rs < 0)) {
	    const char	*ngp, *mnp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procaccess: sys NETGROUP DB\n") ;
#endif

	    for (i = 0 ; (rs = vecstr_get(&netgroups,i,&ngp)) >= 0 ; 
	        i += 1) {

	        if (ngp == NULL) continue ;

	        for (j = 0 ; (rs = vecstr_get(&names,j,&mnp)) >= 0 ; 
	            j += 1) {

	            if (mnp == NULL) continue ;

	            if (isdigit(mnp[0]))
	                continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf(
	                    "procaccess: sysnetgr n=%s m=%s\n",
	                    ngp,mnp) ;
#endif

	            if (innetgr(ngp,mnp,NULL,pip->domainname))
	                break ;

	        } /* end for (machine names) */

	        if (rs >= 0) break ;
	    } /* end for (netgroups) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procaccess: system rs=%d\n",
	            rs) ;
#endif

	} /* end if (checking UNIX netgroups) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procaccess: anyallowed rs=%d\n",rs) ;
#endif

ret2:
	vecstr_finish(&names) ;

ret1:
	vecstr_finish(&netgroups) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procaccess: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procaccess) */


static int procstampfile(pip)
struct proginfo	*pip ;
{
	int		rs = SR_OK ;

	if ((pip->stampfname != NULL) && (pip->stampfname[0] != '\0'))
	    rs = u_utime(pip->stampfname,NULL) ;

	return rs ;
}
/* end subroutine (procstampfile) */


/* find a program server and evaluate its security */
#if	CF_PROCFINDPROG
static int procfindprog(pip,program,progpath,sp)
struct proginfo	*pip ;
const char	program[] ;
char		progpath[] ;
int		*sp ;			/* secure path? */
{
	vecstr		already ;
	int		rs = SR_OK ;
	int		i ;
	int		sl = 0 ;
	const char	*cp ;

	if (program[0] == '/') {

	    sl = 0 ;
	    *sp = pip->f.secure_svcfile ;
	    rs = procxfile(pip,program) ;

	} else if ((rs = vecstr_start(&already,3,0)) >= 0) {

/* search the PCS area(s) first */

	    if ((rs = mkpath3(progpath,pip->pr,"bin",program)) >= 0) {
	        sl = rs ;
	        rs = vecstr_add(&already,progpath,sl) ;

	    *sp = pip->f.secure_root ;
	    if (rs >= 0)
	        rs = procxfile(pip,progpath) ;

/* PCS 'sbin' */

	    if (isNotPresent(rs)) {

	        rs = mkpath3(progpath,pip->pr,"sbin",program) ;
	        sl = rs ;
	        if (rs >= 0)
	            vecstr_add(&already,progpath,sl) ;

	        *sp = pip->f.secure_root ;
	        if (rs >= 0)
	            rs = procxfile(pip,progpath) ;

	    } /* end if (PCS sbin) */

/* do we need to search the given search path? */

	    if (isNotPresent(rs)) {

	        *sp = pip->f.secure_path ;
	        for (i = 0 ; vecstr_get(&pip->pathexec,i,&cp) >= 0 ; i += 1) {
	            const char	*pp ;

	            if (cp == NULL) continue ;

	            rs = sl = 0 ;
	            pp = program ;
	            if (cp[0] != '\0') {
	                pp = progpath ;
	                rs = mkpath2(progpath,cp,program) ;
	                sl = rs ;
	            }

	            if ((cp[0] == '\0') || 
	                (vecstr_find(&already,pp) < 0)) {

	                if (rs >= 0)
	                    rs = procxfile(pip,pp) ;

	                if (rs >= 0) break ;
	            }

	        } /* end for (extra search paths) */

	    } /* end if (checking extra search path) */

/* do we need to search regular directories? */

	    cp = "/usr/bin" ;
	    if (isNotPresent(rs) && (vecstr_find(&pip->pathexec,cp) < 0)) {

	        *sp = TRUE ;
	        rs = mkpath2(progpath,cp,program) ;
	        sl = rs ;
	        if (rs >= 0)
	            rs = procxfile(pip,progpath) ;

	    } /* end if (checking regular area) */

	    cp = "/usr/sbin" ;
	    if (isNotPresent(rs) && (vecstr_find(&pip->pathexec,cp) < 0)) {

	        *sp = TRUE ;
	        rs = mkpath2(progpath,cp,program) ;
	        sl = rs ;
	        if (rs >= 0)
	            rs = procxfile(pip,progpath) ;

	    } /* end if (checking regular area) */

	    } /* end if */
	    vecstr_finish(&already) ;
	} /* end if (vecstr) */

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (procfindprog) */
#endif /* CF_PROCFINDPROG */


static int procfreeall(pip)
struct proginfo	*pip ;
{
	struct subinfo	*sip = pip->sip ;
	SVCENTRY	*pep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	while (cq_rem(&sip->qfree,(void **) &pep) >= 0) {
	    if (pep != NULL) {
	        rs1 = uc_free(pep) ;
		if (rs >= 0) rs = rs1 ;
	        pep = NULL ;
	    }
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: cleanup 1\n") ;
#endif

	while (cq_rem(&sip->qcom,(void **) &pep) >= 0) {
	    if (pep != NULL) {
	        rs1 = svcentry_finish(pep) ;
		if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(pep) ;
		if (rs >= 0) rs = rs1 ;
	        pep = NULL ;
	    }
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progwatch: cleanup 2\n") ;
#endif

	for (i = 0 ; vechand_get(&sip->trun,i,&pep) >= 0 ; i += 1) {
	    if (pep != NULL) {
	        rs1 = svcentry_finish(pep) ;
		if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(pep) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (procfreeall) */


static int proclogjobs(pip)
struct proginfo	*pip ;
{
	struct subinfo	*sip = pip->sip ;
	SVCENTRY	*pep ;
	CQ_CUR		cur ;
	int		rs = SR_OK ;
	int		i ;
	int		to ;
	int		njobs = 0 ;

	if (! pip->open.logprog)
	    goto ret0 ;

#ifdef	COMMENT
	to = (sip->to_logjobs / 5) ;
#else
	to = sip->to_logjobs ;
#endif /* COMMENT */
	if (to == 0) to = 2 ;

	if ((pip->daytime - sip->ti_logjobs) < to)
	    goto ret0 ;

	sip->ti_logjobs = pip->daytime ;

/* run-Q */

	for (i = 0 ; vechand_get(&sip->trun,i,&pep) >= 0 ; i += 1) {
	    if (pep == NULL) continue ;

	    njobs += 1 ;
	    rs = proclogjob(pip,pep) ;

	    if (rs < 0) break ;
	} /* end for */

/* wait-Q */

	if (rs >= 0) {
	    if ((rs = cq_curbegin(&sip->qcom,&cur)) >= 0) {

	        while (cq_enum(&sip->qcom,&cur,&pep) >= 0) {
		    if (pep == NULL) continue ;

	            njobs += 1 ;
	            rs = proclogjob(pip,pep) ;

	            if (rs < 0) break ;
	        } /* end while */

	        cq_curend(&sip->qcom,&cur) ;
	    } /* end if (cursor) */
	} /* end if */

	logfile_setid(&pip->lh,pip->logid) ;

ret0:
	return (rs >= 0) ? njobs : rs ;
}
/* end subroutine (proclogjobs) */


static int proclogjob(pip,pep)
struct proginfo	*pip ;
SVCENTRY	*pep ;
{
	struct subinfo	*sip = pip->sip ;
	time_t		at, et ;
	int		rs ;

	if ((rs = svcentry_arrival(pep,&at)) >= 0) {
	    char	timebuf[TIMEBUFLEN + 1] ;

	    et = (pip->daytime - at) ;
	    logfile_setid(&pip->lh,pep->jobid) ;
	    logfile_printf(&pip->lh,"%s job-out svc=%s (%u)",
		timestr_logz(pip->daytime,timebuf),
		    pep->name,pep->pid) ;
	    logfile_printf(&pip->lh,"job-out elapsed=%s",
		timestr_elapsed(et,timebuf)) ;

	} /* end if */

	return rs ;
}
/* end subroutine (proclogjob) */


static int proclogsecurity(struct proginfo *pip,const char *pfname)
{
	int		al ;
	const char	*ap ;
	if ((al = sfbasename(pfname,-1,&ap)) > 0) {

	    if (pip->open.logprog) {
	        if ((pip->uid != pip->euid) || (pip->gid != pip->egid))
	                logfile_printf(&pip->lh,"security reset uid=%d\n",
	                    pip->uid) ;
	    } /* end if (log entry) */

	    if (pip->debuglevel > 0) {
		char	timebuf[TIMEBUFLEN+1] ;
	        timestr_logz(pip->daytime,timebuf) ;
	        bprintf(pip->efp,"%s: %s server=%t\n",
	            pip->progname,timebuf,ap,al) ;
	    }

	} /* end if (sfbasename) */
	return SR_OK ;
} 
/* end subroutine (proclogsecurity) */


#if	CF_SVCFILEFREE || CF_ACCTABFREE

static int procdbdump(pip)
struct proginfo	*pip ;
{
	struct subinfo	*sip = pip->sip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((pip->daytime - sip->ti_dbdump) > TO_DBDUMP) {

	            sip->ti_dbdump = pip->daytime ;

#if	CF_SVCFILEFREE 
	            if ((rs >= 0) && pip->open.svcfile) {

	                svcfile_close(sip->sfp) ;

	                rs1 = svcfile_open(sip->sfp,pip->svcfname) ;
	                pip->open.svcfile = (rs1 >= 0) ;

	            }
#endif /* CF_SVCFILEFREE */

#if	CF_ACCTABFREE
	            if ((rs >= 0) && pip->open.accfname) {

	                acctab_close(sip->atp) ;

	                rs1 = acctab_open(sip->atp,pip->acctab,NULL) ;
	                pip->open.svcfile = (rs1 >= 0) ;

	            }
#endif /* CF_ACCTABFREE */

	} /* end if */

	return rs ;
}
/* end subroutine (procdbdump) */

#endif /* CF_SVCFILEFREE || CF_ACCTABFREE */


static int procxfile(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) {
#if	CF_SPERM
	        rs = sperm(&pip->id,&sb,X_OK) ;
#else
	        rs = perm(fname,-1,-1,NULL,X_OK) ;
#endif
	    }
	}

	return rs ;
}
/* end subroutine (procxfile) */


#if	CF_ENVLOCAL

static int loadexports(pip,elp)
struct proginfo	*pip ;
vecstr		*elp ;
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; pip->envv[i] != NULL ; i += 1) {
	    rs = vecstr_add(elp,pip->envv[i],-1) ;
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (loadexports) */

#endif /* CF_ENVLOCAL */


static void int_all(int sn)
{
	int		oerrno = errno ;

#if	CF_DEBUGS
	debugprintf("progwatch/int_all: sn=%u\n",sn) ;
#endif

	switch (sn) {
	case SIGINT:
	    if_int = TRUE ;
	    break ;
	default:
	    if_term = TRUE ;
	    break ;
	} /* end switch */

#if	CF_DEBUGS
	debugprintf("progwatch/int_all: ret sn=%u\n",sn) ;
	debugprintf("progwatch/int_all: if_int=%u if_term=%u\n",
		if_int,if_term) ;
#endif

	errno = oerrno ;
}
/* end subroutine (int_all) */


