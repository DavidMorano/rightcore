/* defs (pcsgetmail) */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vecstr.h>
#include	<ids.h>
#include	<logsys.h>
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
#define	LOGNAMELEN	32
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	EX_FORWARDED	1		/* mail is being forwarded */
#define	EX_ACCESS	2		/* could not acccess mail */
#define	EX_NOSPACE	3		/* no space on user's filesystem */
#define	EX_INVALID	4		/* something invalid */
#define	EX_LOCKED	5		/* user's mail is locked */

#define	BUFLEN		(2 * MAXPATHLEN)

#define	LOGCNAME	"log"
#define	FORWARDED	"Forward to "	/* the forwarding indicator */

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
	uint		sysv_rt:1 ;
	uint		sysv_ct:1 ;
	uint		setuid:1 ;
	uint		setgid:1 ;
	uint		nodel:1 ;	 /* do not delete after retrieval */
	uint		logprog:1 ;
	uint		logfile:1 ;
	uint		logsys:1 ;
	uint		pcsconf:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	vecstr		mailusers ;
	vecstr		maildirs ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* program directory */
	const char	*progdname ;	/* program directory */
	const char	*progname ;	/* program name */
	const char	*pr ;		/* PCS "root" directory */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*nodename ;	/* USERINFO */
	const char	*domainname ;	/* USERINFO */
	const char	*username ;	/* USERINFO */
	const char	*homedname ;	/* USERINFO */
	const char	*gecosname ;	/* USERINFO */
	const char	*realname ;	/* USERINFO */
	const char	*org ;		/* USERINFO */
	const char	*name ;		/* USERINFO */
	const char	*fullname ;	/* USERINFO */
	const char	*mailname ;	/* USERINFO */
	const char	*logid ;	/* USERINFO */
	const char	*hostname ;	/* derived */
	const char	*groupname ;
	const char	*tmpdname ;	/* temporary directory */
	const char	*maildname ;	/* system mail directory */
	const char	*folderdname ;	/* user folder directory */
	const char	*lfname ;
	const char	*mbox ;		/* target mail-box */
	const char	*mfname ;	/* mail-file (of mailbox) */
	void		*efp ;		/* error Basic file */
	void		*ofp ;		/* output Basic file */
	void		*contextp ;
	void		*lip ;
	void		*uip ;
	void		*userlist ;
	void		*config ;
	void		*pcsconf ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	IDS		id ;
	LOGSYS		ls ;
	LOGFILE		lh ;
	time_t		daytime ;	/* system time of day */
	pid_t		pid ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	gid_t		gid_mail ;	/* GID for group 'mail' */
	int		pwdlen ;
	int		debuglevel ;	/* debugging level */
	int		verboselevel ;	/* verbosity level */
	int		logsize ;	/* log-size (maintenance) */
	int		serial ;
	int		timeout ;	/* general timeout */
	int		to_mailspool ;	/* mailspool timeout */
	int		to_mailbox ;	/* mailbox timeout */
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
}
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


