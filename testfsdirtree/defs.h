/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<logfile.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
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

#define	BUFLEN		(5 * 1024)
#define	JOBIDLEN	14


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		efile:1 ;
	uint		ofile:1 ;
	uint		log:1 ;
	uint	onckey : 1 ;		/* we have an ONC private key */
	uint	slog : 1 ;		/* system log */
	uint	daemon : 1 ;		/* are we in daemon mode ? */
	uint	pidlock : 1 ;		/* PID lock initiated ? */
	uint	named : 1 ;		/* do we have named services ? */
	uint	srvtab : 1 ;		/* do we have an server table ? */
	uint	acctab : 1 ;		/* do we have an access table ? */
	uint	path : 1 ;		/* we have a path file */
	uint	defacc : 1 ;		/* default access restrictions */
	uint	secure : 1 ;		/* all secure */
	uint	secure_root : 1 ;	/* secure root directory */
	uint	secure_conf : 1 ;	/* secure configuration file */
	uint	secure_srvtab : 1 ;
	uint	secure_acctab : 1 ;
	uint	secure_path : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;		/* environment */
	const char	*pwd ;
	const char	*progdname ;
	const char	*progename ;
	const char	*progname ;		/* program name */
	const char	*pr ;			/* program root directory */
	const char	*searchname ;		/* program search name */
	const char	*version ;		/* program version string */
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*hostname ;		/* concatenation of N + D */
	const char	*username ;		/* ours */
	const char	*groupname ;		/* ours */
	const char	*logid ;		/* default program LOGID */
	const char	*tmpdname ;		/* temporary directory */
	const char	*workdname ;
	const char	*stampdname ;		/* timestamp directory */
	const char	*pidfname ;
	const char	*lockfname ;		/* lock file */
	const char	*pathfname ;
	const char	*prog_rmail ;
	const char	*prog_sendmail ;
	const char	*defuser ;		/* default for servers */
	const char	*defgroup ;		/* default for servers */
	const char	*defacc ;		/* default access group */
	const char	*srvtab, *acctab ;	/* file names */
	bfile		*lfp ;		/* system log "Basic" file */
	bfile		*efp ;
	bfile		*lockfp ;	/* BIO FP for lock file */
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	LOGFILE		lh ;		/* program activity log */
	vecstr	exports ;		/* exports */
	vecstr	path ;			/* search path for servers */
	IDS	ids ;
	time_t	daytime ;
	pid_t	pid, ppid ;
	uid_t	uid, euid ;		/* UIDs */
	gid_t	gid, egid ;
	gid_t	gid_pcs ;
	int	pwdlen ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
	int	quietlevel ;		/* quiet level */
	int	serial ;
	int	interval ;		/* program check interval (secs) */
	int	runtime ;		/* program run time (secs) */
	int	maxjobs ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct progconfig {
	int		mincheck ;
	const char	newsdname[MAXPATHLEN + 2] ;
	const char	stampfname[MAXPATHLEN + 2] ;
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


