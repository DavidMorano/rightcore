/* defs */


/* revision history:

	= 1998-02-12, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<vecitem.h>
#include	<ids.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<localmisc.h>

#include	"logzones.h"
#include	"msgid.h"
#include	"lookaddr.h"


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

#ifndef	LOGNAMELEN
#ifdef	PASS_MAX
#define	LOGNAMELEN	PASS_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
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

#ifndef	MAILTEXTCOLS
#define	MAILTEXTCOLS	998
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
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

#define	MSGINFO		struct msginfo
#define	MSGINFO_FL	struct msginfo_flags

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_GMCUR	struct locinfo_gmcur
#define	LOCINFO_RNCUR	struct locinfo_rncur

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		akparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		multirecip:1 ;
	uint		setuid:1 ;
	uint		setgid:1 ;
	uint		trusted:1 ;
	uint		pcsconf:1 ;
	uint		pcspoll:1 ;
	uint		logprog:1 ;
	uint		logconf:1 ;
	uint		logenv:1 ;
	uint		logmsg:1 ;
	uint		logzone:1 ;
	uint		logmsgid:1 ;
	uint		logsys:1 ;
	uint		diverting:1 ;
	uint		comsat:1 ;
	uint		spam:1 ;
	uint		mbtab:1 ;
	uint		nopollmsg:1 ;
	uint		crnl:1 ;
	uint		optlogconf:1 ;
	uint		optlogmsg:1 ;
	uint		optlogzone:1 ;
	uint		optlogenv:1 ;
	uint		optlogmsgid:1 ;
	uint		optlogsys:1 ;
	uint		optdivert:1 ;
	uint		optforward:1 ;
	uint		optnospam:1 ;
	uint		optnorepeat:1 ;
	uint		optin:1 ;
	uint		optmailhist:1 ;
	uint		optdeliver:1 ;
	uint		optcopy:1 ;
	uint		optspambox:1 ;	/* deliver spam to spam-box */
	uint		optfinish:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	cchar		**envv ;
	cchar		*pwd ;
	cchar		*progename ;
	cchar		*progdname ;
	cchar		*progname ;
	cchar		*pr ;
	cchar		*searchname ;
	cchar		*version ;
	cchar		*banner ;
	cchar		*nodename ;	/* USERINFO */
	cchar		*domainname ;	/* USERINFO */
	cchar		*username ;	/* USERINFO */
	cchar		*homedname ;	/* USERINFO */
	cchar		*groupname ;	/* USERINFO */
	cchar		*name ;		/* USERINFO */
	cchar		*fullname ;	/* USERINFO */
	cchar		*mailname ;	/* USERINFO */
	cchar		*logid ;	/* USERINFO */
	cchar		*cluster ;	/* USERINFO */
	cchar		*username_pcs ;
	cchar		*tmpdname ;
	cchar		*deadmaildname ;
	cchar		*copymaildname ;
	cchar		*boxdname ;
	cchar		*lfname ;	/* prog-log file-name */
	cchar		*zfname ;	/* zone-log file-name */
	cchar		*csfname ;	/* CONF COMSAT file-name */
	cchar		*mbfname ;	/* CONF mailbox-tab file-name */
	cchar		*spfname ;	/* CONF spam file-name */
	cchar		*spambox ;	/* CONF spam box-name */
	cchar		*defbox ;	/* CONF default box-name */
	cchar		*lockaddr ;
	cchar		*envfromaddr ;
	cchar		*msgsubject ;
	cchar		*protospec ;
	cchar		*portspec ;
	cchar		*uu_machine ;
	cchar		*uu_user ;
	void		*lip ;
	void		*efp ;
	void		*uip ;
	void		*pcsconf ;
	void		*userlist ;	/* user-list state */
	void		*namecache ;
	struct timeb	now ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	arg ;
	PROGINFO_FL	open ;
	DATER		tmpdate ;
	LOGFILE		lh ;
	LOGFILE		envsum ;
	LOGZONES	lz ;
	IDS		id ;
	vecstr		maildirs ;
	time_t		daytime ;
	uid_t		uid, euid, uid_maildir, uid_divert ;
	gid_t		gid, egid, gid_maildir ;
	gid_t		gid_mail ;
	pid_t		pid ;
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		to_spool ;
	int		to_msgread ;
	int		nrecips ;
	int		nmsgs ;
	int		pserial ;
	int		serial ;
	int		c_processed, c_delivered ;
	int		port_comsat ;
	char		zname[DATER_ZNAMESIZE+1] ;
	char		stamp[TIMEBUFLEN+1] ;
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
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;

struct locinfo_gmcur {
	GRMEMS_CUR	gmcur ;
} ;

struct locinfo_rncur {
	SYSREALNAME_CUR	rncur ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		mids:1 ;
	uint		mboxes:1 ;
	uint		gm:1 ;
	uint		rn:1 ;
	uint		la:1 ;		/* LOOKADDR object */
} ;

struct locinfo {
	PROGINFO	*pip ;
	vecstr		stores ;
	vecstr		mboxes ;
	MSGID		mids ;
	LOOKADDR	la ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	LOCINFO_FL	f ;
	LOCINFO_FL	open ;
	int		to ;
} ;

struct msginfo_flags {
	uint		messageid:1 ;
	uint		spam:1 ;
	uint		spamdeliver:1 ;
} ;

struct msginfo {
	DATER		edate ;		/* envelope date */
	time_t		etime ;		/* envelope time */
	time_t		mtime ;		/* message time */
	uint		moff ;		/* file offset of MSG */
	uint		mlen ;		/* message length */
	int		mi ;		/* message index */
	int		clen ;		/* content-length */
	int		clines ;	/* content-lines */
	MSGINFO_FL	f ;
	char		e_from[MAILADDRLEN+1] ;
	char		h_messageid[MAILADDRLEN+1] ;
	char		h_returnpath[MAILADDRLEN+1] ;
	char		h_deliveredto[MAILADDRLEN+1] ;
	char		h_xoriginalto[MAILADDRLEN+1] ;
	char		h_errorsto[MAILADDRLEN+1] ;
	char		h_sender[MAILADDRLEN+1] ;
	char		h_from[MAILADDRLEN+1] ;
	char		h_replyto[MAILADDRLEN+1] ;
	char		h_to[MAILADDRLEN+1] ;
	char		h_subject[MAILADDRLEN+1] ;
	char		h_articleid[MAXNAMELEN+1] ;
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
extern int proginfo_getpwd(PROGINFO *,char *,int) ;
extern int proginfo_finish(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


