/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<fsdirtree.h>
#include	<vecpstr.h>
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

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#define	MEGABYTE	(1024 * 1024)
#define	UNIXBLOCK	512


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		log:1 ;
	uint		nochange:1 ;
	uint		print:1 ;
	uint		follow:1 ;	/* follow symbolic links */
	uint		iacc:1 ;	/* ignore no-access */
	uint		nice:1 ;	/* nice value */
	uint		zargs:1 ;	/* allow zero arguments */
	uint		tardname:1 ;
	uint		younger:1 ;
	uint		sufreq:1 ;
	uint		sufacc:1 ;
	uint		sufrej:1 ;
	uint		nostop:1 ;
	uint		cores:1 ;	/* specify core files */
	uint		links:1 ;	/* file-link management */
	uint		f_uniq:1 ;
	uint		f_name:1 ;
	uint		f_noprog:1 ;
	uint		f_nopipe:1 ;
	uint		f_nodev:1 ;
	uint		f_noname:1 ;
	uint		f_nolink:1 ;
	uint		f_nosock:1 ;
	uint		f_nodoor:1 ;
	uint		f_noextra:1 ;
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
	const char	*tmpdname ;
	const char	*helpfname ;
	const char	*tardname ;
	void		*efp ;
	void		*ofp ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	FSDIRTREE_STAT	tarstat ;
	LOGFILE		lh ;
	VECPSTR		sufs[3] ;
	HDB		links ;
	time_t		daytime ;
	uid_t		uid, euid ;
	uint		bytes ;
	uint		megabytes ;
	uint		fts ;		/* file types specified */
	uint		fnos ;		/* file "nos" */
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		namelen ;
	int		younger ;
	int		nice ;
	int		c_files ;
	int		c_processed ;
	int		c_linkerr ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,const char *,
		const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_proedname(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


