/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<dater.h>
#include	<ids.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<localmisc.h>


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

#define	STDINFNAME	"*STDIN*"
#define	STDOUTFNAME	"*STDOUT*"
#define	STDERRFNAME	"*STDERR*"

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
	uint	progdash : 1 ;
	uint	akopts : 1 ;
	uint	aparams : 1 ;
	uint	quiet : 1 ;
	uint	outfile : 1 ;
	uint	errfile : 1 ;
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
	uint	pidfile : 1 ;
	uint	logfile : 1 ;
	uint	server : 1 ;
	uint	mailer : 1 ;
	uint	msfile : 1 ;
	uint	speedint : 1 ;
	uint	pollint : 1 ;
	uint	lockint : 1 ;
	uint	markint : 1 ;
	uint	runint : 1 ;
	uint	config : 1 ;
	uint	spooldir : 1 ;
} ;

struct proginfo_config {
	PARAMFILE	p ;
	EXPCOOK	cooks ;
	uint		f_p ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progename ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*tmpdname ;
	char		*serverefname ;
	char		*queuename ;
	char		*jobname ;
	char		*mailaddr ;
	char		*mailaddr_admin ;
	char		*prog_server ;
	char		*prog_mailer ;
	void		*efp ;
	struct proginfo_flags	f, have, changed, final ;
	struct proginfo_flags	open ;
	struct proginfo_config	config ;
	LOGFILE		lh ;
	IDS		ids ;
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
	int		speedint ;
	int		runint ;
	int		disint ;
	char		jobgrade[4] ;
	char		logid[LOGIDLEN + 1] ;
	char		cmd[LOGIDLEN + 1] ;
	char		spooldname[MAXNAMELEN + 1] ;
	char		speedname[MAXNAMELEN + 1] ;
	char		logfname[MAXNAMELEN + 1] ;
	char		pidfname[MAXNAMELEN + 1] ;
	char		reqfname[MAXNAMELEN + 1] ;
	char		msfname[MAXNAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	progconfig_init(struct proginfo *,const char *) ;
extern int	progconfig_check(struct proginfo *) ;
extern int	progconfig_read(struct proginfo *) ;
extern int	peogconfig_free(struct proginfo *) ;

extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


