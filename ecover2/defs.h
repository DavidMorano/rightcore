/* defs INCLUDE FILE */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


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


#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif

#ifndef	PROGINFO_FL
#define	PROGINFO_FL	struct proginfo_flags
#endif

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

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		8192

#define	NOPWORDS	128		/* words (64 bit) per block */


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		logprog:1 ;
	uint		log:1 ;
	uint		stderror:1 ;
	uint		seekable:1 ;
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
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*logid ;
	const char	*tmpdname ;		/* temporary directory */
	const char	*msgfname ;
	const char	*lfname ;
	void		*efp ;			/* error Basic file */
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	logfile		lh ;
	time_t		daytime ;
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;		/* debugging level */
	int		verboselevel ;		/* verbosity level */
	int		logsize ;
	int		necinfo ;		/* number OPwords for ECINFO */
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

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
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


