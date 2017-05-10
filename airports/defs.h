/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<dater.h>
#include	<paramfile.h>
#include	<sockaddress.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<lfm.h>

#include	"localmisc.h"



#ifndef	nelem
#ifdef	nelements
#define	nelem		nelements
#else
#define	nelem(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif

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

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	STDFNAMEIN	"*STDIN*"
#define	STDFNAMEOUT	"*STDOUT*"
#define	STDFNAMEERR	"*STDERR*"

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		100
#define	VBUFLEN		(2 * MAXPATHLEN)
#define	EBUFLEN		(3 * MAXPATHLEN)





struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	quiet : 1 ;
	uint	pidlock : 1 ;
	uint	daemon : 1 ;
	uint	log : 1 ;
	uint	speed : 1 ;
	uint	zerospeed : 1 ;
	uint	nodeonly : 1 ;
	uint	tmpdate : 1 ;
	uint	disable : 1 ;
	uint	all : 1 ;
	uint	def : 1 ;
	uint	list : 1 ;
	uint	jobid : 1 ;
	uint	configfile : 1 ;
	uint	reqfile : 1 ;
	uint	lockfile : 1 ;
	uint	pidfile : 1 ;
	uint	logfile : 1 ;
	uint	slfile : 1 ;
	uint	ddir : 1 ;
	uint	pollint : 1 ;
	uint	lockint : 1 ;
	uint	markint : 1 ;
	uint	runint : 1 ;
	uint	config : 1 ;
	uint	background : 1 ;
} ;

struct proginfo_config {
	PARAMFILE	p ;
	EXPCOOK	cooks ;
	uint		f_p ;
} ;

struct proginfo_client {
	SOCKADDRESS	sa ;
	int		fd ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progename ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*version ;
	char		*banner ;
	char		*searchname ;
	char		*helpfname ;
	char		*tmpdname ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*queuename ;
	char		*jobname ;
	char		*mailaddr ;
	void		*efp ;
	struct proginfo_flags	f ;
	struct proginfo_flags	have ;
	struct proginfo_flags	final ;
	struct proginfo_flags	open ;
	struct proginfo_flags	change ;
	struct proginfo_config	config ;
	LOGFILE		lh ;
	LFM		lockfile, pidfile ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		uid ;
	gid_t		gid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		nice ;
	int		loglen ;
	int		pollint ;
	int		markint ;
	int		lockint ;
	int		pidint ;
	int		runint ;
	int		disint ;
	int		fd_req ;
	int		fd_listentcp ;
	int		fd_listenpass ;
	char		logid[LOGIDLEN + 1] ;
	char		cmd[LOGIDLEN + 1] ;
	char		spooldname[MAXNAMELEN + 1] ;
	char		ddname[MAXNAMELEN + 1] ;
	char		logfname[MAXNAMELEN + 1] ;
	char		pidfname[MAXNAMELEN + 1] ;
	char		reqfname[MAXNAMELEN + 1] ;
	char		slfname[MAXNAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;


extern int	progconfig_init(struct proginfo *,const char *) ;
extern int	progconfig_check(struct proginfo *) ;
extern int	progconfig_read(struct proginfo *) ;
extern int	peogconfig_free(struct proginfo *) ;


#endif /* DEFS_INCLUDE */



