/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	INC_DEFS
#define	INC_DEFS	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>

#include	"jobdb.h"


#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#define	CMDBUFLEN	(2 * MAXPATHLEN)
#define	SRVARGLEN	(8 * 1024)

#define	BUFLEN		(5 * 1024)
#define	NLENV		40
#define	ENVLEN		2000
#define	JOBIDLEN	14
#define	QUEUESPECLEN	(MAXPATHLEN + MAXHOSTNAMELEN + 3)

#define	MAXSLEEPTIME	250	/* must not be longer than 2min 30sec */




struct proginfo_flags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	srvtab : 1 ;		/* we have a service table file */
	uint	interrupt : 1 ;		/* we're supposed to have INT file */
	uint	quiet : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	struct proginfo_flags	f ;
	VOLATILE int	f_exit ;
	logfile	lh ;		/* program activity log */
	logfile	eh ;		/* error log */
	VECSTR		exports ;
	VECSTR		paths ;
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*lockfp ;	/* BIO FP for lock file */
	bfile		*pidfp ;	/* BIO FP for PID file */
	char	*progname ;		/* program name */
	char	*programroot ;		/* program root directory */
	char	*logid ;		/* default program LOGID */
	char	*srvtab ;		/* service file name */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*nodename ;
	char	*domainname ;
	char	*username ;
	char	*groupname ;
	char	*directory ;		/* directory to watch */
	char	*interrupt ;
	char	*lockfile ;		/* lock file */
	char	*pidfile ;		/* mutex PID file */
	pid_t	pid ;
	uid_t	uid ;			/* real UID */
	uid_t	euid ;			/* effective UID */
	gid_t	gid ;
	time_t	daytime ;
	int	pwdlen ;
	int	debuglevel ;		/* debugging level */
	int	polltime ;
	int	maxjobs ;
} ;



/* job states */

#define	JOBSTATE_WATCH		0	/* just entered system */
#define	JOBSTATE_MAXJOBS	1	/* resource wait */
#define	JOBSTATE_STARTED	2	/* execution has been started */
#define	JOBSTATE_STALE		3	/* no service entry for job */
#define	JOBSTATE_ROUTING	4	/* job is being routed */
#define	JOBSTATE_TIMEWAIT	5	/* time wait */
#define	JOBSTATE_MAX		6	/* NUMBER OF STATES */



/* watch state */

struct watchstate {
	int	c[JOBSTATE_MAX] ;
	int	logid_count ;
} ;


/* the job entry structure */

struct jobentry {
	VECSTR		path, srvargs ;
	struct ema	from, error_to, sender ;
	struct job_status	**jspp ;
	offset_t	offset, size ;
	time_t	date ;			/* job date (from header) */
	time_t	atime ;			/* job arrival time */
	time_t	mtime ;			/* job modification time */
	time_t	wtime ;			/* wake-up time */
	pid_t	pid ;
	int	index ;			/* index into status table */
	int	state ;
	int	clen ;
	int	sfd ;			/* shared memory file descriptor */
	char	filename[MAXNAMELEN + 1] ;
	char	logid[15], jobid[15] ;
	char	service[MAXNAMELEN + 1] ;
	char	orig_machine[MAXHOSTNAMELEN + 1] ;
	char	orig_user[MAXUSERNAMELEN + 1] ;
	char	queuespec[QUEUESPECLEN + 1] ;
	char	ofname[MAXNAMELEN + 1] ;
	char	efname[MAXNAMELEN + 1] ;
} ;


#endif /* INC_DEFS */


