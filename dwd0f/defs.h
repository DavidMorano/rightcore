/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	LOGFILE_LOGIDLEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		MAX((5 * 1024),MAXPATHLEN)

#define	NLENV		40
#define	ENVLEN		2000

#define	CMDBUFLEN	(2 * MAXPATHLEN)

#define	MAXSLEEPTIME	270	/* must not be longer than 2min 30sec */
#define	LOCKTIMEOUT	300	/* lock file timeout (5min) */


/* program exit codes */

#define	PRS_USAGE	1
#define	PRS_OK		0
#define	PRS_BAD		-1
#define	PRS_BADFORK	-2
#define	PRS_BADARGS	-3
#define	PRS_BADLOG	-4
#define	PRS_BADOPT	-5
#define	PRS_BADARG	-6
#define	PRS_BADNUM	-7
#define	PRS_BADEXTRA	-8
#define	PRS_BADVALUE	-9
#define	PRS_BADWORKING	-10
#define	PRS_BADQUEUE	-11
#define	PRS_BADUSER	-12
#define	PRS_BADINT	-13
#define	PRS_BADSRV	-14
#define	PRS_NOSRV	-15
#define	PRS_BADDIR	-16
#define	PRS_BADCONFIG	-17
#define	PRS_BADLOCK	-18
#define	PRS_BADPIDOPEN	-19
#define	PRS_BADPIDLOCK	-20
#define	PRS_BADLOCK2	-21
#define	PRS_BADPID2	-22


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;	/* we have STDERR */
	uint		log:1 ;
	uint		srvtab:1 ;	/* we have a service table file */
	uint		interrupt:1 ;	/* we-re supposed to have INT file */
	uint		poll:1 ;	/* poll only mode */
	uint		lockfile:1 ;
	uint		pidfile:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* program directory */
	const char	*progdname ;	/* program directory */
	const char	*progname ;	/* program name */
	const char	*pr ;		/* program root directory */
	const char	*searchname ;	/* program search name */
	const char	*version ;	/* program version string */
	const char	*banner ;
	const char	*nodename ;	/* machine node name */
	const char	*domainname ;	/* INET domain name */
	const char	*username ;
	const char	*groupname ;
	const char	*logid ;
	const char	*command ;
	const char	*srvtab ;	/* service file name */
	const char	*directory ;	/* directory to watch */
	const char	*interrupt ;
	const char	*tmpdname ;	/* temporary directory */
	const char	*workdname ;
	const char	*lockfname ;	/* lock file */
	const char	*pidfname ;	/* mutex lock file */
	void		*efp ;
	void		*lfp ;		/* system log "Basic" file */
	void		*lockfp ;	/* lock file BIO FP */
	vecstr		exports ;
	vecstr		paths ;
	LFM		pider ;
	logfile		lh ;		/* program activity log */
	logfile		eh ;		/* error log */
	struct proginfo_flags	have, f, changed, final ;
	pid_t		pid ;
	uid_t		uid ;		/* real UID */
	uid_t		euid ;		/* effective UID */
	time_t		daytime ;
	VOLATILE int	f_exit ;
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;	/* verbosity level */
	int		pollmodetime ;
	int		polltime ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;


/* job states */

#define	STATE_WATCH	0		/* just entered system */
#define	STATE_WAIT	1		/* resource wait */
#define	STATE_STARTED	2		/* execution has been started */
#define	STATE_STALE	3		/* no service entry for job */


struct jobentry {
	offset_t	size ;
	time_t		daytime ;	/* daytime at start of job */
	time_t		stime ;		/* initial creation time */
	time_t		mtime ;
	pid_t		pid ;
	int		state ;
	char		filename[MAXNAMELEN + 1] ;
	char		ofname[MAXNAMELEN + 1] ;
	char		efname[MAXNAMELEN + 1] ;
	char		logid[LOGFILE_LOGIDLEN + 1] ;
} ;


/*************************************************************************

#	The following substitutions are made on service daemon invocations:
#
#		%f	file name
#		%j	job name
#		%s	service name
#		%d	directory path
#		%h	remote hostname
#		%a	service arguments
#		%n	current node name
#		%u	username
#

**************************************************************************/

struct expand {
	char	*a ;
	char	*s ;
	char	*f ;
	char	*m ;
	char	*d ;
	char	*l ;
	char	*c ;
	char	*w ;
	char	*r ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,const char *,
		const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,const char *,int,const char **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


