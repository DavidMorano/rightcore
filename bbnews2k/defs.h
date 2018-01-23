/* defs (BBNEWS) */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/timeb.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<dater.h>
#include	<ids.h>
#include	<expcook.h>
#include	<localmisc.h>


/* standard stuff */

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

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		120
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(6 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
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


/* this was used in the old days (so we continue to supply it) */
#ifndef	HI_MSGID
#define	HI_MSGID	HI_MESSAGEID
#endif

/* program execution modes */

#define	PM_READ		0
#define	PM_HEADER	1
#define	PM_NAMES	2
#define	PM_COUNT	3
#define	PM_SUBSCRIPTION	4
#define	PM_MAILBOX	5
#define	PM_TEST		6
#define	PM_OVERLAST	7

/* Save Modes */

#define	SMODE_MAILBOX	0
#define	SMODE_OUT	1

/* miscellaneous */

#define COLS		COLUMNS
#define	DATE1970	(24 * 3600)

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#define BUFSIZE		((2*MAXPATHLEN) + 4)
#define	BUFLEN		BUFSIZE
#define	CMDBUFLEN	((2 * MAXPATHLEN) + 100)
#define	JOBIDLEN	32

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

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
	uint		pcsconf:1 ;
	uint		svars:1 ;
	uint		cooks:1 ;
	uint		lfname:1 ;
	uint		cmdfname:1 ;
	uint		nosysconf:1 ;
	uint		logprog:1 ;
	uint		logsize:1 ;
	uint		exit:1 ;
	uint		terminal:1 ;
	uint		ansiterm:1 ;
	uint		popscreen:1 ;
	uint		clock:1 ;
	uint		new:1 ;
	uint		old:1 ;
	uint		all:1 ;
	uint		every:1 ;
	uint		reverse:1 ;
	uint		newprogram:1 ;
	uint		interactive:1 ;
	uint		mailbox:1 ;
	uint		mailcheck:1 ;
	uint		catchup:1 ;
	uint		nopage:1 ;
	uint		sysv_rt:1 ;
	uint		sysv_ct:1 ;
	uint		combine:1 ;
	uint		count:1 ;	/* not used */
	uint		extrascan:1 ;
	uint		query:1 ;
	uint		newmessages:1 ;
	uint		subscribe:1 ;	/* subscription mode */
	uint		test:1 ;
	uint		term:1 ;	/* terminal-mode */
	uint		description:1 ;
	uint		header:1 ;	/* listing header values */
	uint		readtime:1 ;	/* readable user file */
	uint		addenv:1 ;
	uint		envdate:1 ;
	uint		envfrom:1 ;
	uint		useclen:1 ;
	uint		useclines:1 ;
	int		nextmov:1 ;
	int		nextdel:1 ;
	int		svpercent:1 ;
	int		sjpercent:1 ;
	uint		id:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;
	cchar		*progdname ;
	cchar		*progname ;
	cchar		*pr ;		/* program root */
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*usysname ;	/* UNAME OS system-name */
	cchar		*umachine ;	/* UNAME machine name */
	cchar		*urelease ;	/* UNAME OS release */
	cchar		*uversion ;	/* UNAME OS version */
	cchar		*architecture ;	/* UAUX machine architecture */
	cchar		*platform ;	/* UAUX machine platform */
	cchar		*provider ;	/* UAUX machine provider */
	cchar		*hz ;		/* OS HZ */
	cchar		*nodename ;	/* USERINFO */
	cchar		*domainname ;	/* USERINFO */
	cchar		*username ;	/* USERINFO */
	cchar		*homedname ;	/* USERINFO */
	cchar		*shell ;	/* USERINFO */
	cchar		*org ;		/* USERINFO */
	cchar		*gecosname ;	/* USERINFO */
	cchar		*realname ;	/* USERINFO */
	cchar		*name ;		/* USERINFO */
	cchar		*fullname ;	/* USERINFO full-name */
	cchar		*mailname ;	/* USERINFO mail-abbreviated-name */
	cchar		*tz ;		/* USERINFO */
	cchar		*maildname ;	/* USERINFO */
	cchar		*logid ;	/* USERINFO ID for logging purposes */
	cchar		*pcsusername ;
	cchar		*hostname ;	/* derived */
	cchar		*jobid ;
	cchar		*tmpdname ;
	cchar		*newsdname ;
	cchar		*folderdname ;
	cchar		*vmdname ;
	cchar		*cfname ;
	cchar		*lfname ;
	cchar		*ufname ;
	cchar		*cmdfname ;
	cchar		*helpfname ;
	cchar		*cmdhelpfname ;
	cchar		*mbname_def ;
	cchar		*mbname_in ;
	cchar		*mbname_spam ;
	cchar		*mbname_trash ;
	cchar		*prog_editor ;
	cchar		*prog_mailer ;
	cchar		*prog_metamail ;
	cchar		*prog_bbpost ;
	cchar		*prog_pager ;
	cchar		*prog_print ;
	cchar		*prog_shell ;
	cchar		*prog_getmail ;
	cchar		*prog_postspam ;
	cchar		*prefix ;
	cchar		*termtype ;
	cchar		*mailhost ;
	cchar		*fromnode ;
	cchar		*querytext ;
	cchar		*mailbox ;
	cchar		*envfrom ;
	cchar		*kbdtype ;
	cchar		*mbname_cur ;
	cchar		*testmsg ;
	void		*efp ;
	void		*ofp ;
	void		*ifp ;
	void		*iap ;		/* interactive */
	void		*buffer ;	/* general buffer */
	void		*contextp ;	/* SHELL context */
	void		*lip ;		/* local information */
	void		*uip ;		/* USERINFO object */
	void		*userlist ;	/* user-list state */
	void		*config ;	/* configuration */
	void		*pcsconf ;	/* save space when not needed */
	void		*hdr ;		/* for HDRDECODE */
	PROGINFO_FL	have, final, f, changed ;
	PROGINFO_FL	open ;
	struct timeb	now ;
	logfile		lh ;
	DATER		envdate, tmpdate ;
	IDS		id ;
	EXPCOOK		cooks ;
	vecstr		svars ;
	time_t		daytime ;
	time_t		ti_config ;
	time_t		ti_clock ;
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		termlines ;
	int		showlines ;
	int		linelen ;
	int		mailcheck ;
	int		sortmode ;		/* sorting mode used */
	int		header ;		/* list this header type */
	int		whichenvdate ;		/* which date for envelope */
	int		count ;
	int		lines ;
	int		svlines ;
	int		sjlines ;
	int		n ;
	int		to_mailcheck ;
	int		to_config ;
	int		to_clock ;
	int		to_read ;
	int		to_info ;
	int		f_exit ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;

struct pivars {
	cchar		*vpr1 ;
	cchar		*vpr2 ;
	cchar		*vpr3 ;
	cchar		*pr ;
	cchar		*vprname ;
} ;

struct arginfo {
	int		argc ;
	int		ai, ai_max, ai_pos ;
	cchar		**argv ;
	cchar		*afname ;
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


