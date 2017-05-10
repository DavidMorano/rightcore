/* watch */

/* process the service names given us */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_ALWAYSDEF	0		/* always want the default access? */
#define	CF_FASTQ	0		/* fast Q allocation of PROEGENTRYs */
#define	CF_SRVTABCHECK	1		/* check SRVTAB for changes */
#define	CF_ACCTABCHECK	1		/* check ACCTAB for changes */
#define	CF_SRVTABFREE	0		/* free up SRVTAB occassionally? */
#define	CF_ACCTABFREE	0		/* free up ACCTAB occassionally? */
#define	CF_LOGONLY	0		/* log exit only w/ daemon? */
#define	CF_POLL		1		/* use 'poll(2)'? */
#define	CF_LOGFILECHECK	1		/* logfile check */
#define	CF_SPERM	1		/* use 'sperm(3dam)' */


/* revision history:

	= 1991-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/


/*****************************************************************************

        This subroutine is responsible for processing the jobs that have been
        handed to us from the initialization code.

	Returns:

	OK	may not really matter in the current implementation!


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<field.h>
#include	<ids.h>
#include	<exitcodes.h>

#include	"srvtab.h"
#include	"acctab.h"
#include	"progentry.h"
#include	"cq.h"
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* local defines */

#define	W_OPTIONS	(WNOHANG)
#define	MAXOUTLEN	61
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#define	TO_LOCKFILE	0		/* lock timeout */
#define	TO_POLL		2		/* poll interval */
#define	TO_TIMEOUT	45		/* miscellaneous timeout */
#define	TO_MAINT	(3 * 60)	/* miscellaneous maintenance */

#define	POLLSEC		1000

#define	JOBFRACTION	10

#ifndef	XDEBFILE
#define	XDEBFILE	"/var/tmp/pcspoll.deb"
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsdd(char *,int,const char *,uint) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	pcsgetprog(const char *,char *,const char *) ;
extern int	pcsgetprogpath(const char *,char *,const char *) ;
extern int	getpwd(char *,int) ;

extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* externals variables */


/* local structures */

struct procinfo {
	varsub	*ssp ;			/* string substitutions */
	vecstr	*elp ;			/* export list */
	SRVTAB	*sfp ;			/* service file */
	ACCTAB	*atp ;			/* access table file */
	CQ	qfree, qcom ;		/* free & computer Qs */
	vechand	trun ;			/* run table */
	PROGENTRY_ARGS	args ;
	time_t	daytime ;		/* convenient time of day */
	time_t	t_lockcheck ;
	time_t	t_pidcheck ;
	int	serial ;		/* serial number */
	int	to_minjob ;		/* interval */
	uint	f_error : 1 ;		/* early exit on error */
} ;


/* forward references */

static int	process_name() ;
static int	process_service(struct proginfo *,struct procinfo *,
			SRVTAB_ENT *,PROGENTRY_ARGS *) ;
static int	process_progentrynew(struct proginfo *,
			struct procinfo *,PROGENTRY **) ;
static int	process_progentryfree(struct proginfo *, 
			struct procinfo *,PROGENTRY *) ;
static int	process_deljob(struct proginfo *,struct procinfo *,int,
			PROGENTRY *) ;
static int	process_cycle(struct proginfo *,struct procinfo *) ;
static int	process_findpid() ;
static int	process_startjob() ;
static int	process_check(struct proginfo *,struct procinfo *) ;
static int	process_jobactive(struct proginfo *,struct procinfo *,
			char *,PROGENTRY **) ;
static int	process_checkaccess(struct proginfo *,struct procinfo *,
			char *) ;
static int	writeout(struct proginfo *,int,const char *) ;
static int	findprog(struct proginfo *,const char *,char *,int *) ;
static int	xfile(struct proginfo *,const char *) ;

static void	int_all(int) ;

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */

static int	f_exit ;







int watch(pip,snp)
struct proginfo		*pip ;
vecstr			*snp ;		/* initial service names */
{
	struct procinfo	pi ;

	struct sigaction	sigs ;

	struct pollfd	fds[2] ;

	PROGENTRY	*pep ;

	SRVTAB		*sfp = &pip->stab ;

	SRVTAB_ENT	*sep ;

	sigset_t	signalmask ;

	time_t		t_start, t_check ;
	time_t		t_free ;

#ifdef	DMALLOC
	unsigned long	dmallocmark ;
#endif

	int	rs, rs1, rs2, i ;
	int	loopcount = 0 ;
	int	opts ;
	int	to_poll ;
	int	to_jobslice, to_check ;
#if	defined(DMALLOC) || defined(MALLOCLOG)
	int	dmalloc_count = 0 ;
#endif
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("watch: entered \n") ;
#endif


/* initialize our local information */

	(void) memset(&pi,0,sizeof(struct procinfo)) ;

	pi.ssp = &pip->tabsubs ;
	pi.elp = &pip->exports ;
	pi.sfp = &pip->stab ;
	pi.atp = &pip->atab ;

	cq_init(&pi.qfree) ;

	cq_init(&pi.qcom) ;

	opts = VECHAND_OSWAP | VECHAND_OCONSERVE ;
	vechand_init(&pi.trun,pip->maxjobs,opts) ;	/* run table */

	pi.daytime = time(NULL) ;

	t_start = pi.daytime ;
	t_check = pi.daytime ;
	t_free = pi.daytime ;
	pi.serial = 0 ;
	pi.to_minjob = -1 ;


/* screw the stupid signals */

	memset(&sigs,0,sizeof(struct sigaction)) ;

	(void) uc_sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGTERM,&sigs,NULL) ;

	(void) uc_sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	(void) uc_sigemptyset(&signalmask) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGINT,&sigs,NULL) ;


/* make up the structure of the PROGENTRY_ARGS */

	(void) memset(&pi.args,0,sizeof(PROGENTRY_ARGS)) ;

	pi.args.version = VERSION ;
	pi.args.programroot = pip->pr ;
	pi.args.domainname = pip->domainname ;
	pi.args.nodename = pip->nodename ;
	pi.args.username = pip->username ;
	pi.args.groupname = pip->groupname ;
	pi.args.tmpdir = pip->tmpdname ;
	pi.args.hostname = NULL ;
	pi.args.service = NULL ;
	pi.args.interval = NULL ;
	pi.args.daytime = pi.daytime ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    rs1 = vechand_count(&pi.trun) ;
	    debugprintf("watch: trun -1 count=%d\n",rs1) ;
	    rs1 = vechand_extent(&pi.trun) ;
	    debugprintf("watch: trun -1 extent=%d\n",rs1) ;
	}
#endif

/* load up all of the jobs that we may have been given */

	if (pip->f.named) {

	    for (i = 0 ; vecstr_get(snp,i,&cp) >= 0 ; i += 1) {

	        if (cp == NULL) continue ;

	        rs = process_name(pip,&pi,&pi.args,cp) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: process_name() 1 rs=%d\n",rs) ;
#endif

	    } /* end for */

	} else {

	    for (i = 0 ; srvtab_get(sfp,i,&sep) >= 0 ; i += 1) {

	        if (sep == NULL) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: svc=%s int=%s\n",
	                sep->service,
	                sep->interval) ;
#endif

	        if (pip->f.daemon && (sep->interval == NULL))
	            continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            rs1 = vechand_count(&pi.trun) ;
	            debugprintf("watch: trun 0a count=%d\n",rs1) ;
	            rs1 = vechand_extent(&pi.trun) ;
	            debugprintf("watch: trun 0a extent=%d\n",rs1) ;
	        }
#endif

	        rs = process_service(pip,&pi,sep,&pi.args) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("watch: process_service() 1 rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            rs1 = vechand_count(&pi.trun) ;
	            debugprintf("watch: trun 0b count=%d\n",rs1) ;
	            rs1 = vechand_extent(&pi.trun) ;
	            debugprintf("watch: trun 0b extent=%d\n",rs1) ;
	        }
#endif

	    } /* end for */

	} /* end if */

/* top of loop */

#if	defined(DMALLOC) || defined(MALLOCLOG)
	rs1 = vechand_count(&pi.trun) ;
	rs2 = cq_count(&pi.qcom) ;
	if ((rs1 == 0) && (rs2 == 0)) {

#ifdef	DMALLOC
	    dmallocmark = dmalloc_mark() ;
#endif

#ifdef	MALLOCLOG
	    malloclog_mark() ;
#endif

	}
#endif

	loopcount = 0 ;
	f_exit = FALSE ;
	while (! f_exit) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        rs1 = vechand_count(&pi.trun) ;
	        debugprintf("watch: trun 1 count=%d\n",rs1) ;
	        rs1 = vechand_extent(&pi.trun) ;
	        debugprintf("watch: trun 1 extent=%d\n",rs1) ;

	    }
#endif /* CF_DEBUG */

#if	CF_SRVTABFREE || CF_ACCTABFREE
	    if ((loopcount % 300) == 0) {

	        if ((pi.daytime - t_free) > (12 * 3600)) {

	            t_free = pi.daytime ;

#if	CF_SRVTABFREE 
	            if (pip->f.srvtab) {

	                srvtab_close(pi.sfp) ;

	                srvtab_open(pi.sfp,pip->srvtab,NULL) ;

	            }
#endif /* CF_SRVTABFREE */

#if	CF_ACCTABFREE
	            if (pip->f.acctab) {

	                acctab_close(pi.atp) ;

	                acctab_open(pi.atp,pip->acctab,NULL) ;

	            }
#endif /* CF_ACCTABFREE */

	        }
	    }
#endif /* CF_SRVTABFREE || CF_ACCTABFREE */

#if	defined(DMALLOC) || defined(MALLOCLOG)
	    rs1 = vechand_count(&pi.trun) ;
	    rs2 = cq_count(&pi.qcom) ;
	    if ((rs1 == 0) && (rs2 == 0)) {

	        if (dmalloc_count++ > 10) {

	            if (pip->f.srvtab)
	                srvtab_close(pi.sfp) ;

	            if (pip->f.acctab)
	                acctab_close(pi.atp) ;

	            {

#ifdef	DMALLOC
	                dmalloc_log_changed(dmallocmark,1,0,1) ;

	                dmallocmark = dmalloc_mark() ;
#endif

#ifdef	MALLOCLOG
	                malloclog_dump() ;

	                malloclog_mark() ;
#endif

	                dmalloc_count = 0 ;

	            } /* end block */

	            if (pip->f.srvtab)
	                srvtab_open(pi.sfp,pip->srvtab,NULL) ;

	            if (pip->f.acctab)
	                acctab_open(pi.atp,pip->acctab,NULL) ;

	        } /* end if (count reached) */

	    }
#endif /* defined(DMALLOC) || defined(MALLOCLOG) */

	    rs = process_cycle(pip,&pi) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("watch: process_cycle() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

/* check for the case of no current jobs! */

	    if (rs == 0) {

	        if (pip->f.named)
	            break ;

	        if (pip->runint >= 0) {

	            if (pi.daytime > (t_start + pip->runint))
	                break ;

	        }

	        to_poll = MAX((pi.to_minjob / JOBFRACTION),1) ;

		if (to_poll > TO_TIMEOUT)
			to_poll = TO_TIMEOUT ;

	    } else
	        to_poll = TO_POLL ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("watch: to_minjob=%d to_poll=%d\n",
	            pi.to_minjob,to_poll) ;
#endif

#if	CF_POLL
	fds[0].fd = -1 ;
	u_poll(fds,0,(to_poll * POLLSEC)) ;
#else
	    sleep(to_poll) ;
#endif

	    pi.daytime = time(NULL) ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        rs1 = vechand_count(&pi.trun) ;
	        debugprintf("watch: trun 2 count=%d\n",rs1) ;
	        rs1 = vechand_extent(&pi.trun) ;
	        debugprintf("watch: trun 2 extent=%d\n",rs1) ;
	    }
#endif

/* has an interval period expired? */

	    to_jobslice = MAX((pi.to_minjob / JOBFRACTION),1) ;

	    to_check = MIN(pip->checkint,to_jobslice) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("watch: to_check=%d\n",to_check) ;
#endif

	    if (pip->f.daemon && (pi.daytime > (t_check + to_check))) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("watch: checking services\n") ;
#endif

	        rs = process_check(pip,&pi) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("watch: process_check() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	            rs1 = vechand_count(&pi.trun) ;
	            debugprintf("watch: trun 3 count=%d\n",rs1) ;
	            rs1 = vechand_extent(&pi.trun) ;
	            debugprintf("watch: trun 3 extent=%d\n",rs1) ;
	        }
#endif /* CF_DEBUG */

	        t_check = pi.daytime ;

	    } /* end if (checking timestamps) */


/* check up on the log file */

#if	CF_LOGFILECHECK
	    if (pip->f.log)
		logfile_check(&pip->lh,pi.daytime) ;
#endif


#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        rs1 = vechand_count(&pi.trun) ;
	        debugprintf("watch: trun 4 count=%d\n",rs1) ;
	        rs1 = vechand_extent(&pi.trun) ;
	        debugprintf("watch: trun 4 extent=%d\n",rs1) ;
	    }
#endif /* CF_DEBUG */


	    loopcount += 1 ;

	} /* end while (looping) */

#ifdef	DMALLOC
	rs1 = vechand_count(&pi.trun) ;
	rs2 = cq_count(&pi.qcom) ;
	if ((rs1 == 0) && (rs2 == 0))
	    dmalloc_log_changed(dmallocmark,1,0,1) ;
#endif


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("watch: returning! f_exit=%d\n",f_exit) ;
#endif


#if	CF_LOGONLY
	f = pip->f.daemon ;
#else
	f = TRUE ;
#endif

	if (f) {

	    pi.daytime = time(NULL) ;

	    if (pip->f.daemon && f_exit)
	        cp = "%s server exiting (interrupt)\n" ;

	    else if (pi.f_error)
	        cp = "%s exiting (error detected)\n" ;

	    else if (pip->f.daemon)
	        cp = "%s server exiting (period expired)\n" ;

	    else
	        cp = "%s exiting (work completed)\n" ;

	    if (pip->f.log)
	        logfile_printf(&pip->lh,cp,
	            timestr_logz(pi.daytime,timebuf)) ;

	} /* end if (daemon mode exit stuff) */

	rs = SR_OK ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	    rs1 = vechand_count(&pi.trun) ;
	    debugprintf("watch: trun 5 count=%d\n",rs1) ;
	    rs1 = vechand_extent(&pi.trun) ;
	    debugprintf("watch: trun 5 extent=%d\n",rs1) ;
	}
#endif


/* early and regular exits */
badhup:
baderr:

bad4:

bad1:

bad0:

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("watch: cleanup 0\n") ;
#endif

	while (cq_rem(&pi.qfree,(void **) &pep) >= 0) {

	    if (pep != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("watch: cleanup 0a pep=%08lx\n",pep) ;
#endif

	        uc_free(pep) ;

#ifdef	MALLOCLOG
	        malloclog_free(pep,"watch:pep0a") ;
#endif

	    }

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("watch: cleanup 1\n") ;
#endif

	while (cq_rem(&pi.qcom,(void **) &pep) >= 0) {

	    if (pep != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("watch: cleanup 1a pep=%08lx\n",pep) ;
#endif

	        progentry_free(pep) ;

	        uc_free(pep) ;

#ifdef	MALLOCLOG
	        malloclog_free(pep,"watch:pep1a") ;
#endif

	    }

	} /* end while */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("watch: cleanup 2\n") ;
#endif

	for (i = 0 ; (rs1 = vechand_get(&pi.trun,i,&pep)) >= 0 ; i += 1) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("watch: cleanup 2a i=%d\n",i) ;
#endif

	    if (pep != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("watch: cleanup 2b pep=%08lx\n",pep) ;
#endif

	        progentry_free(pep) ;

	        uc_free(pep) ;

#ifdef	MALLOCLOG
	        malloclog_free(pep,"watch:pep2b") ;
#endif

	    }

	} /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("watch: cleanup 3\n") ;
#endif

	cq_free(&pi.qfree) ;

	cq_free(&pi.qcom) ;

	vechand_free(&pi.trun) ;	/* run table */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("watch: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (watch) */



/* LOCAL SUBROUTINES */



static int process_name(pip,pcp,pap,name)
struct proginfo	*pip ;
struct procinfo	*pcp ;
PROGENTRY_ARGS	*pap ;
char		name[] ;
{
	SRVTAB_ENT	*sep ;

	int		rs ;


	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

	rs = srvtab_find(pcp->sfp,name,&sep) ;

	if (rs < 0)
	    return rs ;

/* does this entry have an interval? */

	rs = process_service(pip,pcp,sep,pap) ;

	return rs ;
}
/* end subroutine (process_name) */


/* get a new PROGENTRY object */
static process_progentrynew(pip,pcp,pepp)
struct proginfo	*pip ;
struct procinfo	*pcp ;
PROGENTRY	**pepp ;
{
	int	rs = SR_EMPTY ;


#if	CF_FASTQ

	rs = cq_rem(&pcp->qfree,(void **) pepp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_progentrynew: cq_rem() rs=%d pep=%08lx\n",
	        rs,*pepp) ;
#endif

#endif /* CF_FASTQ */

	if (rs == SR_EMPTY) {

	    rs = uc_malloc(sizeof(PROGENTRY),pepp) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("process_progentrynew: malloc rs=%d pep=%08lx\n",
	            rs,*pepp) ;
#endif

#ifdef	MALLOCLOG
	    malloclog_alloc(*pepp,sizeof(PROGENTRY),
	        "process_progentrynew:pep") ;
#endif

	} /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	    debugprintf("process_progentrynew: ret rs=%d pep=%08lx\n",
	        rs,*pepp) ;
#endif

	return rs ;
}
/* end subroutine (process_progentrynew) */


/* free up or re-Q up (whatever) an old PROGENTRY object */
static process_progentryfree(pip,pcp,pep)
struct proginfo	*pip ;
struct procinfo	*pcp ;
PROGENTRY	*pep ;
{
	int	rs = SR_EMPTY ;


	if (pep == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process_progentryfree: pep=%08lx\n", pep) ;
#endif

#if	CF_FASTQ
	rs = cq_ins(&pcp->qfree,(void *) pep) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_progentryfree: cq_ins() rs=%d pep=%08lx\n",
	        rs,pep) ;
#endif

#endif /* CF_FASTQ */

	if (rs < 0) {

	    rs = SR_OK ;
	    uc_free(pep) ;

#ifdef	MALLOCLOG
	    malloclog_free(pep,"process_progentryfree") ;
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process_progentryfree: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process_progentryfree) */


/* do a cycle of work */
static int process_cycle(pip,pcp)
struct proginfo	*pip ;
struct procinfo	*pcp ;
{
	PROGENTRY	*pep ;

	int	rs = SR_OK, rs1 ;
	int	ncom, nrun ;
	int	child_stat ;
	int	efd, ofd ;

	char	timebuf[TIMEBUFLEN + 1] ;


#ifdef	DEBFILE
	{
	    char	*p = getenv("PATH") ;
	    if (! d_ispath(p))
	        nprintf(DEBFILE,"%-15s process_cycle: not path\n",
	            pip->logid) ;
	}
#endif /* DEBFILE */

/* do we have to start some new jobs? */

	ncom = cq_count(&pcp->qcom) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_cycle: cq_count() ncom=%d\n",ncom) ;
#endif

	nrun = vechand_count(&pcp->trun) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_cycle: vechand_count() nrun=%d\n",nrun) ;
#endif

	if ((nrun >= 0) && (ncom >= 0)) {

	    if ((ncom > 0) && (nrun < pip->maxjobs)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("process_cycle: ncom=%d\n",ncom) ;
#endif

	        rs1 = cq_rem(&pcp->qcom,(void **) &pep) ;

	        if (rs1 >= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 2) {

	                int	i ;
	                char	*cp ;

	                debugprintf("process_cycle: dequeued rs=%d pep=%08lx\n",
	                    rs1,pep) ;
	                debugprintf("process_cycle: dequeued rs=%d svc=%s p=%s\n",
	                    rs1,pep->name,pep->program) ;
	                debugprintf("process_cycle: dumping exports\n") ;
	                for (i = 0 ; vecstr_get(pcp->elp,i,&cp) >= 0 ; i += 1) {
	                    if (cp == NULL) continue ;
	                    debugprintf("process_cycle: CP=%p\n",cp) ;
	                    debugprintf("process_cycle: export> %w\n",
					cp,strnlen(cp,50)) ;
	                }
	            }
#endif /* CF_DEBUG */

	            ncom -= 1 ;
	            rs1 = process_startjob(pip,pcp,pep) ;

	            if (rs1 < 0) {

	                if (pip->f.log)
	                    logfile_setid(&pip->lh,pep->jobid) ;

	                if (pip->f.log)
	                    logfile_printf(&pip->lh,
	                        "could not start job=%s (%d)\n",
	                        pep->name,rs1) ;

	                if (pip->f.log)
	                    logfile_setid(&pip->lh,pip->logid) ;

	                progentry_free(pep) ;

	                process_progentryfree(pip,pcp,pep) ;

	            } else
	                nrun += 1 ;

	        }

	    } /* end if (starting new jobs) */

	} /* end if */


/* are there any completed jobs yet? */

	if ((rs1 = u_waitpid(-1,&child_stat,W_OPTIONS)) > 0) {

	    int	ji, pid ;


	    pid = rs1 ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process_cycle: child exit, pid=%d stat=%d\n",
	            pid,(child_stat & 0xFF)) ;
#endif

	    if ((ji = process_findpid(pip,pcp,(pid_t) pid,&pep)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_cycle: found child, ji=%d pid=%d\n",
	                ji,pep->pid) ;
#endif

	        if (pip->f.log)
	            logfile_setid(&pip->lh,pep->jobid) ;

	        if (pip->f.log)
	            logfile_printf(&pip->lh, "%s server exit ex=%u\n",
	                timestr_logz(pcp->daytime,timebuf),
	                (child_stat & 255)) ;

/* process this guy's termination */

#if	CF_DEBUGS
	        rs = u_stat(pep->efname,&sb) ;
	        debugprintf("process_cycle: file=%s rs=%d perm=%04o\n",
	            pep->efname,rs,sb.st_mode) ;
#endif

	        if ((efd = u_open(pep->efname,O_RDONLY,0666)) >= 0) {

	            if ((ofd = u_open(pep->ofname,O_RDONLY,0666)) >= 0) {

	                writeout(pip,ofd,"standard output>") ;

	                u_close(ofd) ;

	            }

	            writeout(pip,efd,"standard error>") ;

	            if (pip->f.log)
	                logfile_printf(&pip->lh,"elapsed time %s\n",
	                    timestr_elapsed((pcp->daytime - pep->atime),
	                    timebuf)) ;

	            u_close(efd) ;

	        } else {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process_cycle: child did not 'exec'\n") ;
#endif

	            if (pip->f.log)
	                logfile_printf(&pip->lh,"server did not 'exec'\n") ;

	        }

	        (void) process_deljob(pip,pcp,ji,pep) ;

	        if (pip->f.log)
	            logfile_setid(&pip->lh,pip->logid) ;

	    } else {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	            debugprintf("process_cycle: unknown pid=%d\n",rs1) ;
#endif

	        if (pip->f.log)
	            logfile_printf(&pip->lh,"unknown PID=%d\n",rs1) ;

	    }

	} /* end if (a child process exited) */

/* maintenance the PID mutex lock file */

	if (pip->open.pidlock &&
	    ((pcp->daytime - pcp->t_pidcheck) > TO_MAINT)) {

	    LFM_CHECK	lc ;


	    rs = lfm_check(&pip->pidlock,&lc,pcp->daytime) ;

	    if (rs < 0) {

	            if (pip->f.log)
	                logfile_printf(&pip->lh,
	                    "%s lost PIDLOCK other PID=%d\n",
	                    timestr_logz(pcp->daytime,timebuf),
	                    lc.pid) ;

	        rs = SR_ALREADY ;
		pcp->f_error = TRUE ;
	        goto ret0 ;
	    }

	    pcp->t_pidcheck = pcp->daytime ;

	} /* end if (maintaining the PID mutex file) */


/* check if the server table file (srvtab) has changed */

#if	CF_SRVTABCHECK
	if (! pip->f.named) {

	    if ((rs = srvtab_check(pcp->sfp,pcp->daytime,NULL)) > 0) {

	        if (pip->f.log)
	            logfile_printf(&pip->lh,
			"%s server table file changed\n",
	                timestr_logz(pcp->daytime,timebuf)) ;

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process_cycle: srvtab_check() rs=%d\n",rs) ;
#endif

	}
#endif /* CF_SRVTABCHECK */


#if	CF_ACCTABCHECK
	if (! pip->f.named) {

	    if ((rs = acctab_check(pcp->atp,NULL)) > 0) {

	        if (pip->f.log)
	            logfile_printf(&pip->lh,"%s access table file changed\n",
	                timestr_logz(pcp->daytime,timebuf)) ;

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process_cycle: acctab_check() rs=%d\n",rs) ;
#endif

	}
#endif /* CF_ACCTABCHECK */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_cycle: ret OK rs=%d\n",
	        (ncom + nrun)) ;
#endif

ret0:
	return (rs >= 0) ? (ncom + nrun) : rs ;
}
/* end subroutine (process_cycle) */


/* spawn a job */
static int process_startjob(pip,pcp,pep)
struct proginfo	*pip ;
struct procinfo	*pcp ;
PROGENTRY	*pep ;
{
	pid_t	pid ;

	int	rs, ji ;
	int	f_secure ;

	char	programpath[MAXPATHLEN + 2] ;
	char	*program ;


#ifdef	DEBFILE
	{
	    char	*p = getenv("PATH") ;
	    if (! d_ispath(p))
	        nprintf(DEBFILE,"%-15s process_startjob: not path\n",
	            pip->logid) ;
	}
#endif /* DEBFILE */

	if (pip->f.log)
	    logfile_setid(&pip->lh,pep->jobid) ;


/* can we execute this service daemon? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_startjob: X-check program=>%s<\n",
	        pep->program) ;
#endif

	programpath[0] = '\0' ;
#ifdef	COMMENT
	rs = pcsgetprogpath(pip->pr,programpath,pep->program) ;
#else
	program = pep->program ;
	rs = findprog(pip,pep->program,programpath,&f_secure) ;
#endif /* COMMENT */

	if (rs > 0)
		program = programpath ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_startjob: get-find rs=%d program=>%s<\n",
	        rs,program) ;
#endif /* CF_DEBUG */

	if (rs < 0) {

#ifdef	DEBFILE
	    {
	        char	hexbuf[100 + 1] ;
	        char	*cp ;
	        char	*p = getenv("PATH") ;


	        nprintf(DEBFILE,"%-15s pcsgetprogpath() pr=%s\n",
	            pip->logid,pip->pr) ;
	        nprintf(DEBFILE,"%-15s program=%s\n",
	            pip->logid,program) ;

	        if (p != NULL) {

	            nprintf(DEBFILE,"%-15s PATH=%p\n",
	                pip->logid,p) ;
	            nprintf(DEBFILE,"%-15s PATH=>%w<\n",
	                pip->logid,p,strnlen(p,30)) ;

#ifdef	COMMENT
	            mkhexstr(hexbuf,100,p,16) ;
	            nprintf(DEBFILE,"%-15s PATH= %s\n",
	                pip->logid,hexbuf) ;
#endif

	            if ((*p == '/') || (*p == ':')) {

	                while ((cp = strchr(p,':')) != NULL) {

	                    nprintf(DEBFILE,"%-15s pathdir> %w\n",
	                        pip->logid,p,(cp - p)) ;

	                    p = cp + 1 ;
	                }

	                if (*p)
	                    nprintf(DEBFILE,"%-15s d> %s\n",
	                        pip->logid,p) ;

	            }

	        } else
	            nprintf(DEBFILE,"%-15s PATH is not set!\n",
	                pip->logid) ;

	        nprintf(DEBFILE,"%-15s rs=%d pp=%s\n",
	            pip->logid,rs,program) ;

	    }
#endif /* DEBFILE */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("process_startjob: could not execute\n") ;
#endif

	    if (pip->f.log) {

	        logfile_printf(&pip->lh,"cannot execute server program\n") ;

	        logfile_printf(&pip->lh,"program=%s\n",pep->program) ;

	    }

	    rs = SR_NOEXIST ;
	    goto bad0 ;

	} /* end if */

#ifdef	COMMENT
	if (rs > 0) {

	    if ((pep->program[0] != '/') && (programpath[0] != '/'))
	        mkpath2(programpath, pip->pwd,pep->program) ;

	    program = programpath ;

	}
#endif /* COMMENT */

/* start 'er up */

	rs = vechand_add(&pcp->trun,pep) ;

	if (rs < 0)
	    goto bad0 ;

	ji = rs ;

/* let's fork the processing subroutine and get on with our lives! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_startjob: about to fork\n") ;
#endif

	logfile_flush(&pip->lh) ;

	bflush(pip->efp) ;

	rs = u_fork() ;

	if (rs < 0) {

	    if (pip->f.log)
	        logfile_printf(&pip->lh,
	            "we couldn't do a fork (%d)\n",
	            rs) ;

	    goto bad1 ;
	}

	pid = rs ;
	if (pid == 0) {

/* we are now the CHILD!! */

	    u_close(0) ;

	    u_close(1) ;

	    u_close(2) ;

	    u_open("/dev/null",O_RDONLY,0666) ;

	    u_open(pep->ofname,O_WRONLY,0666) ;

	    u_open(pep->efname,O_WRONLY,0666) ;

	    if ((! f_secure) && (pip->uid != pip->euid))
		u_seteuid(pip->uid) ;

/* do it */

	    rs = execute(pip,program,pep->srvargs,pcp->elp) ;


	    if (rs < 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("process_startjob: could not exec prog=%s rs=%d\n",
	                program,rs) ;
#endif

	        (void) u_unlink(pep->efname) ;

	    }

	    u_exit(EX_NOEXEC) ;

	} /* end if (child process) */

	pep->pid = pid ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_startjob: server pid=%d\n",
	        pid) ;
#endif

	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    char	*arg0 = strbasename(program) ;


	    if (pip->f.log) {

	        logfile_printf(&pip->lh,"%s server=%s\n",
	            timestr_logz(pcp->daytime,timebuf),
	            arg0) ;

		if (! f_secure)
	    	logfile_printf(&pip->lh,"security reset uid=%d\n",
			pip->uid) ;

	    } /* end if (log entry) */

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s server=%s\n",
	            pip->progname,
	            timestr_logz(pcp->daytime,timebuf),
	            arg0) ;

	} /* end block */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_startjob: forked pid=%d\n",pep->pid) ;
#endif

	if (pip->f.log)
	    logfile_setid(&pip->lh,pip->logid) ;

	return rs ;

/* bad things come here */
bad1:
	(void) vechand_del(&pcp->trun,ji) ;

bad0:
	if (pip->f.log)
	    logfile_setid(&pip->lh,pip->logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_startjob: ret bad rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process_startjob) */


/* find a job by its PID */
static int process_findpid(pip,pcp,pid,pepp)
struct proginfo	*pip ;
struct procinfo	*pcp ;
pid_t		pid ;
PROGENTRY	**pepp ;
{
	PROGENTRY	*pep ;

	int	rs, i ;


	if (pepp == NULL)
	    pepp = &pep ;

	for (i = 0 ; (rs = vechand_get(&pcp->trun,i,pepp)) >= 0 ; i += 1) {

	    if (*pepp == NULL) continue ;

	    if (pid == (*pepp)->pid)
	        break ;

	} /* end for */

	return ((rs >= 0) ? i : rs) ;
}
/* end subroutine (process_findpid) */


/* delete a job and free up the progentry */
static int process_deljob(pip,pcp,ji,pep)
struct proginfo	*pip ;
struct procinfo	*pcp ;
int		ji ;
PROGENTRY	*pep ;
{

	int	rs ;


	progentry_free(pep) ;

	rs = vechand_del(&pcp->trun,ji) ;

	process_progentryfree(pip,pcp,pep) ;

	return rs ;
}
/* end subroutine (process_deljob) */


/* check on program intervals when in daemon mode */
static int process_check(pip,pcp)
struct proginfo	*pip ;
struct procinfo	*pcp ;
{
	SRVTAB		*sfp ;

	PROGENTRY	*pep ;

	SRVTAB_ENT	*sep ;

	int	rs, i ;
	int	n = 0 ;


	pcp->to_minjob = -1 ;

/* update the time-of-day for newly created PROGENTRY objects */

	pcp->args.daytime = pcp->daytime ;

/* loop finding suitable jobs to restart */

	sfp = pcp->sfp ;
	for (i = 0 ; srvtab_get(sfp,i,&sep) >= 0 ; i += 1) {

	    if (sep == NULL) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process_check: svc=%s int=%s\n",
	            sep->service,
	            sep->interval) ;
#endif

	    if (sep->interval == NULL)
	        continue ;

/* is this service already active? (if not, check it) */

	    if (process_jobactive(pip,pcp,sep->service,&pep) < 0) {

	        rs = process_service(pip,pcp,sep,&pcp->args) ;

	        if (rs > 0)
	            n += 1 ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process_check: process_service() 1 rs=%d\n",rs) ;
#endif

	    } else {

	        int	interval ;


	        if (progentry_getinterval(pep,&interval) >= 0) {

	            if ((interval < pcp->to_minjob) || (pcp->to_minjob < 0))
	                pcp->to_minjob = interval ;

	        }

	    } /* end if */

	} /* end for (looping through all services) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (process_check) */


/* check the interval of a service entry */
static int process_service(pip,pcp,sep,pap)
struct proginfo	*pip ;
struct procinfo	*pcp ;
SRVTAB_ENT	*sep ;
PROGENTRY_ARGS	*pap ;
{
	bfile		tsfile ;

	PROGENTRY	*pep ;

	struct ustat	sb ;

	mode_t	oldmask ;

	int	rs ;
	int	interval ;
	int	f_process ;

	char	stampfname[MAXPATHLEN + 1] ;
	char	logid[PROGENTRY_IDLEN + 2] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*access ;


	if ((pcp == NULL) || (sep == NULL))
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_service: entered service=%s\n",
	        sep->service) ;
#endif

#ifdef	DEBFILE
	{
	    char	*p = getenv("PATH") ;
	    if (! d_ispath(p))
	        nprintf(DEBFILE,"%-15s process_service: not path\n",
	            pip->logid) ;
	}
#endif /* DEBFILE */

/* allocate a new PROGENTRY */

	rs = process_progentrynew(pip,pcp,&pep) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_service: newprogentry() rs=%d pep=%08lx\n",
	        rs,pep) ;
#endif

	if (rs < 0)
	    goto bad0 ;

/* initialize this PROGENTRY object */

	rs = progentry_init(pep,pcp->ssp,sep,pap) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_service: progentry_init() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* see if this service is allowed on the host we are running on! */

	rs = progentry_getaccess(pep,&access) ;

	if ((rs < 0) && (rs != SR_EMPTY))
	    goto bad2 ;

/* check no matter what since there may be a default access restriction! */

	rs = process_checkaccess(pip,pcp,access) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process_service: process_checkaccess() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;


/* see if the interval has expired */

	rs = progentry_getinterval(pep,&interval) ;

	if (rs < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("process_service: got a bad interval\n") ;
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

	mkpath2(stampfname, pip->stampdname,sep->service) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("process_service: stampfname=%s\n",
	        stampfname) ;
	    debugprintf("process_service: interval=%d\n",
	        interval) ;
	}
#endif /* CF_DEBUG */

	oldmask = umask(0000) ;

	rs = bopen(&tsfile,stampfname,"wc",0666) ;

	umask(oldmask) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_service: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_service: created=%s\n",
	        ((rs == SR_CREATED) ? "yes" : "no")) ;
#endif

	f_process = FALSE ;
	if (rs != SR_CREATED) {

	    rs = bcontrol(&tsfile,BC_LOCK,TO_LOCKFILE) ;

	    if (rs >= 0) {

	        rs = bcontrol(&tsfile,BC_STAT,&sb) ;

	        if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	                debugprintf("process_service: daytime=%s\n",
	                    timestr_logz(pcp->daytime,timebuf)) ;
	                debugprintf("process_service: mtime=%s\n",
	                    timestr_logz(sb.st_mtime,timebuf)) ;
	            }
#endif /* CF_DEBUG */

	            if ((pcp->daytime - sb.st_mtime) > interval)
	                f_process = TRUE ;

	        }

	    } else
	        f_process = FALSE ;

	} else {

	    bcontrol(&tsfile,BC_CHMOD,0666) ;

	    f_process = TRUE ;

	}

	if (rs < 0)
	    goto bad3 ;

/* at this point, if 'f_process' is TRUE, that means to run that job! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_service: rs=%d should we process ans=%d\n",
	        rs,((rs >= 0) && f_process)) ;
#endif

	if ((rs >= 0) && f_process) {

/* create a job ID for this job */

	    pap->logid = logid ;
	    snsdd(logid,PROGENTRY_IDLEN,pip->logid, pcp->serial) ;

	    pcp->serial += 1 ;
	    if (pip->f.log)
	        logfile_setid(&pip->lh,logid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("process_service: %s service=%s\n",
	            timestr_logz(pcp->daytime,timebuf),
	            pep->name) ;
#endif

	    if (pip->f.log)
	        logfile_printf(&pip->lh,"%s service=%s\n",
	            timestr_logz(pcp->daytime,timebuf),
	            pep->name) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: %s service=%s\n",
	            pip->progname,
	            timestr_logz(pcp->daytime,timebuf),
	            pep->name) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("process_service: progentry_expand()\n") ;
#endif

	    rs = progentry_expand(pep,sep,pap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	        debugprintf("process_service: progentry_expand() rs=%d\n", rs) ;
	        debugprintf("process_service: svc=%s p=>%s<\n",
	            pep->name,pep->program) ;
	    }
#endif

	    if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("process_service: cq_ins() pep=%08lx\n",pep) ;
#endif

	        rs = cq_ins(&pcp->qcom,pep) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("process_service: cq_ins() rs=%d\n", rs) ;
#endif

	        if (rs < 0)
	            goto bad3 ;

/* update the time stamp by writing to it */

	        bprintf(&tsfile,"%s\n",
	            timestr_logz(pcp->daytime,timebuf)) ;

	    } else {

	        if (pip->f.log)
	            logfile_printf(&pip->lh,
	                "could not expand service entry (%d)\n",
	                rs) ;

/* more stuff happens on a failure here below! */

	    }

	    if (pip->f.log)
	        logfile_setid(&pip->lh,pip->logid) ;

	} /* end if (processing this service) */

	bclose(&tsfile) ;

/* OK, let's get out but first make either log entry or clean up */

	if ((rs < 0) || (! f_process)) {

/* we don't need to run this one */

	    progentry_free(pep) ;

	    process_progentryfree(pip,pcp,pep) ;

	} /* end if (more failure stuff) */

	return (rs >= 0) ? f_process : rs ;

/* bad things */
bad3:
	bclose(&tsfile) ;

bad2:
ret1:
	progentry_free(pep) ;

bad1:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process_service: bad1\n") ;
#endif

	rs = process_progentryfree(pip,pcp,pep) ;

bad0:
	return rs ;
}
/* end subroutine (process_service) */


/* is a named job active in the system already? */
static int process_jobactive(pip,pcp,name,pepp)
struct proginfo	*pip ;
struct procinfo	*pcp ;
char		name[] ;
PROGENTRY	**pepp ;
{
	CQ_CURSOR	cur ;

	PROGENTRY	*pep ;

	int	rs, i ;


	for (i = 0 ; (rs = vechand_get(&pcp->trun,i,&pep)) >= 0 ; i += 1) {

	    if (pep == NULL) continue ;

	    if (strcmp(pep->name,name) == 0)
	        break ;

	} /* end for */

	if (rs < 0) {

	    cq_curbegin(&pcp->qcom,&cur) ;

	    while ((rs = cq_enum(&pcp->qcom,&cur,(void **) &pep)) >= 0) {

	        if (strcmp(pep->name,name) == 0)
	            break ;

	    } /* end while */

	    cq_curend(&pcp->qcom,&cur) ;

	} /* end if */

	if (pepp != NULL)
	    *pepp = (rs >= 0) ? pep : NULL ;

	return rs ;
}
/* end subroutine (process_jobactive) */


/**** PROGRAM NOTE :

	At this point, we have a processed (variable substituted, et
	cetera) server entry in the PROGENTRY object.  From here
	on, we want to check the server access list to see if this
	current execution should be allowed.

****/


/* check if this connection is allowed based on the service access */
static int process_checkaccess(pip,pcp,access)
struct proginfo	*pip ;
struct procinfo	*pcp ;
char		access[] ;
{
	int	rs, i, j ;
	int	sl, cl ;

	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process_checkaccess: entered, access=>%s<\n",
	        access) ;
#endif

	rs = SR_OK ;
	if ((access != NULL) || (pip->defacc != NULL)) {

	    FIELD	af ;

	    vecstr	netgroups, names ;

	    int		fl ;

	    char	hostname[MAXHOSTNAMELEN + 2] ;
	    char	*fp ;


	    vecstr_start(&netgroups,4,0) ;

	    vecstr_start(&names,4,0) ;

	    cp = (access != NULL) ? access : pip->defacc ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process_checkaccess: netgroups >%s<\n",cp) ;
#endif

/* process the server access list */

	    if ((rs = field_init(&af,cp,-1)) >= 0) {

	    while ((fl = field_get(&af,NULL,&fp)) >= 0) {

	        if (fl > 0) {

	            int		bl = fl ;

	            char	*bp = fp ;


	            while ((cl = nextfield(bp,bl,&cp)) > 0) {

	                vecstr_add(&netgroups,cp,cl) ;

	                bl -= ((cp + cl) - bp) ;
	                bp = (cp + cl) ;

	            } /* end while */

	        } /* end if */

	    } /* end while */

	    field_free(&af) ;

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
	        debugprintf("process_checkaccess: netgroups:\n") ;
	        for (i = 0 ; vecstr_get(&netgroups,i,&cp) >= 0 ; i+= 1)
	            debugprintf("process_checkaccess: ng=%s\n",cp) ;
	    }
#endif

/* our own machine names */

	    rs = vecstr_add(&names,pip->nodename,-1) ;

	    if (pip->domainname != NULL) {

		rs = snsds(hostname,MAXHOSTNAMELEN,
	            pip->nodename,pip->domainname) ;

	        if (rs > 0)
	            rs = vecstr_add(&names,hostname,rs) ;

	    } /* end if (creating our hostname) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("process_checkaccess: hostnames rs=%d\n",rs) ;
	        for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1)
	            debugprintf("process_checkaccess: mname=%s\n",cp) ;
	    }
#endif


/* try our own netgroups */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process_checkaccess: checking access\n") ;
#endif

	    if (pip->f.acctab) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("process_checkaccess: checking ACCTAB\n") ;
#endif

	        rs = acctab_anyallowed(pcp->atp,&netgroups,&names,
	            pip->username,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("process_checkaccess: acctab rs=%d\n",
	                rs) ;
#endif

	    } /* end if (ACCTAB check) */


/* try the system netgroups (UNIX does not have one simple call as above!) */

	    if ((! pip->f.acctab) || (rs < 0)) {

	        char	*ngp, *mnp ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("process_checkaccess: sys NETGROUP DB\n") ;
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
	                        "process_checkaccess: sysnetgr n=%s m=%s\n",
	                        ngp,mnp) ;
#endif

	                if (innetgr(ngp,mnp,NULL,pip->domainname))
	                    break ;

	            } /* end for (machine names) */

	            if (rs >= 0)
	                break ;

	        } /* end for (netgroups) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("process_checkaccess: system rs=%d\n",
	                rs) ;
#endif

	    } /* end if (checking UNIX netgroups) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process_checkaccess: anyallowed rs=%d\n",rs) ;
#endif

	    vecstr_finish(&netgroups) ;

	    vecstr_finish(&names) ;

	} /* end if (checking client for access to this server) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("process_checkaccess: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process_checkaccess) */


/* write out the output files from the executed program */
static int writeout(pip,fd,s)
struct proginfo	*pip ;
int		fd ;
const char	s[] ;
{
	struct ustat	sb ;

	bfile		file, *fp = &file ;

	int		tlen, len ;

	char		linebuf[MAXOUTLEN + 2] ;


	tlen = 0 ;
	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    if (pip->f.log)
	        logfile_printf(&pip->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,MAXOUTLEN)) > 0) {

	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	                debugprintf("writeout: wo> %w\n",linebuf,len) ;
#endif

	            if (pip->f.log)
	                logfile_printf(&pip->lh,"| %w\n",
	                    linebuf,len) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */


/* find a program server and evaluate its security */
static int findprog(pip,program,progpath,sp)
struct proginfo	*pip ;
const char	program[] ;
char		progpath[] ;
int		*sp ;			/* secure path? */
{
	int	rs, sl ;
	int	i ;

	char	*cp ;


	if (program[0] == '/') {

	    sl = 0 ;
	    *sp = pip->f.secure_srvtab ;
	    rs = xfile(pip,program) ;

	} else {

	    vecstr	already ;


	    vecstr_start(&already,3,0) ;

/* search the PCS area(s) first */

	    rs = mkpath3(progpath,pip->pr,"bin",program) ;

	    sl = rs ;
	    if (rs >= 0)
	        vecstr_add(&already,progpath,sl) ;

	    *sp = pip->f.secure_root && pip->f.secure_srvtab ;
	    if (rs >= 0)
	    	rs = xfile(pip,progpath) ;

/* PCS 'sbin' */

	    if (rs < 0) {

	    	rs = mkpath3(progpath,pip->pr,"sbin",program) ;

		sl = rs ;
	        if (rs >= 0)
	            vecstr_add(&already,progpath,sl) ;

	    	*sp = pip->f.secure_root && pip->f.secure_srvtab ;
	    	if (rs >= 0)
	    	    rs = xfile(pip,progpath) ;

	    } /* end if (PCS sbin) */

/* do we need to search the given search path? */

	    if (rs < 0) {

	        *sp = pip->f.secure_path ;
	        for (i = 0 ; vecstr_get(&pip->path,i,&cp) >= 0 ; i += 1) {

	            char	*pp ;


		    if (cp == NULL) continue ;

	            rs = sl = 0 ;
	            pp = (char *) program ;
	            if (cp[0] != '\0') {

	                pp = (char *) progpath ;
	                rs = sl = mkpath2(progpath,cp,program) ;

	            }

		    if ((cp[0] == '\0') || (vecstr_find(&already,pp) < 0)) {

		        if (rs >= 0)
	                    rs = xfile(pip,pp) ;

	                if (rs >= 0)
	                    break ;

		    }

	        } /* end for (extra search paths) */

	    } /* end if (checking extra search path) */

/* do we need to search regular directories? */

	    cp = "/usr/bin" ;
	    if ((rs < 0) && (vecstr_find(&pip->path,cp) < 0)) {

	        rs = mkpath2(progpath,cp,program) ;

		sl = rs ;
	        *sp = pip->f.secure_srvtab ;
		if (rs >= 0)
	            rs = xfile(pip,progpath) ;

	    } /* end if (checking regular area) */

	    cp = "/usr/sbin" ;
	    if ((rs < 0) && (vecstr_find(&pip->path,cp) < 0)) {

	        rs = mkpath2(progpath,cp,program) ;

		sl = rs ;
	        *sp = pip->f.secure_srvtab ;
		if (rs >= 0)
	            rs = xfile(pip,progpath) ;

	    } /* end if (checking regular area) */

	    vecstr_finish(&already) ;

	} /* end if */

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (findprog) */


static int xfile(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;


	rs = u_stat(fname,&sb) ;

	if (rs >= 0) {

	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) {

#if	CF_SPERM
		rs = sperm(&pip->ids,&sb,X_OK) ;
#else
	        rs = perm(fname,-1,-1,NULL,X_OK) ;
#endif

	    }
	}

	return rs ;
}
/* end subroutine (xfile) */


static void int_all(sn)
int	sn ;
{


	f_exit = TRUE ;
}
/* end subroutine (int_all) */



