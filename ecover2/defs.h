/* defs INCLUDE FILE */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	BUFLEN		8192

#define	NOPWORDS	128		/* words (64 bit) per block */

#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif

#ifndef	PROGINFO_FL
#define	PROGINFO_FL	struct proginfo_flags
#endif

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo

#define	ECMSGDESC	struct ecmsgdesc

#define	FILEINFO	struct fileinfo

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
	uint		infile:1 ;
	uint		logprog:1 ;
	uint		log:1 ;
	uint		logsize:1 ;
	uint		unscramble:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progdname ;		/* program directory */
	const char	*progename ;		/* program directory */
	const char	*progname ;		/* program name */
	const char	*pr ;			/* program root directory */
	const char	*searchname ;		/* program search name */
	const char	*version ;
	const char	*banner ;	/* USERINFO */
	const char	*nodename ;	/* USERINFO */
	const char	*domainname ;	/* USERINFO */
	const char	*username ;	/* USERINFO */
	cchar		*org ;		/* USERINFO */
	cchar		*gecosname ;	/* USERINFO */
	cchar		*name ;		/* USERINFO */
	cchar		*realname ;	/* USERINFO */
	cchar		*fullname ;	/* USERINFO */
	cchar		*mailname ;	/* USERINFO */
	cchar		*logid ;	/* USERINFO */
	cchar		*hostname ;	/* USERINFO derived */
	const char	*groupname ;	/* gotten */
	const char	*tmpdname ;		/* temporary directory */
	const char	*msgfname ;
	const char	*lfname ;
	void		*efp ;			/* error Basic file */
	void		*lip ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	logfile		lh ;
	time_t		daytime ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	uint		hv ;
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbosity level */
	int		logsize ;
	int		n ;
	int		necinfo ;		/* number OPwords for ECINFO */
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	cchar		**argv ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;

struct ecmsgdesc {
	const char	*mp ;
	int		rlen ;
} ;

struct fileinfo {
	time_t		mtime ;
	uint		len ;
	uint		cksum ;
} ;


#ifdef	__cplusplus
}
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


