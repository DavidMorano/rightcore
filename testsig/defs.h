/* defs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>

#include	<vechand.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<paramopt.h>
#include	<fsdirtree.h>
#include	<vecpstr.h>
#include	<hdbstr.h>
#include	<hdb.h>
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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	MEGABYTE	(1024 * 1024)
#define	UNIXBLOCK	512

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		tmpdir:1 ;
	uint		logprog:1 ;
	uint		logsize:1 ;
	uint		nochange:1 ;
	uint		print:1 ;
	uint		follow:1 ;	/* follow symbolic links */
	uint		tardirs:1 ;
	uint		nice:1 ;	/* nice value */
	uint		ff:1 ;		/* first-follow */
	uint		iacc:1 ;	/* ignore no-access */
	uint		im:1 ;		/* ignore missing */
	uint		zargs:1 ;	/* allow zero arguments */
	uint		younger:1 ;
	uint		sufreq:1 ;
	uint		sufacc:1 ;
	uint		sufrej:1 ;
	uint		nostop:1 ;
	uint		cores:1 ;	/* specify core files */
	uint		rmfile:1 ;
	uint		rmdirs:1 ;
	uint		dirs:1 ;	/* dir-id management */
	uint		files:1 ;	/* file-id management */
	uint		links:1 ;	/* file-link management */
	uint		readable:1 ;	/* readable */
	uint		older:1 ;	/* older */
	uint		accessed:1 ;	/* accessed */
	uint		prune:1 ;	/* prune-mode */
	uint		summary:1 ;	/* summary mode (for lines) */
	uint		ignasscomm:1 ;	/* ignore assembly comments */
	uint		igncomm:1 ;	/* ignore comments w/ character */
	uint		f_uniq:1 ;
	uint		f_name:1 ;
	uint		f_prog:1 ;
	uint		f_noprog:1 ;
	uint		f_nopipe:1 ;
	uint		f_nodev:1 ;
	uint		f_noname:1 ;
	uint		f_nolink:1 ;
	uint		f_nosock:1 ;
	uint		f_nodoor:1 ;
	uint		f_noextra:1 ;
	uint		f_nodotdir:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;
	const char	*progdname ;
	const char	*progname ;
	const char	*pr ;
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*username ;
	const char	*groupname ;
	const char	*nodename ;
	const char	*domainname ;
	cchar		*gecosname ;
	cchar		*realname ;
	cchar		*name ;
	cchar		*fullname ;
	cchar		*mailname ;
	cchar		*org ;
	cchar		*logid ;
	const char	*homedname ;
	cchar		*hostname ;
	const char	*tmpdname ;
	const char	*tardname ;
	const char	*jobdname ;
	const char	*lfname ;
	const char	*hfname ;
	cchar		**prune ;
	void		*lip ;
	void		*efp ;
	void		*ofp ;
	void		*userlist ;	/* user-list state */
	void		*config ;	/* configuration */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	PARAMOPT	aparams ;
	FSDIRTREE_STAT	tarstat ;
	LOGFILE		lh ;
	VECHAND		tardirs ;
	VECPSTR		sufs[3] ;
	VECPSTR		rmdirs ;
	HDBSTR		dirnames ;
	HDB		dirs ;		/* dir-id management */
	HDB		files ;		/* file-id management */
	HDB		links ;		/* link-something management */
	dev_t		tardev ;	/* target directory device (linker) */
	time_t		daytime ;
	uid_t		uid, euid, uid_tmp ;
	gid_t		gid, egid, gid_tmp ;
	pid_t		pid ;
	uint		bytes ;
	uint		megabytes ;
	uint		fts ;		/* file types specified */
	uint		fnos ;		/* file "nos" */
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		namelen ;
	int		younger ;
	int		older ;
	int		accessed ;
	int		nice ;
	int		cch ;		/* comment character */
	int		n ;
	int		c_files ;
	int		c_processed ;
	int		c_linkerr ;
	int		c_lines ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	const char	**argv ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(PROGINFO *,cchar **,cchar *,cchar *) ;
extern int proginfo_setentry(PROGINFO *,const char **,const char *,int) ;
extern int proginfo_setversion(PROGINFO *,const char *) ;
extern int proginfo_setbanner(PROGINFO *,const char *) ;
extern int proginfo_setsearchname(PROGINFO *,const char *,const char *) ;
extern int proginfo_setprogname(PROGINFO *,const char *) ;
extern int proginfo_setexecname(PROGINFO *,const char *) ;
extern int proginfo_setprogroot(PROGINFO *,const char *,int) ;
extern int proginfo_pwd(PROGINFO *) ;
extern int proginfo_rootname(PROGINFO *) ;
extern int proginfo_progdname(PROGINFO *) ;
extern int proginfo_progename(PROGINFO *) ;
extern int proginfo_nodename(PROGINFO *) ;
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_getename(PROGINFO *,char *,int) ;
extern int proginfo_getenv(PROGINFO *,const char *,int,const char **) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


