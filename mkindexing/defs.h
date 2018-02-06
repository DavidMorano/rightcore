/* defs */


/* revision history:

	= 1999-09-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bits.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<eigendb.h>
#include	<strstore.h>
#include	<ids.h>
#include	<hdb.h>
#include	<ptm.h>
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
#define	LOGNAMELEN	32
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#ifndef	NATURALWORDLEN
#define	NATURALWORDLEN	50
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	STDFNAMENULL
#define	STDFNAMENULL	"*STDNULL*"
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo

#define	PIVARS		struct pivars


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		af:1 ;
	uint		logprog:1 ;
	uint		lfname:1 ;
	uint		args:1 ;
	uint		ids:1 ;
	uint		eigendb:1 ;
	uint		removelabel:1 ;
	uint		wholefile:1 ;
	uint		append:1 ;
	uint		prefix:1 ;
	uint		tablen:1 ;
	uint		linelen:1 ;
	uint		indent:1 ;
	uint		keyfold:1 ;
	uint		nodebug:1 ;
	uint		iacc:1 ;	/* ignore no-access problems */
	uint		optbible:1 ;
	uint		optnofile:1 ;
	uint		optoutcookie:1 ;
	uint		optpar:1 ;
	uint		optuniq:1 ;
	uint		optaudit:1 ;
	uint		optsendparams:1 ;
} ;

struct proginfo {
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;	/* program directory */
	cchar		*progdname ;	/* program directory */
	cchar		*progname ;	/* program name */
	cchar		*pr ;		/* program root directory */
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*username ;
	cchar		*groupname ;
	cchar		*nodename ;
	cchar		*domainname ;
	cchar		*name ;
	cchar		*fullname ;
	cchar		*logid ;
	cchar		*tmpdname ;	/* temporary directory */
	cchar		*basedname ;	/* base directory name */
	cchar		*lfname ;	/* file-name log */
	cchar		*ifname ;	/* file-name input */
	cchar		*dfname ;	/* file-name default */
	cchar		*ndbname ;	/* bible-name DB */
	cchar		*eigenlang ;
	cchar		*eigenfname ;
	cchar		*sdn ;		/* stored-directory-name */
	cchar		*sfn ;		/* stored-file-name */
	void		*sip ;		/* subinfo */
	void		*efp ;		/* error Basic file */
	void		*userlist ;
	vecstr		stores ;
	logfile		lh ;
	EIGENDB		eigendb ;
	IDS		id ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	time_t		daytime ;
	int		pwdlen ;
	int		progmode ;	/* program mode */
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;
	int		quietlevel ;
	int		logsize ;
	int		minwordlen ;
	int		maxwordlen ;
	int		maxkeys ;	/* maximum keys per tag entry */
	int		eigenwords ;	/* default EIGENWORDS */
	int		tablen ;	/* the hash table length */
	int		linelen ;	/* line-fold length */
	int		indent ;
	int		nprocessed ;
	int		ncpu ;
	int		npar ;		/* number in parallel */
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
} ;

struct arginfo {
	cchar		**argv ;
	cchar		*afname ;
	BITS		pargs ;
	VECSTR		args ;
	int		argc ;
	int		ai_max, ai_pos ;
} ;

struct keyinfo {
	STRSTORE	eigenwords ;
	uint		minwlen ;
	uint		maxwlen ;
	uint		maxkeys ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(PROGINFO *,cchar **,cchar *,cchar *) ;
extern int proginfo_setprogroot(PROGINFO *,cchar *,int) ;
extern int proginfo_setentry(PROGINFO *,cchar **,cchar *,int) ;
extern int proginfo_setversion(PROGINFO *,cchar *) ;
extern int proginfo_setbanner(PROGINFO *,cchar *) ;
extern int proginfo_setsearchname(PROGINFO *,cchar *,cchar *) ;
extern int proginfo_setprogname(PROGINFO *,cchar *) ;
extern int proginfo_setexecname(PROGINFO *,cchar *) ;
extern int proginfo_pwd(PROGINFO *) ;
extern int proginfo_progdname(PROGINFO *) ;
extern int proginfo_progename(PROGINFO *) ;
extern int proginfo_nodename(PROGINFO *) ;
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_getename(PROGINFO *,char *,int) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


