/*
 * bat_common.h
 *
 * Common include file for batch programs.
 * Expects BSD4_2 to be defined if running 4.[23] BSD.
 *
 * This must be included *after* all the other system include files,
 * so we can invent the stuff not already defined.
 */


#ifdef OLD_DIR
#include <sys/dir.h>
#define struct_dir struct direct
#else
#include <dirent.h>
#define struct_dir struct dirent
#endif /* OLD_DIR */


typedef double DOUBLE;

#ifdef BSD4_2
#undef DIRSIZ
#define	DIRSIZ	32	/* Used to be 14; could be MAXNAMLEN... -IAN! */
typedef int fdev;
#else /*BSD4_2*/
typedef dev_t fdev;
#endif /*BSD4_2*/

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef MAXBSIZE
#define MAXBSIZE	4096	/* correct for IRIS */
#endif

#ifndef RLIM_NLIMITS
#define RLIM_NLIMITS	6	/* Number of resources, eg RLIMIT_CPU */
#endif

#ifndef R_OK
# ifdef SYSVR4
#  include <unistd.h>
# else
#  include <sys/file.h>	/* Not needed in 4.2 for some reason. */
# endif
#endif
#ifndef R_OK
#define	R_OK	04
#endif

/*
 * What the first bit of a cancelled job looks like.
 */
#ifdef apollo
#define	CANCEL_FIRSTLINE	"#!/bin/sh\n"
#define	CANCEL_SECONDLINE	"echo Job cancelled by"
#define	CANCEL_THIRDLINE	"sleep 60\n"
#else /*apollo*/
#define	CANCEL_FIRSTLINE	"#!/bin/cat\n"
#define	CANCEL_SECONDLINE	"Job cancelled by"
#endif

#if defined(BSD4_2) && !defined(__hpux)
extern char	*sys_siglist[NSIG];
#else /*BSD4_2*/
static char	*sys_siglist[NSIG] = {
	"Signal 0",
	"Hangup",			/* SIGHUP */
	"Interrupt",			/* SIGINT */
	"Quit",				/* SIGQUIT */
	"Illegal instruction",		/* SIGILL */
	"Trace/BPT trap",		/* SIGTRAP */
	"IOT trap",			/* SIGIOT */
	"EMT trap",			/* SIGEMT */
	"Floating point exception",	/* SIGjpE */
	"Killed",			/* SIGKILL */
	"Bus error",			/* SIGBUS */
	"Segmentation fault",		/* SIGSEGV */
	"Bad system call",		/* SIGSYS */
	"Broken pipe",			/* SIGPIPE */
	"Alarm clock",			/* SIGALRM */
	"Terminated",			/* SIGTERM */
	"User-defined signal 1",	/* SIGUSR1 */
	"User-defined signal 2",	/* SIGUSR2 */
	"Child exited",			/* SIGCLD */
	"Power fail",			/* SIGPWR */
	"Signal 20",
	"Signal 21",
	"Signal 22",
	"Signal 23",
	"Signal 24",
	"Signal 25",
	"Signal 26",
	"Signal 27",
	"Signal 28",
	"Signal 29",
	"Signal 30",
#ifndef __hp9000s800
	"Signal 31"
#endif
};
#endif /*BSD4_2*/


#define	Q_QUEUESTAT		"queuestat"	/* queue status file */
#define	Q_PROFILE		"profile"	/* name of prifile file */
#define	Q_CFDIR			"CFDIR"		/* directory for cf* files */

#define BATCH_MAIL_USERID	"submit"	/* for system error mail */
#define DEV_NULL		"/dev/null"	/* null q_supervisor */

#define Q_STARTNEW	0	/* "exec on" in profile; start new jobs */
#define Q_DRAINING	1	/* "exec off" or "exec drain" in profile */

/* the length of the status prefix */
#define BAT_PREFIX_LEN          2

/* the maximum length of a user-id as defined by utmp */
#define BAT_MAXUID sizeof(((struct utmp *)NULL)->ut_name)

/* startup configuration for queues */
#define BAT_INICE	0	/* initial nice value */
#define BAT_IMAXEXEC	1	/* initial number of running jobs */
#define BAT_ILOADSCHED	25	/* initial load average to schedule jobs */
#define BAT_ILOADSTOP	50	/* initial load average to stop running jobs */
#define BAT_IMINFREE	0	/* initial min. number of free devices */

/*
 * the following #defines are used in batch.c
 */
#define BAT_SH		"/bin/sh"	/* real Bourne shell */
#define BAT_CSHELL	"csh"		/* strncmp for csh or *tcsh */

#define STRSIZ	256		/* sprintf string buffers */

typedef char bat_Mail;
#define MAIL_START	(1<<0)
#define MAIL_END	(1<<1)
#define MAIL_CRASH	(1<<2)

#define CONTROL_STR     "# BATCH: Start of User Input\n" 
#define RETURN_LINES	5

/*
 * Pathnames
 */

/* The next two are defined in Makefile */
/* #define SPOOLDIR	"/usr/spool/batch"	/* main spool directory */
/* #define PIDFILE	"/usr/spool/pid/batchd"	/* lock file for our pid */

#ifndef	SPOOLDIR
#define	SPOOLDIR	"/var/spool/jobsub"
#endif

#ifndef	PIDDIR
#define	PIDDIR		"/var/run"
#endif

#define DEBUGFILE	"/tmp/batchd.debug"	/* debug log file */
#define SLEEPTIME	(1*60)			/* idle timer between jobs */

/*
 * Default PATH; used to keep shells happy when they start up,
 * before PATH is set in the queued batch file itself.
 */
#if	defined(SUNOS5)
#define DEFPATH		"/usr/bin:/usr/extra/bin:/usr/extra/sbin"
#else
#define DEFPATH		"/usr/ucb:/bin:/usr/bin"
#endif


/*
 * Handy macros to simplify varargs error messages.
 * Assumes a global char errstr[STRSIZ];
 * Not re-entrant!
 */

#define error1(fmt,a1)		{ sprintf(errstr,fmt,a1); error(errstr); }
#define error2(fmt,a1,a2)	{ sprintf(errstr,fmt,a1,a2); error(errstr); }
#define error3(fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); error(errstr); }
#define error4(fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); error(errstr); }
#define syserror1(fmt,a1)	{ sprintf(errstr,fmt,a1); syserror(errstr); }
#define syserror2(fmt,a1,a2)	{ sprintf(errstr,fmt,a1,a2); syserror(errstr); }
#define syserror3(fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); syserror(errstr); }
#define syserror4(fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); syserror(errstr); }
#define syswarn1(fmt,a1)	{ sprintf(errstr,fmt,a1); syswarn(errstr); }
#define syswarn2(fmt,a1,a2)	{ sprintf(errstr,fmt,a1,a2); syswarn(errstr); }
#define syswarn3(fmt,a1,a2,a3) \
	{ sprintf(errstr,fmt,a1,a2,a3); syswarn(errstr); }
#define syswarn4(fmt,a1,a2,a3,a4) \
	{ sprintf(errstr,fmt,a1,a2,a3,a4); syswarn(errstr); }



