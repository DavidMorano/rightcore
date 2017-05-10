/* batchd */

/* batch job daemon program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SERVERERR	0		/* some extra debugging of some sort */
#define	CF_UGETLOADAVG	1		/* use |getloadavg(3c)| */


/*
 *  Batch daemon
 */

#ifdef RISCos
/* using -D SYSTYPE_BSD43 makes mips look like SUN, but need to special
 * case out the load calculations.  
*/
#define sun
#endif


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/resource.h>
#if defined(sequent)
# include <i386/vmparam.h>
#endif
#include <sys/stat.h>
#include <nlist.h>
#include <fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include	<search.h>
#include	<malloc.h>
#include <stdio.h>
#include <errno.h>

#ifdef BSD4_2
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#else /*BSD4_2*/
#include <sys/times.h>
#include <ustat.h>
#include <dirent.h>
#endif /*BSD4_2*/

#ifdef __hpux
#include <sys/pstat.h>		/* HP-UX process status stuff */
#include "hp.h"
#endif

#ifdef __hpux
#include <sys/times.h>
#include <ustat.h>
#endif /*__hpux*/

#ifdef SUNOS5
#include <sys/utsname.h>
#include <sys/times.h>

#ifdef	SUNOS5KVM
#include <kvm.h>
#endif

#else
# define strsignal(s) sys_siglist[s]
#endif

#if defined(__osf__)
#include <sys/mount.h>
#include <sys/table.h>
#endif

#include	<vsystem.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<getxusername.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"

#include "bat_system.h"		/* sets BSD4_2 and JOBCTL flags */
#include "lex.h"
#include "bat_common.h"	



/* local defines */

#ifndef	USERNAMELEN
#ifdef	LOGNAMELEN
#define	USERNAMELEN	LOGNAMELEN
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	LOGMSGLEN
#define	LOGMSGLEN	1024
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	DEBUGFNAME	"jobsubs.deb"


/*
 * Handy macros to simplify varargs error messages.
 * Assumes a global char errstr[STRSIZ];
 * Not re-entrant!
 */
#define merror1(fmt,a1)		\
	{ sprintf(errstr,fmt,a1); merror(errstr); }
#define merror2(fmt,a1,a2)	\
	{ sprintf(errstr,fmt,a1,a2); merror(errstr); }
#define merror3(fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); merror(errstr); }
#define merror4(fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); merror(errstr); }
#define mperror1(fmt,a1)	\
	{ sprintf(errstr,fmt,a1); mperror(errstr); }
#define mperror2(fmt,a1,a2)	\
	{ sprintf(errstr,fmt,a1,a2); mperror(errstr); }
#define mperror3(fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); mperror(errstr); }
#define muser1(who,fmt,a1)	\
	{ sprintf(errstr,fmt,a1); muser(who, errstr);}
#define muser2(who,fmt,a1,a2) \
	{ sprintf(errstr,fmt,a1,a2); muser(who, errstr); }
#define muser3(who,fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); muser(who, errstr); }
#define muser4(who,fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); muser(who, errstr); }
#define muser5(who,fmt,a1,a2,a3,a4,a5) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5); muser(who, errstr); }
#define muser6(who,fmt,a1,a2,a3,a4,a5,a6) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5,a6); muser(who, errstr); }
#define muser7(who,fmt,a1,a2,a3,a4,a5,a6,a7) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4,a5,a6,a7); muser(who, errstr); }
#define queuestat1(who,fmt,a1) \
	{ sprintf(errstr,fmt,a1); queuestat(who, errstr);}
#define queuestat2(who,fmt,a1,a2) \
	{ sprintf(errstr,fmt,a1,a2); queuestat(who, errstr); }
#define queuestat3(who,fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); queuestat(who, errstr); }
#define queuestat4(who,fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); queuestat(who, errstr); }
#define mdebug(str)	      { if (debug) muser(debugfile,str); }
#define mdebug1(str,a1)	      { if (debug) muser1(debugfile,str,a1); }
#define mdebug2(str,a1,a2)    { if (debug) muser2(debugfile,str,a1,a2); }
#define mdebug3(str,a1,a2,a3) { if (debug) muser3(debugfile,str,a1,a2,a3); }
#define mdebug4(str,a1,a2,a3,a4) \
	{ if (debug) muser4(debugfile,str,a1,a2,a3,a4); }
#define mdebug5(str,a1,a2,a3,a4,a5) \
	{ if (debug) muser5(debugfile,str,a1,a2,a3,a4,a5); }

/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern int optind;
extern char *optarg;

extern char **environ;


/* local structures */

/*
 * Generic queue structure.
 * The set of queues, jobs, and running jobs are kept
 * in doubly linked lists like this.  The first element
 * is not part of the queue, so "q == q->q_forw" iff q is empty.
 * An empty queue is returned by emptyq().
 * Items are added by enqueue(), removed by dequeue().
 * The queue links must appear first in queue structures
 * because of the way insque() and remque() work.
 */

#if	(! defined(SUNOS5))

struct qelem {
	struct qelem *q_forw;
	struct qelem *q_back;
	char q_data[1];		/* not used */
};

#endif /* (! defined(SUNOS5)) */

#define FOREACH(p, set) 	/* iterate p over set */ \
	for (p = set; (p = p->forw) != set; )

struct job {
	struct job *forw, *back;	/* queue links; must be first */
	char	j_cfname[MAXNAMELEN+1];	/* batch queue cf* file name */
	int	j_localuid;		/* local uid running job */
	char	*j_userid;		/* userid@host running the job */
	char	*j_mailuserid;		/* userid@host to get mail */
	char	*j_jobname;		/* user-supplied name of job */
	char	*j_umask;		/* umask of job */
	char	*j_directory;		/* working directory of job */
	time_t	j_qtime;		/* time queued */
	int	j_pid;			/* non-zero iff job is running */
	char	j_seen;			/* marks jobs we found in a queue */
	struct jobqueue *j_queue;		/* backpointer to queue */
	DOUBLE	j_totalcpu;
} ;

struct jobqueue {
	struct jobqueue *forw, *back;	/* queue links; must be first */
	unsigned q_nochange:1,	/* Set if no change since last runqueue() */
		q_drain:1,	/* Set to stop new jobs from starting */
		q_deleteq:1,	/* Set if queue is being deleted */
		q_stopped:1,	/* Set if queue stopped for lower load */
		q_startup:1,	/* Set after leftover of* files cleaned out */
		q_restart:1,	/* Restart on system crash? */
		q_seen:1,	/* Used while scanning for deleted queues */
		q_noprofile:1;	/* No profile; throttle error msgs */
	short	q_nice;		/* Job priority */
	short	q_nexec;	/* Current # of executing jobs */
	short	q_maxexec;	/* Max # of executing jobs */
	short	q_loadsched;	/* Stop scheduling new jobs if over this */
	short	q_loadstop;	/* Stop jobs if over this load */
	long	q_minfree;	/* minimum free space on device */
	fdev	q_mfdev;	/* dev that minfree applies to */
	time_t	q_mtime;	/* mtime of queue directory */
	time_t	q_profmtime;	/* mtime of queue profile file */
	bat_Mail q_supmail;	/* Type of mail for supervisor */
	bat_Mail q_usermail;	/* Type of notification for user */
	char	*q_supervisor;	/* queue supervisor (usually a file name) */
	char	*q_name;	/* name of queue, e.g. troff, later */
	char	*q_cfdir;	/* q_name/Q_CFDIR */
	char	*q_profile;	/* q_name/Q_PROFILE */
	char	*q_queuestat;	/* q_name/Q_QUEUESTAT */
	char	*q_status1;	/* line 1 of status */
	char	*q_program;	/* external queue enabling/disabling control */
	char	*q_timestop;	/* time spec. during which queue is enabled */
	char	*q_timesched;	/* time spec. during which new jobs are run */
	int	q_oldstat;	/* Previous status from "program" option */
	int	q_statcntr;	/* Counter for "program" status message */
	struct job *q_jobs;	/* linked list of jobs */
#ifdef BSD4_2
	struct rlimit q_rlimit[RLIM_NLIMITS];	/* Resource hard/soft limits */
#endif
} *queues ;

/*
 * List of running jobs.
 * We use this to map a dead child's pid to a job (and hence queue).
 */
struct running {
	struct	running *forw, *back;	/* queue links; must be first */
	int r_pid;
	struct job *r_job;
} *running ;

/*
 * Signals to ignore.
 */
const char ignore[] = {
	SIGPIPE,
#ifdef JOBCTL
	SIGTTOU, SIGTTIN, SIGTSTP,
#endif
} ;


/* forward references */

static int	getoutnodename(char *,int) ;

static SIGRET sigalrm();
static SIGRET sigchld();
static SIGRET sighandler();
static SIGRET toggledebug();
static SIGRET restartdaemon();
#ifdef BSD4_2
static SIGRET sig_ignore();
#endif

static int muser(char *,char *) ;

static char *syserr();

static struct qelem *enqueue();
static struct qelem *emptyq();

static struct job *jobinfo();
#ifdef __hpux
char *mymalloc();
#else
static char *mymalloc();
#endif
static char *mycalloc();
static FILE *sendmail();
static fdev getfs();
static long fsfree();
static DIR *opencfdir();
static char *myhostname();
static void mperror(), merror(), readqueues(), getload(),
	drainqueue(), readpro(), requeue(), runqueue(),
	queuestat(), abortall(), terminated(), merror(), mailclose(),
	freequeue(), freeqstorage(), releasedev(), freejob(),
	freerunning(), mailback();
static int Ktorl(), rltoi(), itorl(), sigalljobs(), startjob(), sigjob(),		waitforchild(), abortjob();


/* local variables */

static FILE	*efp = stderr ;

static int nkids;
static int saverrno;		/* used to save errno */
static int sigalrmflag;
static int sigchldflag;
static char *spooldir = SPOOLDIR ;
static char *pidfile = PIDFILE ;

#ifdef __hpux
char *debugfile = DEBUGFILE;
char errstr[STRSIZ];			/* Global sprintf buffer */
int debug = 0;				/* turn on for debugging output */
static int hpuxsystemflag;		/* true if just did "system" call */
static int hpuxstatus;			/* Pass child status around */
static int terminatedfailed;		/* true if "terminated" failed */
#else
static char *debugfile = DEBUGFILE;
static char errstr[STRSIZ];		/* Global sprintf buffer */
static int debug = 0;			/* turn on for debugging output */
#endif

static char *envuser = NULL;		/* $USER */
static int batchdpid = 0;		/* getpid() */
static int restartflag = 0;		/* set to cause reloading batchd */
static int totalrunning = 0;		/* sum of all q_nexec job counts */
static int needtime;			/* true when we need the time */

static const char	*envinit[] = { "LOGNAME=root", 0, 0, 0 };
static const char	*prog_mailer = "/usr/bin/rmail" ;

static char	mailaddr[MAXPATHLEN + 1] ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat sbuf;

	struct jobqueue *qp;

	FILE	*f ;

	DIR	*spooldirdot ;

	LOGFILE		lh ;

	unsigned sleeptime = SLEEPTIME ;

	time_t	daytime, spool_mtime = 0 ;

	pid_t	pid = getpid() ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, load[3] ;
	int	cl ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_logopen = FALSE ;

	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	username[USERNAMELEN + 1] ;
	char	logid[LOGFILE_LOGIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*logfname = NULL ;
	char	*execfname = NULL ;
	char	*progname = NULL ;
	char	*p ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = strbasename(argv[0]) ;

	if ((cp = getenv(VARSTDERRFNAME)) != NULL)
		efp = fopen(cp,"a+") ;

#ifdef	DEBUGFNAME
	nprintf(DEBUGFNAME,"main: errfname=%s\n",cp) ;
	for (i = 0 ; envv[i] != NULL ; i += 1)
	nprintf(DEBUGFNAME,"main: envv[%u]=%s\n",i,envv[i]) ;
#endif

#if	CF_SERVERERR && 0
	fprintf(efp,"%s: hello there\n",progname) ;
	fflush(efp) ;
#endif

	mailaddr[0] = '\0' ;

	getnodedomain(nodename,domainname) ;

	/*
	 * Grab USER from environment for debugging.
	 * Save our pid so children can kill us if something
	 * goes really wrong.
	 */

	envuser = username ;
	getusername(username,USERNAMELEN,-1) ;

#ifdef	DEBUGFNAME
	nprintf(DEBUGFNAME,"main: here is\n") ;
#endif

#if defined(BSD4_2) && !defined(SUNOS5)
	/*
	 * Ignore SIGTTOU before we try to print any messages.
	 */
	(void)signal(SIGTTOU, SIG_IGN);
	setlinebuf(efp);
#endif

	if ((cp = getenv("_")) != NULL)
		execfname = cp ;

	if ((execfname == NULL) && (argv[0][0] == '/'))
		execfname = argv[0] ;

	if ((cp = getenv(VARMAILER)) != NULL)
		prog_mailer = cp ;

	if ((cp = getenv(VARMAILER)) != NULL) {
		
		rs = cfdecui(cp,-1,&sleeptime) ;

		if ((rs < 0) || (sleeptime > (24*60)))
			sleeptime = SLEEPTIME ;

	}

	if ((cp = getenv(VARSPOOLDNAME)) != NULL)
		spooldir = cp ;

	if ((cp = getenv(VARPIDFNAME)) != NULL)
		pidfile = cp ;

	if ((cp = getenv(VARLOGFNAME)) != NULL)
		logfname = cp ;

#ifdef	DEBUGFNAME
	nprintf(DEBUGFNAME,"main: 1 spooldname=%s\n",spooldir) ;
	nprintf(DEBUGFNAME,"main: 1 pidfname=%s\n",pidfile) ;
	nprintf(DEBUGFNAME,"main: 1 logfname=%s\n",logfname) ;
#endif

	while ((i = getopt(argc, argv, "d:p:t:D")) != EOF) {

		switch (i) {

		case 'd':	/* -d spooldir */
			spooldir = optarg;
			break;

		case 'p':
			pidfile = optarg;
			break;

		case 't':
			sleeptime = atoi(optarg);
			if ((int)sleeptime <= 0)
				goto usage;

			break;

		case 'D':
			debug++;
			break;

		default:
		usage:
			fprintf(efp, 
			"%s: USAGE> %s [-d dir] [-p file] [-t time] [-D]\n", 
			argv[0],argv[0]) ;

			exit(1);

		} /* end switch */

	} /* end while */

	if (optind < argc)
		goto usage;	/* spurious args */

#ifdef	DEBUGFNAME
	nprintf(DEBUGFNAME,"main: spooldname=%s\n",spooldir) ;
	nprintf(DEBUGFNAME,"main: pidfname=%s\n",pidfile) ;
	nprintf(DEBUGFNAME,"main: logfname=%s\n",logfname) ;
#endif

	if (mailaddr[0] == '\0') {

		if ((cp = getenv(VARMAILADDR)) != NULL)
			sncpy1(mailaddr,MAXPATHLEN,cp) ;

	}

	if (mailaddr[0] == '\0') {

		sncpy5(mailaddr,MAXPATHLEN,username,"@",
			nodename,".",domainname) ;

	}

/* open a log file if we have one */

	if ((logfname != NULL) && (logfname[0] != '-')) {

		cp = getenv("NODE") ;
		if (cp == NULL)
			cp = "jobsubs" ;

		snsd(logid,LOGFILE_LOGIDLEN,cp,((uint) pid)) ;

		rs1 = logfile_open(&lh,logfname,0,0666,logid) ;
		f_logopen = (rs1 >= 0) ;

		daytime = time(NULL) ;

		logfile_printf(&lh,"%s Jobsubs",
			timestr_logz(daytime,timebuf)) ;

	} /* end if (log file) */


	/*
	 * Make sure we start with a clean signal state.
	 */
	for (i = 1; i <= NSIG; i++)
		(void) signal(i, SIG_DFL);

/* setup new environment */

	i = 1;

#ifdef	COMMENT
	/* Preserve $TZ so localtime() works under SysV */
	if ((p = getenv("TZ")) != NULL)
		envinit[i] = p - strlen("TZ=");

	environ = &envinit[1];	/* get rid of environment */
#endif /* COMMENT */

/* continue with thie utter crap! */

#ifdef BSD4_2
# ifdef SUNOS5
        {
                sigset_t s;
                sigfillset(&s);
                sigprocmask(SIG_UNBLOCK, &s, 0);
        }
# else
        (void)sigsetmask(0);            /* unblock all signals */
# endif
#endif

	umask(022);		/* set reasonable mask */

	if (chdir(spooldir) < 0) {
		fprintf(efp, 
		"%s: Can't chdir to %s: %s\n", 
		argv[0], spooldir, syserr()) ;

		exit(1);
	}

	if ((spooldirdot = opendir(".")) == NULL)
		fprintf(efp, "%s: Can't open spool dir %s: %s\n",
			argv[0], spooldir, syserr()), exit(1);

	if (stat(".", &sbuf) < 0)
		fprintf(efp, "%s: fstat %s: %s\n",
			argv[0], spooldir, syserr()), exit(1);

#ifdef	COMMENT
	if (sbuf.st_uid != 0)
		fprintf(efp, "%s: %s is not owned by root\n",
			argv[0], spooldir), exit(1);
#endif /* COMMENT */

#ifdef	COMMENT
	if (sbuf.st_mode & (S_IWRITE>>6))
		fprintf(efp, "%s: %s: mode %03o allows general write\n",
			argv[0], spooldir, sbuf.st_mode), exit(1);
#endif /* COMMENT */

	if ((f = fopen(pidfile, "r")) != NULL) {

		if (fscanf(f, "%d", &i) == 1 && i > 1 && kill(i, 0) == 0) {
			fprintf(efp, 
			"%s: Batchd seems to be running "
			"already (%s contains pid %d). "
			"Please check.\n", 
			argv[0], pidfile, i);
			exit(1);
		}
		fclose(f);
	}
	if ( debug ) {
		char fname[MAXPATHLEN + 1];


		/* Start with fresh debug file */

		sprintf(fname, "%s.bak", debugfile);
		(void) rename(debugfile, fname);

	} else {

		if ((i = fork()) == -1)
			error("Could not fork\n");

		if (i)
			_exit(0);
	}
	batchdpid = getpid();
	running = (struct running *) emptyq();
	queues = (struct jobqueue *) emptyq();

	/* Log my pid */
	if ((f = fopen(pidfile, "w")) != NULL) {

		fprintf(f, "%d\n", batchdpid);
		if ( fclose(f) == EOF ) {
			saverrno = errno;
			fprintf(efp, "Failure on fclose(%s): %s\n",
				pidfile, syserr() );
			errno = saverrno;
			error2("Failure on fclose(%s): %s\n",
				pidfile, syserr() );
		}

	} else {

		saverrno = errno;
		fprintf(efp, "Failure on fopen(%s,\"w\"): %s\n",
			pidfile, syserr() );
		errno = saverrno;
		error2("Failure on fopen(%s,\"w\"): %s\n", pidfile, syserr() );
	}

	if (!debug) {

		for (i = 0; i < sizeof ignore/sizeof ignore[0]; i++)
			(void)signal(ignore[i], SIG_IGN);

#ifdef	COMMENT
		nice(-40);
		nice(20);
		nice(0);
#endif /* COMMENT */

		/*
		 *  Get rid of controlling tty
		 */
		setsid() ;

	}

	(void)signal(SIGALRM, sigalrm);
	(void)signal(SIGHUP,  sighandler);
	(void)signal(SIGTERM, sighandler);
	(void)signal(SIGINT,  sighandler);
	(void)signal(SIGQUIT, sighandler);
	(void)signal(SIGCHLD, sigchld);
#ifdef SIGUSR1
	(void)signal(SIGUSR1, toggledebug);
#endif
#ifdef SIGUSR2
	(void)signal(SIGUSR2, restartdaemon);
#endif

#ifdef __hpux
	hpuxsystemflag = 0;		/* Clear HP-UX "system" call flag */
#endif

	/*
	 * Go to sleep for a while before flooding the system with
	 * jobs, in case it crashes again right away, or the
	 * system manager wants to prevent jobs from running.
	 * Send a SIGALRM to give it a kick-start.
	 */

#ifdef	COMMENT
	if (!debug) {
		alarm(sleeptime);
#if defined(BSD4_2) && !defined(SUNOS5)
		sigpause(0);
#else
		pause();
#endif
		(void) alarm(0);
	}
#endif /* COMMENT */

	for (;;) {

		/*
		 *  Check if the main spool directory changed.  If so, make
		 *  a new list of potential queue entries.
		 */
		if (stat(".", &sbuf) < 0) {
			mperror1("Can't fstat(%s/.) - batchd exiting",
				spooldir);
			exit(1);
		}
		if (sbuf.st_mtime != spool_mtime) {
			spool_mtime = sbuf.st_mtime;
			mdebug("re-reading spool directory\n");
			readqueues(spooldirdot);
		}
		getload(load);
		mdebug3("load averages are %d, %d, %d\n", load[0],
		   load[1], load[2]);

		/*
		 * Get the current time at most once during
		 * the queue loop below.
		 */
		needtime = 1;

		/*
		 *  Now check each potential queue entry.
		 */
		FOREACH (qp, queues) {

			mdebug1("checking queue %s\n", qp->q_name);
			/*
			 * If we get a signal to restart the daemon, we
			 * have to drain all the queues first.
			 */
			if ( restartflag )
				drainqueue(qp);
			/*
			 *  Check if its profile file changed; if so re-read it.
			 *  Be generous, if somebody deleted it don't affect
			 *  the queue or what's running.
			 */
			if (stat(qp->q_profile, &sbuf) < 0) {
				int e = errno;
				if ( stat(qp->q_name, &sbuf) < 0 ){
					/* queue vanished; this will be
					 * handled by readqueues() next
					 * time around.
					 */
					mperror1("Can't stat(%s); "
						"queue vanished?",
						qp->q_name);
				} else {
					/*
					 * Profile has vanished.
					 * Complain, but only once; otherwise
					 * we'll flood the mail system.
					 */
					if (!qp->q_noprofile) {
						errno = e;
						mperror1("Can't stat(%s); "
							"profile missing?",
							qp->q_profile);
						qp->q_noprofile = 1;
					}
				}
			} else if (sbuf.st_mtime != qp->q_profmtime) {
				qp->q_profmtime = sbuf.st_mtime;
				qp->q_noprofile = 0;
				mdebug1("\tnew profile modify time; "
					"reading profile %s\n",
					qp->q_profile);
				readpro(qp);
			}
			/*
			 * If not draining, we check for more jobs to add
			 * to our internal lists.  We could always add jobs,
			 * even if draining, since draining prevents jobs
			 * from starting, but that would just waste memory.
			 * In fact, we should probably release the lists
			 * of non-running jobs in drained queues -- we can
			 * get the lists back when the queue is restarted.
			 * Check the queue directory change time.  If it
			 * changed, then something new might have been queued
			 * or deleted.  Unfortunately, output files are also
			 * created in the queue directory, causing unnecessary
			 * requeueing when they get added and deleted.
			 */
			if ( qp->q_drain == Q_STARTNEW ) {
				DIR *cfdir;

				if ( (cfdir=opencfdir(qp)) != NULL ){
				    	if (stat(qp->q_cfdir, &sbuf) >= 0
					    && sbuf.st_mtime != qp->q_mtime) {
						qp->q_mtime = sbuf.st_mtime;
						mdebug1(
		"\tnew '%s' modify time; checking for newly queued jobs\n",
							qp->q_cfdir);
						requeue(qp);
					}
					closedir(cfdir);
				}
			}
			/*
			 * Handle starting and stopping our running jobs
			 * and start new jobs if conditions are right.
			 * Even if draining we call runqueue() for the
			 * purpose of starting and stopping current jobs.
			 */
			runqueue(qp, load);

		} /* end for */

		/*
		 * We have started as many jobs as we can.
		 * Now wait for signals.  ALRM is sent by BATCH when a
		 * new job is queued; we also send it to ourselves every
		 * sleeptime seconds so we keep track of changing load.
		 * SIGUSR2 turns on a flag that causes us to stop accepting
		 * new jobs, drain the current ones, and after everything
		 * is gone reload ourself from argv[0].  This makes
		 * starting a new batchd much easier; replace the binary
		 * and send batchd SIGUSR2.
		 * Any job terminating causes CHLD.  Make sure SIGCHLD
		 * is permitted, so we can get job statuses.
		 *
		 * If waitforchild() found no dead kids, don't
		 * rescan the queue, just reset the alarm.
		 * This seems to happen if NOKMEM is defined,
		 * since the popen()/pclose() generates a SIGCHLD.
		 */

resetalarm:
		mdebug1("Setting alarm to %d\n", sleeptime);
		alarm(sleeptime);
		while ( sigalrmflag == 0  &&  sigchldflag == 0 ) {

			mdebug("pause...\n");

#if defined(BSD4_2) && !defined(SUNOS5)
			sigpause(0);
#else
			pause();
#endif

			mdebug("BING\n");
		}

		(void) alarm(0);
		mdebug("Signalled; checking\n");

#ifdef __hpux
		/*
		 * HP-UX: check if any of our child processes have
		 * overstepped their cpu limit.  GRS
		 */
		if (sigalrmflag)
			CheckHPRunTimes();
#endif

		sigalrmflag = 0;
		if (sigchldflag) {

			mdebug("SIGCHLD flag set; running waitforchild()...\n");
			sigchldflag = 0;
			if (waitforchild() == 0) {
				mdebug("No child found in waitforchild()\n");
				goto resetalarm;
			}
		}

		if (restartflag && totalrunning == 0 ) {

			vecstr	av ;


			mdebug("Queues drained; starting new batchd...\n");
			FOREACH (qp, queues) {
				muser1(qp->q_supervisor,
				   "%s: restarting: batch daemon restarting\n",
				   qp->q_name);
				queuestat(qp,"Daemon restarting\n");
			}

			/* Arrange that almost everything get closed on exec.
			 */
			for (i=3; i < getdtablesize() ; i += 1)
				(void) fcntl(i, F_SETFD, 1);/* close-on-exec */

			(void) unlink(pidfile);

#ifdef	COMMENT
			rs = vecstr_start(&av,20,VECSTR_OCOMPACT) ;

			if (rs >= 0) {

				cl = sfbasename(execfname,-1,&cp) ;

				vecstr_add(&av,cp,cl) ;

				for (i = 1 ; argv[i] != NULL ; i += 1)
					vecstr_add(&av,argv[i],-1) ;

				u_execv(execfname, av.va) ;

				vecstr_finish(&av) ;

			}
#else /* COMMENT */
				u_execv(execfname,argv) ;
#endif /* COMMENT */

			error1("Cannot execv '%s'; batchd exiting\n", argv[0]);

			break ;
		}
		mdebug("looking around...\n");

	} /* end for */

	if (f_logopen)
		logfile_close(&lh) ;

	ex = EX_OK ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
bad1:
	ex = EX_NOPROG ;
	goto ret0 ;

}
/* end subroutine (main) */


static char * stralloc( str )
char *str;
{
	char *p = mymalloc(strlen(str)+1);


	strcpy(p,str);
	return(p);
}


static SIGRET sigchld()
{

	sigchldflag = 1;

#ifdef	COMMENT
	mdebug("SIGCHLD\n");
#endif

}


static SIGRET sigalrm()
{


#ifndef BSD4_2
	(void)signal(SIGALRM, sigalrm);
#endif /*BSD4_2*/
	sigalrmflag = 1;

#ifdef	COMMENT
	mdebug("SIGALRM\n");
#endif

}


/* This will probably be called during a SHUTDOWN of the system.
 * Hope we have enough time to send all these messages...
 */

static SIGRET sighandler(sig)
int sig;
{
	struct jobqueue *qp;

#ifdef BSD4_2
# ifdef SUNOS5
        sigset_t all, old;

        sigfillset(&all);
        sigprocmask(SIG_BLOCK, &all, &old);
# else
	(void) sigblock(~0);		/* Block everything */
# endif
#else /*BSD4_2*/
	(void)signal(SIGALRM, SIG_IGN);
	(void)signal(SIGHUP,  SIG_IGN);
	(void)signal(SIGINT,  SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, SIG_IGN);
	(void)signal(SIGCHLD, SIG_IGN);
#endif /*BSD4_2*/

#ifdef	COMMENT
	mdebug1("Signal %d\n", sig);
#endif

	(void) unlink(pidfile);
	/*
	 * First send the messages, then do the aborting,
	 * in case we are short of time (e.g. during SHUTDOWN).
	 */

#ifdef	COMMENT
	FOREACH (qp, queues)
		muser2(qp->q_supervisor,
		   "%s: %s -- Aborting all jobs; batch daemon exiting.\n",
		   qp->q_name, (sig<NSIG) ? strsignal(sig) : "SHUTDOWN" );
#endif /* COMMENT */

	FOREACH (qp, queues)
		abortall(qp);

	_cleanup();			/* Flush stdio buffers */

#ifdef BSD4_2
# ifdef SUNOS5
        sigprocmask(SIG_SETMASK, &old, 0);
# else
	(void) sigsetmask(0);		/* Unblock everything */
# endif
#endif /*BSD4_2*/

	signal(sig,SIG_DFL);		/* Permit the signal */
	kill(batchdpid,sig);
	exit(1);
}


static SIGRET toggledebug()
{


	debug = !debug;
}


static SIGRET restartdaemon()
{


	mdebug("RESTART daemon signal\n");
	restartflag = 1;		/* start draining queues */
	kill(batchdpid,SIGALRM);		/* wake us up */
}


/* Called from main() if we've received a SIGCHLD child ending signal.
 * Returns number of child processes reaped.
 */

static int waitforchild()
{
	int pid;
	DOUBLE totalcpu;

#if defined(BSD4_2) && !defined(SYSVR4)
	union wait status;
	struct rusage rusage;
#else /*BSD4_2*/

#ifndef CLK_TCK
#define CLK_TCK 60
#endif /*CLK_TCK*/
#ifdef SUNOS5
	int status;
#else
	union { int w_status; } status;
#endif
	struct tms tms;
	DOUBLE oldtotalcpu, newtotalcpu;
#endif /*BSD4_2*/

	int found = 0;	/* "found a child" flag */
	int s;


#if defined(BSD4_2) && !defined(__hpux) && !defined(SUNOS5)
	while ((pid = wait3(&status, WNOHANG, &rusage)) > 0) {
		totalcpu = (double)( rusage.ru_utime.tv_sec
			   + rusage.ru_stime.tv_sec )
			 + (double)( rusage.ru_utime.tv_usec
			   + rusage.ru_stime.tv_usec ) / 1000000.0;
#else /*BSD4_2*/
	(void) times(&tms);
	oldtotalcpu = (DOUBLE)tms.tms_cutime + tms.tms_cstime;
#ifdef __hpux
	while ((pid = wait3(&status, WNOHANG, NULL)) > 0) {
#else
	while ((pid = wait(&status)) > 0) {
#endif

#ifdef SUNOS5
		s = status;
#else
		s = status.w_status;
#endif
		(void) times(&tms);
		newtotalcpu = (DOUBLE)tms.tms_cutime + tms.tms_cstime;
		totalcpu = (newtotalcpu - oldtotalcpu) / (DOUBLE)CLK_TCK;
		oldtotalcpu = newtotalcpu;
#endif /*BSD4_2*/

		mdebug3("wait exit pid=%d, stat=0%o, cpu=%.1f\n",
			pid, s, totalcpu);
		terminated(pid, s, totalcpu);

#ifdef __hpux
		/*
		 * If "terminated" couldn't find the job, and "hpuxsystemflag"
		 * is 1 (indicating we just made a call to "system"),
		 * we've (probably) got the status of the process run by
		 * the "system" call. If so, save this status in
		 * "hpuxstatus" for future reference.
		 *
		 * Otherwise, remove this job from the processes being watched.
		 */
		if (terminatedfailed && hpuxsystemflag) {
			hpuxsystemflag = 0;
			hpuxstatus = status.w_status;
		} else {
			RmHPWatch(pid);
		}
#endif
		found++;		/* Set child found flag */
	}

#ifndef BSD4_2
	(void)signal(SIGCHLD, sigchld);
#endif /*BSD4_2*/

	return found;
}


extern int sys_nerr;
extern char *sys_errlist[];


static char * syserr()
{
	static char buf[80];


	if (errno >= 0 && errno < sys_nerr)
		return(sys_errlist[errno]);

	sprintf(buf,"Unknown error %d", errno);
	return(buf);
}


static void mperror(s)
char *s;
{
	char *p;
	char str[STRSIZ];	/* must have own internal buffer */


	if ( (p= strchr(s,'\n')) != NULL )
		*p = '\0';

	sprintf(str,"%s: %s\n", s, syserr());

	if ( p )
		*p = '\n';

	merror(str);
}


static void merror(s)
char *s;
{
	FILE *mail;


	if (debug && envuser != NULL)
		mail = sendmail(envuser, envuser);
	else
		mail = sendmail(mailaddr, (char *)NULL);

	/*
	 * Might as well throw in a subject.  Makes filtering mail easier
	 * Need a blank line in case text line begins with a blank.
	 * The text, which we're sure is one line,
	 * already includes one newline.
	 * ..sah
	 */
	fprintf(mail, 
		"Subject: batch queue error on %s: %s\n%s", 
		myhostname(),
		s, s);

	mailclose(mail);
}


/*
 * Fatal error; causes batchd or child process to exit.
 */
error(s)
char *s;
{
	char str[STRSIZ];	/* must have own internal buffer */


	sprintf(str, "BATCHD fatal error; batch terminating:\n - %s", s);
	merror(str);
	_cleanup();
	abort();
	exit(1);
}


static int muser(s, m)
char *s, *m;
{
	FILE *mail;
	time_t now;


	/* Sometimes the q_supervisor field will be NULL.  */

	if (s == NULL || strcmp(s,DEV_NULL) == 0)
		return;

	if (strchr(s,'/') != NULL) {

		if ((mail = fopen(s, "a")) == NULL) {
			mperror1("Can't fopen(%s,\"a\"); using STDERR", s);
			mail = efp;
		}
		(void) fcntl(fileno(mail), F_SETFL, O_APPEND);
		now = time((time_t *)0);
		fprintf(mail, "%-15.15s ", ctime(&now) + 4);

	} else {
		/*
		 * If it's going to a real user, which so far is
		 * just for chatty job-startup messages,
		 * throw in a subject. ..sah
		 */
		mail = sendmail(s,(char *)NULL);
		fprintf(mail, "Subject: batch queue on %s: %s\n", 
			myhostname(),
			m);
	}
	fputs(m, mail);
	mailclose(mail);

	return 0 ;
}


static FILE * sendmail(to,from)
char *to;
char *from;
{
	FILE *mailer;
	int pid;
	struct running *rp;
	struct {int read; int write;} mailp;

	char	*cp ;


	if ( to == NULL ) {
		fprintf(efp, 
			"BATCHD: internal error: sendmail to NULL\n");
		return efp;
	}
	if ( from == NULL )
		from = "batch";

	if (debug) {
		fprintf(efp, "SENDMAIL: To '%s' from '%s': ", to, from);
		(void) fflush(efp);
		return efp;
	}
	if (pipe((int *)&mailp) < 0) {

		perror("batchd pipe()");

err:
		fprintf(efp, "BATCHD: sendmail to '%s' failed\n", to);
		return efp;
	}

	switch (pid = fork()) {

	case 0:
		if (mailp.read != 0) {
			close(0);
			dup(mailp.read);
			close(mailp.read);
		}
		close(mailp.write);

/* the preferred mailer program */

		if ((prog_mailer != NULL) &&
			(prog_mailer[0] != '\0') &&
			(prog_mailer[0] != '-')) {

		sfbasename(prog_mailer,-1,&cp) ;

		execl(prog_mailer,cp,to,NULL) ;

		perror("BATCHD execl failed");
		fflush(efp);

		}

/* the other mailer program if the preferred one fails */

		execl("/usr/bin/rmail", "rmail", to,NULL) ;

		perror("BATCHD execl /bin/rmail failed");
		fflush(efp);

		exit(1);
		/*NOTREACHED*/

	case -1:
		goto err;

	default:
		close(mailp.read);

	} /* end switch */

	nkids++;
	rp = (struct running *)
		enqueue((struct qelem *)running, (int)sizeof *rp);

	rp->r_pid = pid;
	mailer = fdopen(mailp.write, "w");

	if (mailer == NULL) {
		perror("BATCHD sendmail: FDOPEN failed");
		close(mailp.write);
		goto err;
	}
	/*
	 * Just one \n here so we can throw in a subject. ..sah
	 */
	fprintf(mailer, "To: %s\n", to);
	fprintf(mailer, "From: The Batch Daemon <batch>\n");
	fflush(mailer);
	if (ferror(mailer)) {
		fclose(mailer);
		goto err;
	}
	return mailer;
}


static void mailclose(file)
FILE *file;
{


	if (file != efp)
		(void) fclose(file);
	else
		(void) fflush(file);
}


/*
 * Read the home directory, looking for all current queue entries.
 * Symbolic links are ignored, so you can symlink queue names together
 * and the batch command will put all requests in one queue directory.
 * readqueues() is done any time we wake up and notice the home directory
 * has changed since we last did this.
 *
 * We first turn off the "seen" bit on every queue we already know about.
 * Then, we look at all the queues in the directory.
 * Any queues left, that have "seen" bits still off, must have been deleted
 * and are to be shut down.
 */

static void readqueues(dir)
DIR *dir;
{
	struct jobqueue *qp;
	struct_dir *d;
	struct ustat sb;


	FOREACH (qp, queues)
		qp->q_seen = 0;

	rewinddir(dir);
	while ((d = readdir(dir)) != NULL) {
		/*
		 *  Ignore everything starting with a dot.
		 */
		if ( d->d_name[0] == '.' )
			continue;

		if (lstat(d->d_name, &sb) < 0) {

			mperror1("Can't lstat(%s); skipping queue",
				d->d_name);
			continue;
		}
		/*
		 * If the queue name is a local symlink we can safely ignore
		 * it because it must link to some other queue name.
		 */
		if ((sb.st_mode & S_IFMT) == S_IFLNK) {

			char buf[MAXPATHLEN + 1];
			int cc;


			if ((cc=readlink(d->d_name,buf,(int)sizeof(buf))) < 0) {

				mperror1("Can't readlink(%s); skipping queue",
					d->d_name);
				continue;
			}
			buf[cc] = '\0';
			mdebug2("Symlinked queue '%s'->'%s'\n",
				d->d_name, buf);

			if (strchr(buf,'/') == NULL)
				continue;	/* skip relative symlinks */

			/*
			 * Must be a symlink with a slash in it;
			 * have to assume it's a different queue.
			 * stat() it to get its real mode.
			 */
			if (stat(d->d_name, &sb) < 0) {

				mperror1("Can't stat(%s); skipping queue",
					d->d_name);
				continue;
			}
		}
		if ((sb.st_mode & S_IFMT) != S_IFDIR) {
			errno = ENOTDIR;
			mperror1("Junk file name '%s' ignored", d->d_name);
			continue;
		}
		/*
		 *  Found a directory.  Do we already know about it?
		 */
		FOREACH (qp, queues)
			if (strcmp(d->d_name, qp->q_name) == 0)
				break;
		/*
		 *  Was it a new queue?
		 */
		if (qp == queues) {

			char str[STRSIZ];


			qp = (struct jobqueue *)
			    enqueue((struct qelem *)queues, (int)sizeof *qp);
			/*
			 * This stuff is freed by freequeue() when
			 * the queue entry is deleted.
			 */
			qp->q_name = stralloc(d->d_name);

			mkpath2(str,qp->q_name,Q_QUEUESTAT) ;

			qp->q_queuestat = stralloc(str);

			mkpath2(str,qp->q_name,Q_PROFILE) ;

			qp->q_profile = stralloc(str);

			mkpath2(str,qp->q_name,Q_CFDIR) ;

			qp->q_cfdir = stralloc(str);

			qp->q_jobs = (struct job *) emptyq();

			mdebug1("New queue '%s'\n", qp->q_name);
		}
		qp->q_seen = 1;
	}
	/*
	 * Any directories which have been deleted imply the queues
	 * should end violently since we can't get the output files.
	 * If jobs are executing, setting q_deleteq will cause
	 * the queue entry to be released when the last job terminates.
	 * If no jobs are executing, we release the entry now.
	 */
	FOREACH (qp, queues) {

		if (qp->q_seen)
			continue;

		merror2("'%s': Aborting vanished queue containing %d jobs\n",
			qp->q_name, qp->q_nexec);

		qp->q_deleteq = 1;
		if (qp->q_nexec == 0)
			freequeue(qp);
		else
			abortall(qp);
	}
}


/*
 *  (Re-)Read a profile file under the given queue entry.
 */
static void readpro(qp)
struct jobqueue *qp;
{

#ifdef BSD4_2
	static struct rlimit minus_one = { 
		(unsigned) -1,
		(unsigned) -1
	};
	int ri;
	int rl;
	int limit;
#endif /*BSD4_2*/

	int i;
	enum { E_ON, E_DRAIN, E_OFF } execflag;
	FILE *pro;
	bat_Mail *mailp;
	char *startmsg;
	enum keyword kwd;
	extern enum keyword yylex();
	extern char yytext[];
	extern int yylineno;


	/*
	 *  First set up queue to all defaults so if we re-read a file
	 *  we don't get the old values.
	 */
	freeqstorage(qp);		/* get rid of old info */
	qp->q_usermail = qp->q_supmail = MAIL_END|MAIL_CRASH;
	qp->q_restart = 0;
#ifdef BSD4_2
	for ( i=0; i<RLIM_NLIMITS; i++ )
		qp->q_rlimit[i] = minus_one;
#endif /*BSD4_2*/

	qp->q_deleteq	= 0;
	qp->q_nochange	= 0;
	qp->q_drain	= Q_STARTNEW;	/* Default is to run the queue */
	qp->q_nice	= BAT_INICE;
	qp->q_maxexec	= BAT_IMAXEXEC;
	qp->q_loadsched	= BAT_ILOADSCHED;
	qp->q_loadstop	= BAT_ILOADSTOP;
	qp->q_minfree	= BAT_IMINFREE;
	qp->q_oldstat	= -1;
	qp->q_statcntr	= 0;

	/*
	 *  Try to read the profile file.  If not there, assume shutdown this
	 *  queue.
	 */
	execflag = E_ON;		/* set later if "exec" used */
	if ((pro = fopen(qp->q_profile, "r")) == NULL) {
		mperror1("Can't fopen(%s,\"r\"); draining this queue",
			qp->q_profile);
		drainqueue(qp);	/* stop further jobs from starting */
		return;
	}

	lexfile(pro);

#define	LEX(kwd)		(kwd = yylex())

	while ((int)LEX(kwd) != 0) {

		switch (kwd) {

		case K_LINE:
			break;
		case K_EXEC:
			switch (LEX(kwd)) {

			case K_OFF:
				/* abort running jobs */
				/* don't start new jobs */
				execflag = E_OFF;
				break;
			case K_LINE:
			case K_ON:
				/* don't abort running jobs */
				/* start new jobs */
				execflag = E_ON;
				break;
			case K_DRAIN:
				/* don't abort running jobs */
				/* don't start new jobs */
				execflag = E_DRAIN;
				break;
			default:
				goto syntaxerr;

			} /* end switch */

			break;

		case K_MAXEXEC:
			if (LEX(kwd) != K_NUMBER)
				goto syntaxerr;
			i = atoi(yytext);
			if ( i < 0 || i > 100 )
				goto syntaxerr;/* Dumb error msg. -IAN! */

			qp->q_maxexec = (char)i;
			break;
		case K_SUPERVISOR:
			if (LEX(kwd) != K_VARIABLE)
				goto syntaxerr;
			qp->q_supervisor = stralloc(yytext);
			break;
		case K_MAIL:
			mailp = &qp->q_usermail;
		mailstuff:
			*mailp = 0;
			if (LEX(kwd) == K_LINE)
				break;
			if (kwd != K_VARIABLE)
				goto syntaxerr;
			if (strchr(yytext, 's'))
				*mailp |= MAIL_START;
			if (strchr(yytext, 'e'))
				*mailp |= MAIL_END;
			if (strchr(yytext, 'c'))
				*mailp |= MAIL_CRASH;
			break;
		case K_MAILSUPERVISOR:
			mailp = &qp->q_supmail;
			goto mailstuff;

		case K_LOADSCHED:
			if (LEX(kwd) != K_NUMBER)
				goto syntaxerr;

			i = atoi(yytext);

			if ( i < 0 || i > 100 )
				goto syntaxerr;/* Dumb error msg.  -IAN! */

			qp->q_loadsched = (char)i;
			break;

#ifdef JOBCTL
		case K_LOADSTOP:
			if (LEX(kwd) != K_NUMBER)
				goto syntaxerr;
			i = atoi(yytext);
			if ( i < 0 || i > 100 )
				goto syntaxerr;/* Dumb error msg. -IAN! */
			qp->q_loadstop = (char)i;
			break;
#endif /*JOBCTL*/

		case K_NICE:
			if (LEX(kwd) != K_NUMBER)
				goto syntaxerr;
			i = atoi(yytext);
			if ( i < (-40) || i > 40 )
				goto syntaxerr;
				/* What a dumb error msg.  -IAN! */
			qp->q_nice = (char) i;
			break;

#ifdef BSD4_2
		case K_RLIMITCPU:
		case K_RLIMITFSIZE:
		case K_RLIMITDATA:
		case K_RLIMITSTACK:
		case K_RLIMITCORE:
		case K_RLIMITRSS:
			/* If only one number is given, we set only the
			 * current limit.  If two numbers, we set current and
			 * maximum limits.
			 */
/* index in q_rlimit array */
			ri = rltoi(rl = Ktorl(kwd));

			if (LEX(kwd) != K_NUMBER)
				goto syntaxerr;

			limit = atoi(yytext);
			if (LEX(kwd) == K_LINE) {

				if (getrlimit(rl,&(qp->q_rlimit[ri])) != 0) {
					mperror1("%s: getrlimit failed",
						qp->q_name);
					break;
				}
			}
			qp->q_rlimit[ri].rlim_cur = limit;
			if (kwd == K_LINE)
				break;

			if (kwd != K_NUMBER)
				goto syntaxerr;

			qp->q_rlimit[ri].rlim_max = atoi(yytext);
			break;
#endif /*BSD4_2*/

		case K_MINFREE:
			if (LEX(kwd) != K_VARIABLE)
				goto syntaxerr;

			if ((qp->q_mfdev = getfs(yytext)) < 0) {
				merror2("%s: getfs(%s) failed\n",
					qp->q_profile, yytext );
/* eat the number */
				LEX(kwd);			
				break;
			}
			if (LEX(kwd) != K_NUMBER || (i = atoi(yytext)) < 0) {
				releasedev(qp->q_mfdev);
				goto syntaxerr;
			}
			qp->q_minfree = i;
			break;
		case K_RESTART:
			qp->q_restart = 1;
			break;
#ifndef BSD4_2
#ifndef JOBCTL
		case K_LOADSTOP:
#endif /*JOBCTL*/
		case K_RLIMITCPU:
		case K_RLIMITFSIZE:
		case K_RLIMITDATA:
		case K_RLIMITSTACK:
		case K_RLIMITCORE:
		case K_RLIMITRSS:
			merror1(
			"'%s' is not available on this system, ignored\n",
			    yytext);
			while (LEX(kwd) != K_LINE)
				;
			break;
#endif /*BSD4_2*/

		case K_PROGRAM:
			if (LEX(kwd) != K_VARIABLE)
				goto syntaxerr;

			if (qp->q_program)
				merror1(
		"%s: profile has more than one program spec.\n", 
			qp->q_name);

			qp->q_program = 
			mymalloc(40+strlen(yytext) + strlen(qp->q_name)+3);

			sprintf(qp->q_program, "%s; %s %s",
				debug ?
					"set -x" :
					"exec </dev/null >/dev/null 2>&1",
				yytext, qp->q_name);
			break;

		case K_TIMESTOP:
			if (LEX(kwd) != K_VARIABLE)
				goto syntaxerr;

			if (qp->q_timestop)
				merror1(
			"%s: profile has more than one timestop spec.\n", 
				qp->q_name);

			qp->q_timestop = stralloc(yytext);
			break;
		case K_TIMESCHED:
			if (LEX(kwd) != K_VARIABLE)
				goto syntaxerr;

			if (qp->q_timesched)
				merror1(
		"%s: profile has more than one timesched spec.\n", 
		qp->q_name);

			qp->q_timesched = stralloc(yytext);
			break;
		default:
			goto syntaxerr;
		}
		if (kwd != K_LINE)
			if (yylex() != K_LINE)
				goto syntaxerr;

	} /* end while */

	(void) fclose(pro);

	switch (execflag) {

	case E_ON:
		startmsg = "enabled";	/* exec on: just keep going */
		break;
	case E_OFF:
		abortall(qp);	/* exec off: kill all jobs and drain queue */
		startmsg = "aborted";
		break;
	case E_DRAIN:
		drainqueue(qp);	/* exec drain: stop further jobs starting */
		startmsg = "draining";
		break;
	default:
		startmsg = "unknown";
		merror2("%s: internal batchd error for exec flag %d\n",
			qp->q_name, execflag);
		drainqueue(qp);	/* stop further jobs from starting */
		return;

	} /* end switch */

	if ( qp->q_supervisor == NULL ) {
		merror1("%s: No queue supervisor userid or file specified\n",
		    qp->q_profile );
		drainqueue(qp);	/* stop further jobs from starting */
		return;
	}

	if ( qp->q_loadsched<=0 || qp->q_loadstop<=0 || qp->q_maxexec<=0 ) {
		merror4(
		    "%s: loadsched %d or loadstop %d or maxecec %d is <= 0\n",
		    qp->q_profile, qp->q_loadsched, qp->q_loadstop,
		    qp->q_maxexec );
		drainqueue(qp);	/* stop further jobs from starting */
		return;
	}

/* start creating a mail message (body text in 'errstr') */

	sprintf(errstr, "%s: %s: maxexec=%d loadsched=%d\n",
		qp->q_name, startmsg, qp->q_maxexec, qp->q_loadsched );

#ifdef JOBCTL
	sprintf( strchr(errstr,'\n'), " loadstop=%d\n", qp->q_loadstop);
#endif

	sprintf( strchr(errstr,'\n'), " nice=%d\n", qp->q_nice);

	if ( qp->q_minfree != 0 )
		sprintf( strchr(errstr,'\n'), 
		" minfree=%ld\n", qp->q_minfree);

	if (qp->q_timestop)
		sprintf(strchr(errstr,'\n'), 
		" timestop=%s\n", qp->q_timestop);

	if (qp->q_timesched)
		sprintf(strchr(errstr,'\n'), 
		" timesched=%s\n", qp->q_timesched);

	if (qp->q_program)
		sprintf(strchr(errstr,'\n'), 
		" program='%s'\n", qp->q_program);

#ifdef BSD4_2
	{
		struct rlimit *rlp = &(qp->q_rlimit[rltoi(RLIMIT_CPU)]);
		if ( rlp->rlim_cur != -1 && rlp->rlim_max != -1 )
			sprintf( strchr(errstr,'\n'), " cpu %d min\n",
			    rlp->rlim_cur/60 );
		else
			sprintf( strchr(errstr,'\n'), " cpu inf\n");
	}
#else
	sprintf( strchr(errstr,'\n'), " cpu inf\n");
#endif

/* do not send stupid mail messages for stupid common reasons */

	switch (execflag) {

	case E_ON:
	case E_OFF:
	case E_DRAIN:
		break ;

	default:
	    muser(qp->q_supervisor, errstr);

	} /* end switch (sending a stupid mail message) */

	qp->q_status1 = stralloc(errstr);

	return;

syntaxerr:
	merror3("%s: Syntax error in profile, line %d near '%s'\n",
	    qp->q_profile, yylineno, yytext);

	drainqueue(qp);	/* stop further jobs from starting */
}


/*
 * Open, or create and open, the directory containing cf* queued files.
 * Return NULL on failure.
 */

static DIR * opencfdir(qp)
struct jobqueue *qp;
{
	DIR *cfdir;


	while ( (cfdir = opendir(qp->q_cfdir)) == NULL) {

		int saveumask;

		if ( errno != ENOENT ) {
			mperror1("Cannot opendir(%s); draining queue",
				qp->q_cfdir);

			drainqueue(qp);	/* stop further jobs from starting */
			return NULL;
		}
		saveumask = umask(022);
		if ( mkdir(qp->q_cfdir, 0775) < 0 ) {
			mperror1("Cannot mkdir(%s); draining queue",
				qp->q_cfdir);
			drainqueue(qp);	/* stop further jobs from starting */
			(void) umask(saveumask);
			return NULL;
		}
		(void) umask(saveumask);

#ifdef	COMMENT
		muser1(qp->q_supervisor, "%s: directory created\n",
			qp->q_cfdir);
#endif /* COMMENT */

		/* mkdir worked; loop back to try opendir() again */

	}
	return cfdir;
}


/*
 * (Re-)Read the q_cfdir directory for a queue and find all jobs ready to run.
 *
 * First, turn off all the "seen" bits for known jobs in this queue.
 * Then, read the queue directory and for each directory entry that
 * starts with "cf" see if we recognize its name.
 * If we recognize its name, turn on the "seen" bit for the job.
 * If unrecognized, record the job and turn on the "seen" bit.
 * After all this, re-scan our list of known jobs in this queue;
 * if any jobs are not marked "seen" then they have been deleted
 * and we must abort them.
 *
 * On start-up, once, check for of* files that indicate jobs were
 * killed in mid-execution.  Restart those that ask for it.
 */

static void requeue(qp)
struct jobqueue *qp;
{
	DIR *cfdir;
	struct_dir *d;
	struct job *jp;
	char fname[MAXNAMELEN*3+3];
	int trouble;


	qp->q_nochange = 0;	/* things have changed */

	if ( (cfdir = opencfdir(qp)) == NULL)
		return;

	FOREACH (jp, qp->q_jobs)
		jp->j_seen = 0;

	while ((d = readdir(cfdir)) != NULL) {

		/*
		 *  Ignore everything starting with a dot.
		 */
		if ( d->d_name[0] == '.' )
			continue;
		/*
		 * Ignore anything that doesn't start with "cf".
		 * Warn about junk files in this directory.
		 * Don't warn about tf* temp files.
		 */
		if ( d->d_name[0] != 'c' || d->d_name[1] != 'f' ) {

			if ( d->d_name[0] != 't' || d->d_name[1] != 'f' )
				merror2("Junk file name '%s/%s' ignored\n",
					qp->q_cfdir, d->d_name);
			continue;
		}
		FOREACH (jp, qp->q_jobs)
			if (strcmp(d->d_name, jp->j_cfname) == 0)
				break;

		if (jp != qp->q_jobs) {
			jp->j_seen = 1;	/* already know about job */
			continue;
		}
		/*
		 * New job; add it.
		 */
		jp = (struct job *)
			enqueue((struct qelem *)qp->q_jobs, (int)sizeof *jp);
		if ( jobinfo(jp, qp, d->d_name) == NULL ) {
			freejob(jp);
			continue;
		}
		mdebug5("\tnew job '%s/%s' (%s) user '%s' (%s)\n",
			qp->q_cfdir, jp->j_cfname, jp->j_jobname,
			jp->j_userid, jp->j_mailuserid);
	}
	closedir(cfdir);
	/*
	 * Look at our internal table and check that every job we
	 * know about is still in the directory.  If one has been
	 * deleted and is currently running, abort it; otherwise
	 * just remove the job from the table.
	 */
	trouble = 0;
	FOREACH (jp, qp->q_jobs)
		if (jp->j_seen == 0) {
			mdebug1("\tdropping vanished job %s\n", jp->j_cfname);
			if (jp->j_pid) {
				if (!abortjob(jp)) {
					trouble = 1;
					merror1(
		"Trouble aborting vanished job %s\n", 
		jp->j_cfname);

					muser1(qp->q_supervisor, 
			"Trouble aborting vanished job %s\n", 
			jp->j_cfname);

		/*
		 * The Apollo loses track of jobs/processes sometimes,
		 * possibly due to forking problems, and
		 * the cleanup routine 'terminated' fails.
		 * To recover from this, just delete the input
		 * and output files manually, and this code will
		 * get batchd back into sync.
		 * Could try
		 *	terminated(jp->j_pid, SIGKILL<<8, 0.0);
		 * instead of the 'freejob' immediately below here,
		 * but what if 'terminated' fails again (it already
		 * failed once or we wouldn't be here). There is
		 * also the problem that the job might not be in
		 * the list of "running" jobs any more.
		 * If 'terminated' could be used reliably, the
		 * 'trouble' loop below this loop could be eliminated.
		 */
					freejob(jp);
				}
			} else
				freejob(jp);
		}
	/*
	 * If there was a vanished job that was supposedly still
	 * running but could not be aborted, the job count for
	 * this queue is probably wrong, so figure it out again.
	 * Also try to correct the totalrunning counter.
	 */
	if (trouble) {
		totalrunning -= qp->q_nexec;
		qp->q_nexec = 0;
		FOREACH (jp, qp->q_jobs)
			if(jp->j_pid)
				qp->q_nexec++;
		totalrunning += qp->q_nexec;
	}

	/*
	 * Check for restarted jobs.  This is done exactly once per new queue,
	 * usually when BATCHD gets started after a reboot and makes a list
	 * of its queues.  Nothing turns q_startup off again, so this gets
	 * done only once, even if a queue is aborted and re-enabled.
	 * You have to actually delete the queue to reset the flag.
	 * We also create a missing q_cfdir subdirectory at this point.
	 */
	if (qp->q_startup == 0) {
		DIR *qname;

		qp->q_startup = 1;
		queuestat(qp,"Daemon restarting\n");

		if((qname = opendir(qp->q_name)) == NULL) {
			mperror1("Can't opendir(%s); draining queue",
				qp->q_name);
		}
		while ((d = readdir(qname)) != NULL) {
			/*
			 *  Ignore everything starting with a dot.
			 */
			if ( d->d_name[0] == '.' )
				continue;

			if ( d->d_name[0] != 'o' || d->d_name[1] != 'f' ) {
				/*
				 * Delete all old process group files.
				 * Zero old queue status files.
				 */
				if ((d->d_name[0]=='e' && d->d_name[1]=='f')) {

					mkpath2(fname,
						qp->q_name, d->d_name) ;

					(void)unlink(fname);
				}
				continue;
			}
			/*
			 * Must be an of* output file.
			 * Do we know about its corresponding cf* file?
			 */
			FOREACH (jp, qp->q_jobs)
				if (strcmp(d->d_name+1, jp->j_cfname+1) == 0)
					break;
			if (jp == qp->q_jobs) {
				/*
				 * cf* file not found: we have a left over
				 * of* output file with no control file.
				 * Complain and get rid of it.
				 */
				mkpath2(fname,qp->q_name, d->d_name) ;

				(void)unlink(fname);
				muser1(qp->q_supervisor,
					"%s: Old output file deleted\n", fname);
				continue;
			}
			/*
			 * Old of* output file for a cf* file found; restart.
			 * mailback() deletes the of* and ef* files.
			 */
			mailback(0, jp, MAIL_CRASH,
			    qp->q_restart?"restarted":"not restarted");

			if (qp->q_restart)
				continue;
			/*
			 *  Restart disallowed, delete the cf* file.
			 */
			mkpath2(fname,qp->q_cfdir, jp->j_cfname) ;

			if ( unlink(fname) == -1) {

				saverrno = errno;
				mperror1("Can't unlink(%s)", fname);
				muser2(qp->q_supervisor,
				    "%s: Can't unlink dead job%s",
				    qp->q_name,
				    saverrno==ENOENT? "": "; stopping queue");
				if (saverrno != ENOENT)
					/* stop further jobs from starting */
					drainqueue(qp);
			}
			freejob(jp);
		}
		closedir(qname);
	}
}


/*
 * Get information on this job and return it in jp.
 * Return NULL if anything goes wrong.
 */

static struct job * jobinfo( jp, qp, cfname )
struct job *jp;		/* where to put job info */
struct jobqueue *qp;	/* queue name on which to queue job */
char *cfname;		/* cf* file name of queued job */
{
	int i;
	char filename[MAXNAMELEN*3+3];
	FILE *f;
	char *p;
	int linecount;
	struct ustat sbuf;
	char buf[1024];		/* Big enough for longest header line */

#define BD_ENT(s) { s, sizeof(s)-1, NULL }

	static struct parse_table {
		char *str;
		int len;
		char **where;
	} parsetable[] = {
		BD_ENT("# userid: "),		/* j_userid */
		BD_ENT("# mailuserid: "),	/* j_mailuserid */
		BD_ENT("# jobname: "),	/* j_jobname */
		BD_ENT("# umask: "),		/* j_umask */
		BD_ENT("# directory: "),	/* j_directory */
		BD_ENT("")
	};


	i = 0;
	parsetable[i++].where = &(jp->j_userid);
	parsetable[i++].where = &(jp->j_mailuserid);
	parsetable[i++].where = &(jp->j_jobname);
	parsetable[i++].where = &(jp->j_umask);
	parsetable[i++].where = &(jp->j_directory);

	if ( strlen(cfname) >= MAXNAMELEN) {
		merror3("%s: '%s': file name longer than %d characters\n",
			qp->q_cfdir, cfname, (MAXNAMELEN-1)) ;
		return NULL;
	}

	mkpath2(filename,qp->q_cfdir, cfname) ;

	if ((f = fopen(filename, "r")) == NULL) {
		mperror1("Can't fopen(%s,\"r\")", filename);
		return NULL;
	}

	if ( fstat(fileno(f), &sbuf) < 0 ) {
		mperror1("Can't fstat(%s)", filename);
		return NULL;
	}

	jp->j_seen     = 1;		/* we've seen this job */
	jp->j_queue    = qp;		/* back pointer to queue */
	jp->j_localuid = sbuf.st_uid;	/* local uid running job */
	jp->j_qtime    = sbuf.st_mtime;	/* time of queueing */
	(void) strcpy(jp->j_cfname, cfname);

	/* Read the file to get userid information
	 * tucked up at the start of the file.
	 * We assume none of the starting lines are longer than sizeof buf.
	 */
	for ( linecount=0;  fgets(buf, (int)sizeof buf, f) != NULL; ) {

		struct parse_table *pt;
		char *q;
		int c;


		++linecount;

		if ( linecount > 10 ) {
			merror2("'%s' line %d: Can't find end of headers.\n",
				filename, linecount );
			return NULL;	/* Header must be in first few lines */
		}

		q = strchr(buf, '\n');	/* find the trailing newline */
		if ( q == NULL ) {

			merror4(
		"'%s' line %d: Line longer than %d chars:\n%70.70s...\n",

				filename, linecount, (int)sizeof(buf), buf);
			return NULL;
		}

		if ( q == buf )
			break;	/* null line means end of headers */

		/*
		 * We should be pointing at a newline here.
		 * Trim trailing white space from line just read.
		 */
		c = *--q;	/* just before the newline */
		while ( isascii(c) && isspace(c) )
			c = *--q;

		*++q = '\0';

		/*
		 * Look up the line prefix, including the comment char.
		 */
		for ( pt=parsetable; pt->len != 0; pt++ ) {

			if ( strncmp(buf, pt->str, pt->len) == 0 ) {
				/*
				 * Trim leading white space.
				 */
				p = buf + pt->len;
				c = *p;
				while ( isascii(c) && isspace(c) )
					c = *++p;

				if ( q > p ) {
					*pt->where = mymalloc(q-p+1);
					strcpy(*pt->where,p);

				} else{

					*pt->where = mymalloc(0+1);
					strcpy(*pt->where,"");
				}
			}
		}
	}
	if ( ferror(f) ) {
		mperror1("Error reading '%s'", filename);
		return NULL;
	}
	(void) fclose(f);

	/*
	 * Only believe the header's userid stuff if ROOT owns the file.
	 * One has to make sure that the file is not writable *at any time*
	 * by a non-priviledged user if we are to believe the header.
	 * We could chown the file from root to j_localuid after reading
	 * the header, since that would let the user read the contents,
	 * but then how do we know the header hasn't been corrupted
	 * after a system crash and restart, during the times when the
	 * file is writable by the user?  Looks like the file has to
	 * be owned by root and thus remain protected at all times.
	 * This means we need one secure userid to own the file (e.g. root),
	 * one to run the file (j_localuid), one who owns the job
	 * (j_userid) and one to send mail to (j_mailuserid).
	 */
	if ( jp->j_localuid != 0 || jp->j_userid == NULL ) {

		struct passwd *pw;


		if ((pw = getpwuid(jp->j_localuid)) == NULL) {

			merror2("%s: %d: no such local uid\n",
				filename, jp->j_localuid);
			return NULL;
		}

		/* This forgery check will go away when we run each job under
		 * its own secure local uid.
		 */

#ifdef	COMMENT /* some sort of stupid UID check */
		if (( jp->j_localuid != 0) && (jp->j_userid != NULL) &&
		    (strcmp(jp->j_userid,pw->pw_name) != 0)) {

			merror3("%s: '%s' attempted to forge userid '%s'\n",
				filename, pw->pw_name, jp->j_userid);
			return NULL;
		}
#endif /* COMMENT */

		jp->j_userid = stralloc(pw->pw_name);
	}

	if ( jp->j_mailuserid == NULL )
		jp->j_mailuserid = stralloc(jp->j_userid);

	if ( jp->j_jobname == NULL )
		jp->j_jobname = stralloc(jp->j_cfname);

	return jp;
}


/*
 * A queue that might have changed.
 * Wander along it and start jobs if possible.
 * If we couldn't fork we just return; we'll try again later.
 * The q_queuestat file contains the queue status.
 */

static void runqueue(qp, load)
struct jobqueue *qp;
int *load;
{
	struct job *jp;
	struct job *bestjp;
	long fsf;

	int start, drain = 0, printstat;
	int	ml ;

	char logmsg[LOGMSGLEN + 1] ;


	/*
	 * The variable "start" is non-zero iff all conditions
	 * are go for running jobs in this queue.
	 * The conditions are: load average low enough,
	 * current time within the queue's time spec(s),
	 * queue program says the queue is enabled.
	 * The last two conditions only apply if the queue
	 * has an associated time spec. and program.
	 *
	 * We check the conditions in increasing order of cost,
	 * to avoid the expensive tests.  (It would be nice to avoid
	 * these expensive tests if there are no jobs in the queue.)
	 * As we go, we append test results to the string "logmsg".
	 */

	start = load[1] < qp->q_loadstop;
	snprintf(logmsg,LOGMSGLEN, "5 min load average %d %s %d, ",
			load[1], start ? "<" : ">=", qp->q_loadstop);

	if (start && (qp->q_timestop || qp->q_timesched)) {

		/* Current time must be within the queue's timestop spec. */
		static struct tm now;


		if (needtime) {		/* read the clock */
			time_t t;
			time(&t);
			now = *localtime(&t);
			needtime = 0;
		}
		if (qp->q_timestop) {
			start = checktime(qp->q_timestop, &now);
			mdebug3("%s: timestop '%s' says %s\n",
				qp->q_name, qp->q_timestop,
				start ? "start" : "stop");

			ml = strlen(logmsg) ;

			snprintf((logmsg + ml),(LOGMSGLEN - ml),
				"timestop '%s' is %sactive, ",
				qp->q_timestop, start ? "" : "in");

		}
		if (start && qp->q_timesched) {
			drain = !checktime(qp->q_timesched, &now);
			mdebug3("%s: timesched '%s' says %s\n",
				qp->q_name, qp->q_timesched,
				drain ? "drain" : "start");

			ml = strlen(logmsg) ;

			snprintf((logmsg + ml),(LOGMSGLEN - ml),
				"timesched '%s' says %s, ",
				qp->q_timesched, drain ? "drain" : "start");

		}
	}
	if (start && qp->q_program) {
		/* Queue's program must agree */
		int status;


		mdebug2("%s: running '%s'\n", qp->q_name, qp->q_program);
		status = system(qp->q_program);
		mdebug3("%s: status is %d/%d\n",
			qp->q_name, status&0xff, (status>>8)&0xff);

#ifdef __hpux
		/*
		 * HP-UX "system" call is totally brain-damaged:
		 * it returns status of -1 if a SIGCHLD signal
		 * handler is active. It reports the status of
		 * the "system" call via the "wait" status, so
		 * go do a waitforchild and get the status from there.
		 * The SIGCHLD may also take several seconds to arrive
		 * if another job finishes at the same time as the
		 * "system" call. To help deal with this, if no
		 * unclaimed child (which should be the "system" call)
		 * is found, just use the status from the last time.
		 * Also, wait for 1 second to let the signal arrive.
		 */
		sleep(1);
		if (status==-1 && sigchldflag) {
			mdebug(
		"SIGCHLD flag set after 'system'; running waitforchild()...\n");
			sigchldflag = 0;
			hpuxstatus = qp->q_oldstat;
			hpuxsystemflag = 1;
			if (waitforchild() == 0) {
				merror("No child found in waitforchild()\n");
			}
			if (hpuxsystemflag) {

				muser1(qp->q_supervisor,
			"%s: No unclaimed child found in waitforchild()\n",
					qp->q_name);

				muser3(qp->q_supervisor,
					"%s: Re-using old status %d/%d\n",
					qp->q_name, hpuxstatus&0xff,
					(hpuxstatus>>8)&0xff);
				hpuxsystemflag = 0;
			}
			status = hpuxstatus;
		}
		mdebug3("%s: status after waitforchild is %d/%d\n",
			qp->q_name, status&0xff, (status>>8)&0xff);
#endif /* __hpux */

		/*
		 * Only print enabled/disabled/drain status if the
		 * queue status has changed, or every 50 passes.
		 */
		qp->q_statcntr++;
		if ((qp->q_oldstat == status) && ((qp->q_statcntr / 50) == 0))
			printstat = 0;

		else {
			printstat = 1;
			qp->q_statcntr = 0;
		}
		qp->q_oldstat = status;		/* Save current status */
		switch ((status&0xff) ? -1 : ((status>>8)&0xff)) {

		case 0:	/* start */
			mdebug1("starting %s\n", qp->q_name);

#ifdef	COMMENT
			if (printstat)
				muser1(qp->q_supervisor, 
					"%s: Queue is enabled\n",
					qp->q_name);
#endif /* COMMENT */

			start = 1;
			ml = strlen(logmsg) ;

			snprintf((logmsg + ml),(LOGMSGLEN - ml),
				"Program '%s' says start, ",
				qp->q_program);

			break;

		case 1: /* stop */
			mdebug1("stopping %s\n", qp->q_name);

#ifdef	COMMENT
			if (printstat)
				muser1(qp->q_supervisor, 
					"%s: Queue is disabled\n",
					qp->q_name);
#endif /* COMMENT */

			start = 0;
			ml = strlen(logmsg) ;

			snprintf((logmsg + ml),(LOGMSGLEN - ml),
				"Program '%s' says stop, ",
				qp->q_program);

			break;

		case 2:	/* don't schedule new jobs */
			mdebug1("draining %s\n", qp->q_name);

#ifdef	COMMENT
			if (printstat)
				muser2(qp->q_supervisor, "%s: Queue is %s\n",
					qp->q_name, 
					qp->q_nexec ? "draining" : "drained");
#endif /* COMMENT */

			ml = strlen(logmsg) ;

			snprintf((logmsg + ml),(LOGMSGLEN - ml),
				"Program '%s' says drain, ",
				qp->q_program);

			drain++;
			break;

		default:
			/*
			 * Bogus exit status.
			 * Stop everything, drain the queue, notify supervisor,
			 * and ignore the program.
			 */
			merror3(
	"Queue %s: program '%s' returned weird status %d; draining queue\n",
				qp->q_name, qp->q_program, status);

			drainqueue(qp);
			free(qp->q_program);
			qp->q_program = NULL;
			start = 0;
			break;
		}
	}
	if (strcmp(logmsg+strlen(logmsg)-2, ", ") == 0)
		logmsg[strlen(logmsg)-2] = 0;		/* clean up msg tail */

	if (qp->q_stopped && start) {		/* Restart stopped queue */

		qp->q_stopped = 0;
		queuestat1(qp, "Restarted; %s\n", logmsg);
		if ( qp->q_nexec != 0 ) {
			muser3(qp->q_supervisor,
			    "%s: Restarted; jobs=%d %s\n",
			    qp->q_name, qp->q_nexec, logmsg);

#ifdef JOBCTL
			(void) sigalljobs(qp, SIGCONT);
#endif
		}
	}

	if (!qp->q_stopped && !start) {		/* Stop the queue */
		qp->q_stopped = 1;
		queuestat1(qp, "Stopped; %s\n", logmsg);
		if ( qp->q_nexec != 0 ) {

			muser3(qp->q_supervisor, "%s: Stopped; jobs=%d %s\n",
			    qp->q_name, qp->q_nexec, logmsg);

#ifdef JOBCTL
			(void) sigalljobs(qp, SIGSTOP);
#endif
		}
	}

	/* We assume that if the queue is stopped we don't want to worry
	 * about starting any more jobs.  Unix doesn't let you exec a
	 * job and have it in "stopped" mode anyway.
	 */
	if ( qp->q_stopped ) {
		mdebug1("\tqueue is (still) stopped; jobs = %d\n", qp->q_nexec);
		/* Don't set the q_nochange flag; we want to check the
		 * load every time around.
		 */
		return;
	}

	/* If we can't start more jobs, or the "program" for this queue
	 * said to drain the queue, don't go on checking.
	 */
	if ((! start) || drain) {
		mdebug1("Can't start new jobs: %s\n", logmsg);
		queuestat1(qp, "Can't start new jobs: %s\n", logmsg);
		return;
	}

	/* Queue is not stopped.
	 * If the q_nochange flag is set, nothing has happened that would
	 * allow us to try to start more jobs, i.e. no job has finished,
	 * the profile hasn't changed, etc., so we don't need to do anything.
	 */
	if (qp->q_nochange) {
		mdebug("\tqueue status has not changed since last time\n");
		return;
	}

	/*
	 * If profile file said "exec off" or
	 * "exec drain", don't start new jobs.
	 */
	if (qp->q_drain == Q_DRAINING) {

		mdebug2("\tqueue is %s; jobs = %d\n",
			qp->q_nexec ? "draining" : "drained", qp->q_nexec);
		queuestat(qp, qp->q_nexec ? "Draining" : "Drained" );
		/* Status of this flag won't change until
		 * a new profile is read.
		 */
		qp->q_nochange = 1;
		return;
	}

	if (load[2] >= qp->q_loadsched) {

		queuestat2(qp,"15 minute load %d >= scheduling limit %d\n",
			load[2], qp->q_loadsched);
		if ( qp->q_nexec != 0 ) {

			muser4(qp->q_supervisor,
				"%s: Paused; jobs=%d  15 min load %d >= %d\n",
				qp->q_name, qp->q_nexec,
				load[2], qp->q_loadsched );

		}

		mdebug2("\tqueue 15 minute load %d >= scheduling limit %d\n",
			load[2], qp->q_loadsched);
		/* Don't set the q_nochange flag; we want to check the
		 * load every time around.
		 */
		return;
	}

	if ( qp->q_minfree != 0 && (fsf=fsfree(qp->q_mfdev)) < qp->q_minfree) {

		queuestat3(qp,"Free space on file system #0%o = %ld < %ld\n",
			qp->q_mfdev, fsf, qp->q_minfree );

		if ( qp->q_nexec != 0 ) {

			muser5(qp->q_supervisor,
	"%s: Paused; jobs=%d space left on file system #0%o = %ld < %ld\n",
				qp->q_name, qp->q_nexec,
				qp->q_mfdev, fsf, qp->q_minfree );
		}
		mdebug3("\tqueue space left on file system #0%o = %ld < %ld\n",
			qp->q_mfdev, fsf, qp->q_minfree );
		/* Don't set the q_nochange flag; we want to check the
		 * file system space every time around.
		 */
		return;
	}

	/*
	 * We must be in a position to consider starting new jobs.
	 * The name of the job determines its priority.
	 * The first letter after "cf" is the overall priority,
	 * after which comes the time the job was queued.
	 * Find the oldest non-running job in the highest priority queue.
	 */
	while (qp->q_nexec < qp->q_maxexec) {

		bestjp = NULL;
		FOREACH (jp, qp->q_jobs) {

			if (jp->j_pid == 0 && (bestjp == NULL ||
			    strcmp(bestjp->j_cfname+BAT_PREFIX_LEN,
			    jp->j_cfname+BAT_PREFIX_LEN) > 0))
				 bestjp = jp;

		}
		if (bestjp == NULL)
			break;		/* no more jobs waiting to start */

#define FORK_FAILED		1
#define UNKNOWN_LOCALUID	2

		switch ( startjob(bestjp) ) {

		case FORK_FAILED:
			return;		/* give up for now; try later */

		case UNKNOWN_LOCALUID:
			break;		/* skip this one */

		case 0:
			break;		/* successful start */

		} /* end switch */

	} /* end while */

	/*
	 * Queue has max jobs running, or no job is waiting to run.
	 * Don't try to do more that starting/stopping currently running
	 * jobs next time around until some event (such as job ending, new
	 * job, new profile, etc.) turns off the q_nochange flag and allows
	 * us to consider starting new jobs.
	 * (Above doesn't hold for programmed or timed queues.)
	 */
	if ((qp->q_program == NULL) && (qp->q_timestop == NULL) && 
		(qp->q_timesched == NULL))
		qp->q_nochange = 1;	/* set before writing queuestat */

	if ( qp->q_nexec == 0 ) {
		queuestat(qp,"No jobs in queue\n");
		mdebug("\tqueue has no jobs\n");

	} else if ( qp->q_nexec < qp->q_maxexec ) {
		queuestat(qp,"All queued jobs started\n");
		mdebug1("\tqueue has all queued jobs started; jobs=%d\n",
			qp->q_nexec );

	} else {
		queuestat(qp,"Running max number of jobs\n");
		mdebug1("\tqueue is running max number of jobs = %d\n",
			qp->q_nexec );
	}

}
/* end subroutine (runqueue) */


/*
 * Write the string into the queue status file.
 */

static void queuestat(qp, str )
struct jobqueue *qp;
char *str;
{
	FILE *fp;


	if ((fp = fopen(qp->q_queuestat, "w")) == NULL) {
		mperror1("Can't open(%s) queue status file for writing",
			qp->q_queuestat);
		return;
	}
	if (qp->q_status1)
		fputs(qp->q_status1, fp);

	fprintf(fp, "%s: drain=%d deleteq=%d stopped=%d jobs=%d: %s\n",
		qp->q_name, qp->q_drain, qp->q_deleteq, qp->q_stopped,
		qp->q_nexec, str);

	if (fclose(fp) != 0)
		mperror1("%s: fclose failed", qp->q_queuestat);
}


#ifdef BSD4_2
static SIGRET sig_ignore(signum) 
{
	return ;
}
#endif


/*
 * Given a pointer to a job that is not yet running,
 * start it executing.
 * Return 0 on success; non-zero on failure to start the job.
 * If the forked child has a disaster, it has no way to communicate
 * that back to batchd itself...
 */

static int startjob(jp)
struct job *jp;
{
	struct jobqueue *qp;
	struct running *rp;
	struct passwd *pw;

	FILE *pgrpfile;

	int i, pid;
	int newpgrp;

	char filename[MAXPATHLEN + 1];
	char fname[MAXPATHLEN + 1];


	qp = jp->j_queue;	/* queue in which job resides */
	mkpath2(filename,qp->q_cfdir, jp->j_cfname) ;

	mdebug1("\tstarting job '%s'\n", filename);

	if ((pw = getpwuid(jp->j_localuid)) == NULL) {

		merror3("%s: Unknown local uid %d (mail=%s)\n", filename,
			jp->j_localuid, jp->j_mailuserid );
		return UNKNOWN_LOCALUID;	/* unsuccessful start */
	}

	/*
	 * Because of the way signals are implemented in Sun Unix
	 *  (a table of function pointers is kept in user space)
	 * we better use fork() instead of vfork(); if you use vfork,
	 * subsequent signal() calls (before an exec) affect the signal tables
	 * of BOTH the parent and the child, which we don't want, and
	 * can cause odd failures of the batch daemon when it tries
	 * to jump in strange ways.  Even under 4.3bsd on Vaxen, the
	 * resource usage is not zeroed in a vfork() child, so be
	 * careful to ignore SIGXCPU until after the exec().
	 * 89/01/06 Even the ignore and handler doesn't work, so don't
	 * bother trying to use vfork() at all until resource usage is
	 * zeroed properly by the kernel.  -IAN!
	 */

#define VFORKBUG
#if defined(BSD4_2) && !defined(sun) && !defined(VFORKBUG)
	pid = vfork();
#else
	pid = fork();
#endif
	if ( pid == -1 ) {
		mperror("batchd forking");
		return FORK_FAILED;	/* unsuccessful start */
	}
	if (pid) {
		/* START OF PARENT */
		int sameuser = 0;
		long delayed;


		mdebug2("\tforked off job '%s', pid %d\n", filename, pid);
		nkids++;
		qp->q_nexec++;
		totalrunning++;	/* total number of running jobs */
		jp->j_pid = pid;
		rp = (struct running *)
			enqueue((struct qelem *)running, (int)sizeof *rp);
		rp->r_pid = pid;
		rp->r_job = jp;
#ifdef __hpux
		/*
		 * Add the job to the list of pids we are watching.  GRS
		 */
		AddHPWatch(pid, qp->q_rlimit);
#endif

#ifdef	COMMENT
		if (qp->q_usermail & MAIL_START)
			muser1(jp->j_mailuserid,
				"%s: Job is starting now.\n", filename);
#endif /* COMMENT */

		if (qp->q_supmail & MAIL_START)
			sameuser = (strcmp(pw->pw_name,jp->j_mailuserid) == 0);

		delayed = time((time_t *)0) - jp->j_qtime;
		delayed = (delayed+30) / 60;	/* convert seconds to minutes */

#ifdef	COMMENT
		muser7(qp->q_supervisor,
			"%s: %s: START (delayed %ld min): %s%s%s%s\n",
			qp->q_name, jp->j_cfname, (long)delayed, pw->pw_name,
			sameuser ? "" : " (",
			sameuser ? "" : jp->j_mailuserid,
			sameuser ? "" : ")" );
#endif /* COMMENT */

		return 0;	/* successful start of job */
		/* END OF PARENT */
	}
	/*
	 * Child. Setuid to the owner, go to the working directory, set up
	 * i/o redirection stuff to mail back output, etc.
	 * If someone deleted the queue, a lot of this setup won't work,
	 * but we don't want to kill off batchd because of it.
	 * If anything fails after the fork(), and we don't kill batchd,
	 * the job will be automatically deleted.  So be careful about
	 * letting a messed-up batchd run and purge all its queues.
	 */

	/* Set our process group to be our pid.  */

#if defined(BSD4_2) && !defined(SYSVR4)
	newpgrp = getpid();
	if ( setpgrp(0, newpgrp) < 0 ) {
		mperror1("setpgrp(0,%d) failed'", newpgrp);
		exit(1);
	}
#else
	newpgrp = setpgrp();
#endif

	/* Write the pgrp to 'ef' filename so the batch cancelling program
	 * knows what process group to killpg().  If write fails,
	 * just assume the queue has been deleted.
	 */
	snprintf(fname,MAXPATHLEN,
		"%s/e%s",qp->q_name, (jp->j_cfname + 1)) ;

	if ((pgrpfile = fopen(fname, "w")) == NULL) {
		mperror1("fopen(%s,\"w\") failed'\n", fname);
		exit(1);
	}
	fprintf(pgrpfile, "pgrp %d\n", newpgrp);
	if ( fclose(pgrpfile) == EOF ) {
		mperror1("fclose on '%s' failed'\n", fname);
		exit(1);
	}

	/* Close all file descriptors and open them as follows:
	 *   0 - /dev/null
	 *   1 - output of* file
	 *   2 - output of* file
	 */
	for (i=0; i < getdtablesize() ; i++)
		close(i);

	open("/dev/null", 0, 0);		/* stdin */

	snprintf(fname,MAXPATHLEN,
		"%s/o%s",qp->q_name, (jp->j_cfname + 1)) ;

	if (creat(fname, 0600) < 0) {		/* stdout */
		mperror1("Can't creat(%s) output file", fname );
		exit(1);
	}
	/* efp */
	if ( dup(1) == -1 ) {
		mperror1("'%s': dup(1) failed", fname );
		exit(1);
	}

	/*
	 * At this point, all user-caused error messages can simply go
	 * in the open output file on efp.
	 */

	/* This creates a file owned by the user.
	 * This means the user can write into this file, so the
	 * user can start a long-sleeping batch job and use this
	 * file for non-quota storage, but then the user could
	 * do that with the user-owned input file too...
	 * If this fails, well, too bad; keep going.  -IAN!
	 */
	if ( fchown(1,pw->pw_uid,pw->pw_gid) == -1 ) {

		mperror3("'%s': fchown(1,%d,%d) failed",
			fname, pw->pw_uid, pw->pw_gid );
		/* no exit; just keep going */
	}

	/*
	 * The original environment has been thrown away, so this becomes
	 * the basic environment.  Everything else comes from "setenv"
	 * commands at the beginning of the queued batch file.
	 */

#define OVERWRITE 1

	if(    setenv("BATCHQUEUE",	qp->q_name,	OVERWRITE)
	    || setenv("BATCHJOB",	jp->j_cfname,	OVERWRITE)
	    || setenv("PATH",		DEFPATH,	OVERWRITE) ){
		fprintf(efp, "BATCHD: '%s': Failed to set environment: %s\n",
			filename, syserr() );
		/* no exit; just keep going */
	}

#ifdef BSD4_2
	/*
	 * vfork doesn't zero resource usage in the child the
	 * way fork does, so ignore XCPU here.  Note that caught
	 * signals are reset to the default after exec.
	 */
#ifdef SIGXCPU
	(void)signal(SIGXCPU, sig_ignore);
#endif
#ifndef __hpux
	for ( i=0; i<RLIM_NLIMITS; i++ ) {
		struct rlimit *rlp = &(qp->q_rlimit[i]);
		if ( rlp->rlim_cur != -1 && rlp->rlim_max != -1 )
			(void) setrlimit( itorl(i), rlp );
	}
#endif /*__hpux*/
#ifndef NO_INITGROUPS
	if ( initgroups(pw->pw_name, pw->pw_gid) < 0 ){
		saverrno = errno;
		fprintf(efp,
		"BATCHD: '%s': initgroups(%s,%d) failed: %s; job deleted",
			filename, pw->pw_name, pw->pw_gid, syserr() );

		errno = saverrno;
		mperror3("'%s': initgroups(%s,%d) failed",
			filename, pw->pw_name, pw->pw_gid );
		exit(1);
	}
#endif /* NO_INITGROUPS */
#endif /*BSD4_2*/
	if (qp->q_nice) {

	    if (abs(qp->q_nice) > 0)
		u_nice(MIN(abs(qp->q_nice),20)) ;

	}

	if ( setgid(pw->pw_gid) == -1 || setuid(pw->pw_uid) == -1 ) {

		saverrno = errno;
		fprintf(efp, 
		"BATCHD: '%s': setgid(%d)/setuid(%d) failed: %s; job deleted",
			filename, pw->pw_gid, pw->pw_uid, syserr());
		errno = saverrno;
		mperror3("'%s': setgid(%d)/setuid(%d) failed",
			filename, pw->pw_gid, pw->pw_uid);
		exit(1);
	}

	/* Go to the working directory before becoming a normal user,
	 * since we assume we had permission to be there when the
	 * file was queued and we may not have permission to get back.
	 * Make sure all pathnames we use are absolute after this chdir().
	 * -IAN!
	 */
	/* Go to the working directory as a normal user; in the presence
	   of NFS, it is much more likely that we can get there but root
	   can't than vice versa, especially since batchd is started with
	   just one default group. [cks:19890928.2203EST] */
	if ( jp->j_directory && chdir(jp->j_directory) == -1 ) {

		fprintf(efp, "BATCHD: '%s': chdir(%s) failed: %s\n",
			filename, jp->j_directory, syserr() );
		fprintf(efp, "BATCHD: directory /tmp used instead\n");
		(void) chdir("/tmp");
		/* no exit; just keep going */
	}

	/*
	 * Make sure we start with a clean signal state.
	 * Don't re-enable the SIGXCPU purposefully ignored above;
	 * it will get reset to default on exec.
	 */
	for (i = 1; i < NSIG; i++) {

#if defined(BSD4_2) && defined(SIGXCPU)
		if( i == SIGXCPU )
			(void)signal(SIGXCPU, sig_ignore);
		else
#endif
			(void) signal(i, SIG_DFL);

	}

#ifdef BSD4_2
#ifdef SUNOS5
	{
		sigset_t s;
		sigfillset(&s);
		sigprocmask(SIG_UNBLOCK, &s, 0);
	}
#else
	(void)sigsetmask(0);		/* unblock all signals */
#endif
#endif

	(void)alarm(0);			/* disable pending alarms */
	if ( jp->j_umask ) {

		int um;
		if ( sscanf(jp->j_umask, "%o", &um) != 1 ) {

			fprintf(efp, 
			"BATCHD: '%s': Unrecognized umask '%s'; 022 assumed\n",
				filename, jp->j_umask );
			um = 022;
			/* no exit; just keep going */
		}
		(void) umask(um);
	}

	/* Because of the chdir() above, we need an absolute pathname.
	 */
	mkpath2(fname, spooldir, filename) ;

#ifdef	COMMENT
	execl(fname, filename,NULL) ;
#else /* COMMENT */
	{
		int	cl ;
		int	ml ;

		char	abuf[MAXNAMELEN + 1] ;
		char	*av[3] ;
		char	*cp ;


		cl = sfbasename(fname,-1,&cp) ;

		ml = MIN(cl,MAXNAMELEN) ;
		strwcpy(abuf,cp,ml) ;

		av[0] = abuf ;
		av[1] = filename ;
		av[2] = NULL ;

		u_execv(fname,av) ;

	}
#endif /* COMMENT */

	saverrno = errno;
	fprintf(efp, "BATCHD: '%s': Unable to execute: %s; job deleted\n",
		fname, syserr() );
	errno = saverrno;
	mperror2("Can't execl(%s,%s,0)", fname, filename);
	exit(1);
	/*NOTREACHED*/
}


#ifdef BSD4_2
/*
 * Structure to match lex keywords to RLIMIT values and to small
 * integers to index the q_rlimit array.
 */
static struct {
	int r;
	enum keyword kwd;
} rtab[] = {
	RLIMIT_CPU,	K_RLIMITCPU,
	RLIMIT_FSIZE,	K_RLIMITFSIZE,
	RLIMIT_DATA,	K_RLIMITDATA,
	RLIMIT_STACK,	K_RLIMITSTACK,
	RLIMIT_CORE,	K_RLIMITCORE,
#ifdef RLIMIT_RSS
	RLIMIT_RSS,	K_RLIMITRSS,
#else
#ifdef RLIMIT_VMEM
	RLIMIT_VMEM,	K_RLIMITRSS,	/* gak */
#endif
#endif
};


/* Turn RLIMIT manifest number into a small Integer 0 <= i < RLIM_NLIMITS
 * used to index the q_rlimit array.
 */

static int rltoi( rl )
int rl;
{
	int i;


	for (i = 0; i < sizeof rtab/sizeof rtab[0]; i++) {

		if (rtab[i].r == rl)
			return i;

	}

	error1("%d: invalid RLIMIT value\n", rl);
	/*NOTREACHED*/
}


/* Turn K token from LEX into RLIMIT number.
 */

static int Ktorl( kwd )
enum keyword kwd;
{
	int i;


	for (i = 0; i < sizeof rtab/sizeof rtab[0]; i++) {

		if (rtab[i].kwd == kwd)
			return rtab[i].r;

	}

	error1("%d: invalid keyword value\n", (int)kwd);
	/*NOTREACHED*/
}

/* Turn small Integer 0 <= i < RLIM_NLIMITS into RLIMIT number.
 */

static int itorl( i )
int i;
{


	if ((unsigned)i < sizeof rtab/sizeof rtab[0])
		return rtab[i].r;

	error1("%d: invalid integer rlimit value\n", i);
	/*NOTREACHED*/
}
#endif /*BSD4_2*/


/*
 * Deal with a terminated job.
 */
static void terminated(pid, status, totalcpu)
int pid;
int status;
DOUBLE totalcpu;
{
	struct running *rp;
	struct jobqueue *qp;
	struct job *jp;
	char cfname[MAXNAMELEN*3+3];
	char str[STRSIZ];


	mdebug1("terminated job pid %d\n", pid);
#ifdef __hpux
	terminatedfailed = 0;
#endif
	FOREACH (rp, running)
		if(rp->r_pid == pid)
			goto found;
#ifdef __hpux
	/*
	 * If the pid wasn't in the internal list, and the "system" call
	 * flag is set, assume all is OK in HP-UX land (and set the
	 * flag to indicate that this routine failed.
	 */
	if (hpuxsystemflag) {
		terminatedfailed = 1;
		return;
	}
#endif

	sprintf(str,
	"Status %d (0%o) from process %d but process not in internal table\n",
		status, status, pid);

	merror(str);
	return;
found:
	nkids--;
	jp = rp->r_job;
	if (jp)
		qp = jp->j_queue;

	freerunning(rp);
	if (jp == NULL) {

		/*  Assume this is return of a mail process */
		if ( status != 0 )
			fprintf(efp,
			"BATCHD: Status %d (0%o) from BATCH MAIL process %d\n",
				status, status, pid);
		return;
	}

	/*
	 * Paranoia
	 */
	{
		struct jobqueue *q;
		struct job *j;


		FOREACH (q, queues) {

			if (q == qp) {

				FOREACH (j, qp->q_jobs)
					if (j == jp)
						goto ok;

			}

		}

		error1("Tables corrupted: pid %d not found\n", pid);

ok:;
	}

	mdebug3("terminated job pid %d name '%s/%s'\n",
		pid, jp->j_queue->q_cfdir, jp->j_cfname );

	jp->j_totalcpu = totalcpu;
	mailback(status, jp, MAIL_END, (char *)0);

#ifdef	COMMENT
	if (qp->q_supmail & MAIL_END) {

		sprintf(errstr,
			"%s: %s: END: cpu %.1fs signal %d exit %d (0%o)\n",
			qp->q_name, jp->j_cfname, totalcpu, status & 0377,
			(unsigned)status >> 8, (unsigned)status >> 8);
		muser(qp->q_supervisor, errstr);
	}
#endif /* COMMENT */

	mkpath2(cfname,qp->q_cfdir, jp->j_cfname) ;

	if (unlink(cfname) == -1) {

		int e = errno;
		

		mperror1("Can't unlink(%s)", cfname);
		sprintf(errstr, "%s: Can't unlink job '%s'%s\n", qp->q_name,
			cfname, e==ENOENT? "": "; stopping queue");
		muser(qp->q_supervisor, errstr);
		if (e != ENOENT)
			drainqueue(qp);	/* stop further jobs from starting */
	}
	freejob(jp);
	qp->q_nochange = 0;
	--totalrunning;	/* total number of running jobs */

	if (--qp->q_nexec == 0) {
		/*
		 *  Drained?
		 */

#ifdef	COMMENT
		if (qp->q_drain == Q_DRAINING) {
			muser1(qp->q_supervisor,
				"%s: drained: finally\n", qp->q_name);
		}
#endif /* COMMENT */

		/*
		 * When a queue vanishes, the q_deleteq flag is set.
		 * When we have dealt with the last terminated
		 * job in this queue, we release the queue table entry.
		 */
		if (qp->q_deleteq)
			freequeue(qp);
	}
}


/* A queue has vanished and no jobs are executing in it.
 * Get rid of all we know about it.
 */

static void freequeue(qp)
struct jobqueue *qp;
{
	struct job *jp;


	if (qp->q_nexec != 0) {

		merror2(
	"Attempt to free queue '%s' before %d jobs have finished",
			qp->q_name, qp->q_nexec);
		return;
	}
	freeqstorage(qp);	/* free profile dynamic storage */
	if (qp->q_name) {
		free(qp->q_name);
		qp->q_name = NULL;
	}
	if (qp->q_queuestat) {
		free(qp->q_queuestat);
		qp->q_queuestat = NULL;
	}
	if (qp->q_profile) {
		free(qp->q_profile);
		qp->q_profile = NULL;
	}
	if (qp->q_cfdir) {
		free(qp->q_cfdir);
		qp->q_cfdir = NULL;
	}

	FOREACH (jp, qp->q_jobs)
		freejob(jp);

	free((char *)qp->q_jobs);
	remque((struct qelem *)qp);
	free((char *)qp);
}


/*
 * Free dynamic strings and close open units in a queue structure.
 * This is done before re-reading the queue profile and when
 * deleting a queue.   Only stuff that might be set in the
 * profile file is freed.
 */

static void freeqstorage(qp)
struct jobqueue *qp;
{

	if (qp->q_minfree) {
		releasedev(qp->q_mfdev);
		qp->q_minfree = 0;
		qp->q_mfdev = -1;
	}
	if (qp->q_supervisor) {
		free(qp->q_supervisor);
		qp->q_supervisor = NULL;
	}
	if (qp->q_status1) {
		free(qp->q_status1);
		qp->q_status1 = NULL;
	}
	if (qp->q_program) {
		free(qp->q_program);
		qp->q_program = NULL;
	}
	if (qp->q_timestop) {
		free(qp->q_timestop);
		qp->q_timestop = NULL;
	}
	if (qp->q_timesched) {
		free(qp->q_timesched);
		qp->q_timesched = NULL;
	}
}


static void freejob(jp)
struct job *jp;
{


	remque((struct qelem *)jp);
	if (jp->j_jobname)
		free(jp->j_jobname);
	if (jp->j_userid)
		free(jp->j_userid);
	if (jp->j_mailuserid)
		free(jp->j_mailuserid);
	if (jp->j_umask)
		free(jp->j_umask);
	if (jp->j_directory)
		free(jp->j_directory);
	free((char *)jp);
}


static void freerunning(rp)
struct running *rp;
{


	remque((struct qelem *)rp);
	free((char *)rp);
}


static void mailback(status, jp, mailstat, expl)
struct job *jp;
bat_Mail mailstat;
char *expl;
{
	extern FILE *sendmail();
	struct jobqueue *qp = jp->j_queue;
	FILE *mail;
	char fname[MAXNAMELEN*3+3];
	char pgrpname[MAXNAMELEN*3+3];
	char iname[MAXNAMELEN*3+3];
	FILE *inputf;
	int n;
	int output;
	int outstat;
	struct ustat sb;

	char *subj_keyword;


	/*
	 * Send mail even if we aren't supposed to if there is output
	 * or status is non-null.
	 * Make sure bounced mail goes to the real user.
	 */
	snprintf(fname,MAXPATHLEN, "%s/o%s", qp->q_name, (jp->j_cfname+1)) ;

	snprintf(pgrpname,MAXPATHLEN, "%s/e%s", qp->q_name, (jp->j_cfname+1)) ;

	outstat = stat(fname, &sb);
	if ((qp->q_usermail & mailstat) == 0  &&
	   (outstat == -1 || sb.st_size==0)  &&
	   status == 0) {
		if(unlink(fname) < 0)
			mperror1("Can't unlink(%s)", fname);
		(void)unlink(pgrpname);
		return;
	}
	mail = sendmail(jp->j_mailuserid,jp->j_userid);
	fprintf( mail, "Sender: %s\n", jp->j_userid);
	fprintf( mail, "Reply-To: %s\n", jp->j_userid);
	fprintf( mail, "Errors-To: %s\n", jp->j_userid);
	/*
	 * Give some sort of hopefully useful subject.
	 */
	if ( mailstat & MAIL_CRASH ) {
		subj_keyword = "Crash";
	} else if ( status != 0 ) {
		subj_keyword = "Unsuccessful";
	} else {
		subj_keyword = "Success";
	}
	fprintf(mail, "Subject: on %s: %s%s: '%s' in '%s' queue\n\n",
		myhostname(),
		subj_keyword, sb.st_size ? "+Output" : "",
		jp->j_jobname, qp->q_name );

	fprintf(mail, "Job '%s' in '%s' queue has ", jp->j_cfname, qp->q_name);

	if (mailstat & MAIL_CRASH)
		fprintf(mail, "been caught in crash; it was %s", expl);

	else {
		fprintf(mail, "completed");
		if(status == 0)
			fprintf(mail, " successfully");
		else {
			int sig = status & 0177;

			if(sig) {
				if(sig > NSIG)
					fprintf(mail, 
					" with unknown status 0%o", status);

				else
					fprintf(mail, 
					" with signal termination: %s",
						strsignal(sig));
				if (status & 0200)
					fprintf(mail, ", and a core dump");

			} else
				fprintf(mail, 
				" with exit code %d", status >> 8);

		}

		fprintf(mail, 
			".\nTotal CPU used: %.1f sec", jp->j_totalcpu);

	}

	fprintf(mail, ".\n");
	/*
	 * Also send along first few lines of the input,
	 * to make it clear what was going on.
	 * sahayman oct 31/86
	 */
	mkpath2(iname,qp->q_cfdir, jp->j_cfname) ;

	if ( (inputf = fopen(iname, "r")) == NULL ) {

		fprintf(mail, "Can't open batch command file '%s': %s\n",
			iname, syserr());
	} else {
		char buf[BUFSIZ];
		int ilines;		/* # of input lines read */


		/*
		 * Look for the line indicating the start of
		 * user commands (past all the setenv environment crap.)
		 * Print first few lines.
		 */
		fprintf(mail, "Command list follows:\n\n");
		ilines = -1;		/* Indicates CONTROL_STR not yet seen */
		while ( fgets(buf, (int)sizeof(buf), inputf) != NULL ) {

			if ( strcmp(buf, CONTROL_STR) == 0 ) {

				for ( ilines = 0; ilines < RETURN_LINES; 
					ilines++ ) {

					if ( fgets(buf, 
					(int)sizeof(buf), inputf) == NULL )
						break;

					fputs(buf, mail);
					ilines++;
				}

/*Print a continuation indication if there's more.  */

				if ( !feof(inputf) && 
				fgets(buf, (int)sizeof(buf), inputf) != NULL )
					fputs("...more...\n", mail);

				break;
			}
		}
		if ( ilines == -1 ) {
			/*
			 * "BATCH: Start of user input" line not seen.
			 * Maybe it was cancelled.
			 * Take a look at the first 2 lines.
			 */

			rewind(inputf);
			if ( fgets(  buf, (int)sizeof(buf), inputf) != NULL
			 &&  strcmp( buf, CANCEL_FIRSTLINE ) == 0
			 &&  fgets(  buf, (int)sizeof(buf), inputf) != NULL
			 &&  strncmp(buf, CANCEL_SECONDLINE,
			     strlen(CANCEL_SECONDLINE)) == NULL ) {
				/* The cancel message looks good */
				fputs( buf, mail );	
			} else {
				fputs(
		"(Mysterious command file - no input seen.)\n", 
			mail);

			}
		} else if ( ilines == 0 ) {
			/*
			 * We saw the control message, but no input lines.
			 * Presumably a null script.
			 */
			fprintf (mail, "(No input)\n");
		}
		fputs("\n", mail);
		fclose(inputf);
	}
	if (outstat == -1)
		fprintf(mail, "Can't stat output file %s\n", fname);

	else if (sb.st_size > 0) {

		fprintf(mail, "Output %sfollows:\n\n", mailstat & MAIL_CRASH?
			"to crash point ": "");

		if ((output = open(fname, 0)) == -1) {
			mperror1("Can't open(%s) output file", fname);
			fprintf(mail,
				"BATCHD can't read output data from '%s'\n",
				fname);
		} else {
			char *buffer = mymalloc(MAXBSIZE);

			(void) fflush(mail);
			while ((n = read(output, buffer, MAXBSIZE)) > 0)
				write(fileno(mail), buffer, n);
			close(output);
			free(buffer);
		}
	}
	if (unlink(fname) < 0)
		mperror1("Can't unlink(%s)", fname);

	(void)unlink(pgrpname);
	mailclose(mail);
}


/*
 *  Abort a particular job.  Use extreme prejudice.
 *  Note that we count on wait getting the process back later.
 */

static int abortjob(jp)
struct job *jp;
{


	return( sigjob(jp, SIGKILL) );
}

/*
 *  Abort all jobs in a particular queue and drain it so no new jobs start.
 */

static void abortall(qp)
struct jobqueue *qp;
{
	int numkilled;


	drainqueue(qp);		/* stop further jobs from starting */
	
	numkilled = sigalljobs(qp, SIGKILL);

#ifdef	COMMENT
	muser3( qp->q_supervisor,
		"%s: Queue aborted; jobs=%d, jobs killed=%d\n",
		qp->q_name, qp->q_nexec, numkilled );
#endif /* COMMENT */

	queuestat1(qp,"Aborted; killed=%d\n", numkilled);
}

/*
 * Send a specified signal to all executing jobs in a queue.
 * Don't try to signal jobs that haven't started running.
 */

static int sigalljobs(qp, sig)
struct jobqueue *qp;
{
	struct job *jp;
	int n = 0;


	FOREACH (jp, qp->q_jobs) {

		if ( jp->j_pid > 0 )
			n += sigjob(jp, sig);

	}

	return n;
}


/*
 * Send a specified signal to a job process group.
 * We arranged that each job is in its own
 * process group, so we can send the signal
 * to all processes that are part of the job.
 * Return 1 if we successfully killed the job.
 */

static int sigjob(jp, sig)
struct job *jp;
{


	if ( jp->j_pid <= 1 ) {

		merror3("Job '%s/%s' has invalid pid %d\n",
			(jp->j_queue && jp->j_queue->q_cfdir)
				? jp->j_queue->q_cfdir : "???",
			jp->j_cfname ? jp->j_cfname : "???",
			jp->j_pid );
		return 0;
	}
	/* negative process id means kill the whole process group */
	return( (kill(-jp->j_pid, sig) == -1) ? 0 : 1 );
}

/*
 * Stop new jobs from starting in this queue.
 * This should be the only place, other than the profile reading,
 * where q_drain is set.
 */

static void drainqueue(qp)
struct jobqueue *qp;
{


	if (qp->q_drain == Q_DRAINING)
		return;		/* already draining */

	qp->q_drain = Q_DRAINING;
	qp->q_nochange = 0;	/* tell runqueue() about the change */

#ifdef	COMMENT
	muser3(qp->q_supervisor, "%s: %s; jobs left = %d\n",
		qp->q_name,
		qp->q_nexec ? "draining" : "drained",
		qp->q_nexec);
#endif /* COMMENT */

}


#if	(! (defined(SOLARIS) && (SOLARIS >= 8)))

struct	nlist nl[] = {

#ifdef stardent
# define unixpath "/unix"
	{ "avenrun" },
#else
#ifdef __hpux
# define unixpath "/hp-ux"
#ifdef __hppa       /* series 700 & 800 */
	{ "avenrun" },
#else               /* series 300 & 400 */
	{ "_avenrun" },
#endif
#else
# define unixpath "/vmunix"
#ifdef SUNOS5
	{ "avenrun" },
#else
	{ "_avenrun" },
#endif
#endif
#endif
	{ 0 },
} ;

#endif /* not SOLARIS-8 system */


#if defined(__osf__)

static void getload(a)
int *a;
{
	struct tbl_loadavg la;


	if (table(TBL_LOADAVG,0,(char *)&la,1,sizeof(la)) < 0) {

		merror("table() failed\n");
		a[0] = a[1] = a[2] = 0;

	} else {

		a[0] = (int) ((float)la.tl_avenrun.l[0]/(float)la.tl_lscale);
		a[1] = (int) ((float)la.tl_avenrun.l[1]/(float)la.tl_lscale);
		a[2] = (int) ((float)la.tl_avenrun.l[2]/(float)la.tl_lscale);
	}
}

#else /* __osf__ */

#ifndef RISCos

#if	defined(SOLARIS) && (SOLARIS >= 8) && CF_UGETLOADAVG

static void getload(a)
int	a[] ;
{
	uint	la[3] ;

	int	rs, i ;


	rs = u_getloadavg(la,3) ;

	for (i = 0 ; i < 3 ; i += 1)
		a[i] = (rs >= 0) ? (la[i] / FSCALE) : 0 ;

	return ;
}
/* end subroutine (getload) */

#else

static void getload(a)
int	a[] ;
{
	static int kmem = -1;

	int i;

#ifdef SUNOS5KVM
	static kvm_t *k = NULL;
#endif

#if defined(vax) || defined(__hpux)
	double avenrun[3];
#else
	long avenrun[3];
#endif

#ifdef NOKMEM
	float aves[3];
#endif


#ifdef NOKMEM

/* Use 'uptime' output for BSD-like systems with no /dev/kmem */

	i = ugetloads(aves);

	if (i < 0) {
		merror("ugetloads failed\n");
		goto failed;
	}

	for (i = 0; i < 3; i += 1)
		a[i] = aves[i];

#else /*NOKMEM*/

#ifdef SUNOS5KBM
	if (k == NULL) {

		if ((k = kvm_open(0, 0, 0, O_RDONLY, 0)) == 0) {
			mperror("kvm_open failed");
			goto failed;
		}
		kvm_nlist(k, nl);
		if (nl[0].n_type==0) {
			merror(" No namelist\n");
			kvm_close(k); k = NULL;
			goto failed;
		}
	}

	if (kvm_read(k, nl[0].n_value, (char *)avenrun, 
		sizeof avenrun) != sizeof avenrun) {

		mperror("Can't kvm_read");
		kvm_close(k); k = NULL;
		goto failed;
	}
#else
	if (kmem == -1) {

#ifdef sgi
# include <sys/sysmp.h>
	nl[0].n_value = sysmp(MP_KERNADDR, MPKA_AVENRUN) & 0x7fffffff;
#else
		nlist(unixpath, nl);

		if (nl[0].n_type == 0) {
			merror1("%s: No namelist\n", unixpath);
			goto failed;
		}

#ifdef stardent
		nl[0].n_value &= 0x7fffffff;
#endif
#endif

		if ((kmem = open("/dev/kmem", 0)) == -1) {

			mperror("Can't open(/dev/kmem)");
			goto failed;
		}
	}

	if ( lseek(kmem, (offset_t)nl[0].n_value, 0) == -1 ) {
		mperror("Can't lseek in kmem");
		goto failed;
	}

	if (read(kmem, (char *)avenrun, sizeof(avenrun)) != sizeof(avenrun)) {

		mperror("Can't read kmem");
		goto failed;
	}
#endif /* SUNOS5KVM */

	for (i = 0; i < 3; i += 1) {

#if defined(sun) || defined(sequent)
		a[i] = avenrun[i] / FSCALE;
#else 
#ifdef sgi
		a[i] = avenrun[i] / 1024;
#else
#if defined(BSD4_2) || defined(__hpux)
		a[i] = avenrun[i];
#else 
#ifdef stardent
		a[i] = (double)avenrun[i] / (1<<16);
#else
		a[i] = avenrun[i] / 1024;
#endif /*stardent*/
#endif /*BSD4_2*/
#endif /*sgi*/
#endif /*sun*/

	} /* end for */

#endif /*NOKMEM*/

	return ;

failed:;
	a[0] = a[1] = a[2] = 0;

}
/* end subroutine (getload) */

#endif /* (SOLARIS >= 8) */

#else /*RISCos*/

#include <sys/fixpoint.h>

static void getload(a)
int *a;
{
	int i;
	static int kmem = -1;
	fix avenrun[3];


	if (kmem == -1) {

		nlist("/unix", nl);
		if (nl[0].n_type==0) {
			merror("/unix: No namelist\n");
			goto failed;
		}
		if ((kmem = open("/dev/kmem", 0)) == -1) {

			mperror("Can't open(/dev/kmem)");
			goto failed;
		}
	}
	if ( lseek(kmem, (offset_t)nl[0].n_value, 0) == -1 ){
		mperror("Can't lseek in kmem");
		goto failed;
	}
	if ( read(kmem, (char *)avenrun, sizeof(avenrun)) != sizeof(avenrun) ){
		mperror("Can't read kmem");
		goto failed;
	}
	for (i = 0; i < 3; i++)
	        a[i] = (int) FIX_TO_INT(avenrun[i]) + .5;
	return;
failed:;
	a[0] = a[1] = a[2] = 0;
}

#endif /* RISCOS */

#endif /* __osf__ */


/* allocate a new element, add it to the queue */
static struct qelem * enqueue(qhead, size)	
struct qelem *qhead;
{
	struct qelem *q;


	q = (struct qelem *) mycalloc(size);	/* init to zeroes */
	insque(q, qhead);
	return q;
}


/* return an empty queue */
static struct qelem * emptyq()		
{
	struct qelem *q;


	q = (struct qelem *) mycalloc((int)sizeof *q);
	q->q_forw = q->q_back = q;
	return q;
}


#ifdef __hpux
	char *
#else
	static char *
#endif
mymalloc(n)
{
	char *p;


	if ((p = malloc((unsigned)n)) == NULL)
		error("Out of memory\n");
	return p;
}


static char * mycalloc(n)
int n;
{
	char *p;


	if ((p = calloc(1,(unsigned)n)) == NULL)
		error("Out of memory\n");
	return p;
}


static char * myhostname()
{
	static char h[256];


	if (h[0] == 0)
		getournodename(h, sizeof h - 1);

	return h;
}


#if defined(BSD4_2)

#if defined(sun) || defined(__hpux)
#include <sys/vfs.h>
#else
#ifdef sgi
#include <mntent.h>
#else
#ifndef apollo
#ifndef stardent
#if !defined(__osf__)
#include <sys/fs.h>
#endif
#include <fstab.h>
#endif
#endif
#endif
#endif


static fdev getfs(file)
char *file;
{
	int fsdev;

#if defined(sun) || defined(apollo) || defined(stardent) || defined(__hpux)
	if ((fsdev = open(file, O_RDONLY, 0)) < 0)
		mperror1("Can't open(%s) for reading", file);
	return fsdev;
#else
#ifdef sgi
	struct mntent *mnt;
	FILE *fp;
	struct ustat stbuf, stb;

	if (stat(file, &stbuf) < 0) {
		mperror1("Can't stat(%s)", file);
		return -1;
	}
	if ((fp = setmntent("/etc/fstab", "r")) == NULL) {
		mperror1("Can't read %s", "/etc/fstab");
		return -1;
	}
	while (mnt = getmntent(fp)) {
		if (stat(mnt->mnt_fsname, &stb) == 0 &&
		    stb.st_rdev == stbuf.st_rdev) {
			endmntent(fp);
			if ((fsdev = open(mnt->mnt_fsname, O_RDONLY)) < 0) {
				mperror1(
				"Can't open(%s) file system for reading",
					mnt->mnt_fsname);
				return -1;
			}
			return fsdev;
		}
	}
	endmntent(fp);
#else
	struct fstab *fsp;
	struct ustat stbuf, stb;

	if (stat(file, &stbuf) < 0) {
		mperror1("Can't stat(%s)", file);
		return -1;
	}
	setfsent();
	while ((fsp = getfsent()) != NULL) {
		if (stat(fsp->fs_spec, &stb) == 0 &&
		    stb.st_rdev == stbuf.st_dev) {
			endfsent();
			if ((fsdev = open(fsp->fs_spec, O_RDONLY)) < 0) {
				mperror1(
				"Can't open(%s) file system for reading",
					fsp->fs_spec);
				return -1;
			}
			return fsdev;
		}
	}
	endfsent();
#endif /*sgi*/
	merror1("%s: located on unknown device\n", file);
	return -1;
#endif /*sun*/
}

	static long
fsfree(dev)
	int dev;
{
#if defined(__osf__)
	struct ustatfs f;

	if (fstatfs(dev, &f, sizeof(f)) < 0) {
		merror1("fstatfs: %s\n", syserr());
		return 0;
	}
	return f.f_bavail*f.f_bsize/1024;
#else /* __osf__ */
#if defined(sun) || defined(__hpux)
# ifdef SUNOS5
	struct statvfs64 f;

	if (fstatvfs64(dev, &f) < 0) {
		merror1("fstatvfs: %s\n", syserr());
		return 0;
	}
	return f.f_bavail*f.f_bsize/1024;
# else
	struct ustatfs64 f;

	if (fstatfs64(dev, &f) < 0) {
		merror1("fstatfs: %s\n", syserr());
		return 0;
	}
	return f.f_bavail*f.f_bsize/1024;
# endif /* SUNOS5 */
#else
#ifdef sgi
#include <sys/statfs.h>
	struct ustatfs f;

	if (fstatfs(dev, &f, sizeof f, 0) < 0) {
		merror1("fstatfs: %s\n", syserr());
		return 0;
	}
	return (f.f_blocks-f.f_bfree)*f.f_bsize/1024;
#else
#ifdef apollo
#include <sys/statfs.h>
/* Correct Apollo DN10000 brain-damage: f_bfree is return in kB, not blocks */
#if (_ISP__A88K == 1)
#define APOLLOFACTOR 4
#else
#define APOLLOFACTOR 1
#endif
        struct ustatfs f;


        if (fstatfs(dev, &f, sizeof f, 0) < 0) {
                merror1("fstatfs: %s\n", syserr());
                return 0;
        }
        return ((float)f.f_bfree/APOLLOFACTOR)*f.f_bsize/1024;
#else
#ifdef stardent
#include <sys/statfs.h>
        struct ustatfs f;

        if (fstatfs(dev, &f, sizeof f, 0) < 0) {
                merror1("fstatfs: %s\n", syserr());
                return 0;
        }
        return f.f_bfree*f.f_bsize/1024;
#else

	struct fs sblock;
	long freesp;

	(void) lseek(dev, (offset_t)(SBLOCK * DEV_BSIZE), 0);
	if (read(dev, (char *)&sblock, (int)sizeof sblock) != sizeof sblock) {
		merror1("Superblock read error: %s\n", syserr());
		return 0;
	}
	freesp = freespace(&sblock, sblock.fs_minfree);
	if (freesp > 0)
		return freesp * sblock.fs_fsize / 1024;
	return 0;
#endif /*stardent*/
#endif /*apollo*/
#endif /*sgi*/
#endif /*sun*/
#endif /*__osf__*/
}


static void releasedev(dev)
int dev;
{
	if (dev > 0)
		close(dev);
}

#else /*BSD4_2*/

#include <sys/param.h>	/* for BBSIZE */


static fdev getfs(file)
char *file;
{
	struct ustat statb;


	if (stat(file, &statb) < 0) {
		mperror1("Can't stat(%s)", file);
		return -1;
	}
	return statb.st_dev;
}


static long fsfree(dev)
dev_t dev;
{
	struct ustat ustatb;

	int	pagesize = getpagesize() ;

	char str[STRSIZ];


	if (ustat(dev, &ustatb) < 0) {
		sprintf(str, "ustat of %d/%d: %s\n", (dev>>8)&0xff, dev&0xff,
		    syserr());
		merror(str);
		return 0;
	} else
		return (ustatb.f_tfree * pagesize/ 1024) ;

}
/* end subroutine (fsfree) */


/*ARGSUSED*/

static void releasedev(dev)
dev_t dev;
{

}

#endif /*BSD4_2*/

#if	((! defined(SUNOS5)) && (! defined(BSD4_2))) || defined(__hpux)

static insque(elem, pred)
struct qelem *elem, *pred;
{
	elem->q_forw = pred->q_forw;
	elem->q_back = pred;
	pred->q_forw->q_back = elem;
	pred->q_forw = elem;
}


static remque(elem)
struct qelem *elem;
{
	elem->q_back->q_forw = elem->q_forw;
	elem->q_forw->q_back = elem->q_back;
	/* return elem; */
}

#endif /*BSD4_2*/


/*
 * Return non-zero if the time `tp' is within
 * the given time specifier.  This function just
 * breaks the specifier into comma-separated elements,
 * calls check1time on each, and returns true if any of them match.
 */

checktime(p, tp)
	char *p;
	struct tm *tp;
{
	char *q;
	int i;


	while (p) {

		if ((q = strchr(p, ',')) != NULL)
			*q = 0;
		i = check1time(p, tp);
		if (q != NULL)
			*q++ = ',';
		if (i)
			return i;
		p = q;
	}
	return 0;
}


check1time(p, tp)
	char *p;
	struct tm *tp;
{
	int i, tl, th, tn, dayok=0;
	static struct {
		char *str;
		enum { Wk, Night, Any, Evening } tok;
	} t[] = {
		"Wk", Wk,
		"Night", Night,
		"Any", Any,
		"Evening", Evening,
	};


	while (isspace(*p))
		p++;

	if (strncmp(p, "SuMoTuWeThFrSa"+tp->tm_wday*2, 2) == 0)
		dayok = 1, p += 2;

	else for (i = 0; i < sizeof t/sizeof t[0]; i++) {

		if (strncmp(p, t[i].str, strlen(t[i].str)) != 0)
			continue;

		p += strlen(t[i].str);

		switch (t[i].tok) {

		case Wk:
			if (tp->tm_wday >= 1 && tp->tm_wday <= 5)
				dayok = 1;
			break;
		case Night:
			if (tp->tm_wday == 6 || /* Sat */
			    tp->tm_hour >= 23 || tp->tm_hour < 8 ||
					/* Sunday before 5pm */
			    (tp->tm_wday == 0 && tp->tm_hour < 17))
				dayok = 1;
			break;
		case Any:
			dayok = 1;
			break;
		case Evening:
			/* Sat or Sun */
			if (tp->tm_wday == 6 || tp->tm_wday == 0 ||
			    tp->tm_hour >= 17 || tp->tm_hour < 8)
				dayok = 1;
			break;
		}

		break;
	}

	if (sscanf(p, "%d-%d", &tl, &th) != 2)
		return dayok;

	tn = tp->tm_hour * 100 + tp->tm_min;

  	if (th < tl) { 		/* crosses midnight */

  		if (tl <= tn || tn < th)
  			return 1;

  	} else {

		if (tl <= tn && tn < th)
			return 1;

	}

	return 0;
}

#ifdef SUNOS5

getdtablesize()
{
	struct rlimit rlim;


	getrlimit(RLIMIT_NOFILE, &rlim);
	return rlim.rlim_cur;
}

static int getournodename(p, n)
char	*p;
int	n ;
{
	struct utsname u;

	uname(&u);
	strncpy(p, u.nodename, n);
	return 0;
}

#endif /* SUNOS5 */



