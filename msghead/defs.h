/* defs */


/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<userinfo.h>
#include	<pcsconf.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<paramfile.h>
#include	<ids.h>
#include	<localmisc.h>


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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFSIZE		4096
#define	NNODES		20		/* default COMSAT nodes */

#define	EX_FORWARDED	1		/* mail is being forwarded */
#define	EX_ACCESS	2		/* could not acccess mail */
#define	EX_NOSPACE	3		/* no space on user's filesystem */
#define	EX_INVALID	4		/* something invalid */
#define	EX_LOCKED	5		/* user's mail is locked */


struct proginfo_flags {
	uint		progdash : 1 ;
	uint		akopts : 1 ;
	uint		akparams : 1 ;
	uint		quiet : 1 ;
	uint		multirecip : 1 ;
	uint		setuid : 1 ;
	uint		setgid : 1 ;
	uint		trusted : 1 ;
	uint		pcsconf : 1 ;
	uint		pcspoll : 1 ;
	uint		logfile : 1 ;
	uint		logconf : 1 ;
	uint		logmsg : 1 ;
	uint		logenv : 1 ;
	uint		logzone : 1 ;
	uint		logmsgid : 1 ;
	uint		logsys : 1 ;
	uint		diverting : 1 ;
	uint		comsat : 1 ;
	uint		spam : 1 ;
	uint		mbtab : 1 ;
	uint		nopollmsg : 1 ;
	uint		crnl : 1 ;
	uint		optlogconf : 1 ;
	uint		optlogmsg : 1 ;
	uint		optlogzone : 1 ;
	uint		optlogenv : 1 ;
	uint		optlogmsgid : 1 ;
	uint		optlogsys : 1 ;
	uint		optdivert : 1 ;
	uint		optforward : 1 ;
	uint		optnospam : 1 ;
	uint		optnorepeat : 1 ;
	uint		optin : 1 ;
	uint		optmailhist : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
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
	char		*homedname ;
	char		*groupname ;
	char		*logid ;
	char		*tmpdname ;
	char		*maildname ;
	char		*deadmaildname ;
	char		*logzonefname ;
	char		*comsatfname ;
	char		*spamfname ;
	char		*spambox ;
	char		*mbfname ;
	char		*lockaddr ;
	char		*envfromaddr ;
	char		*msgfromaddr ;
	char		*msgsubject ;
	char		*protospec ;
	USERINFO	*uip ;
	PCSCONF		*pp ;
	void		*lip ;
	void		*efp ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	arg ;
	struct proginfo_flags	open ;
	struct timeb	now ;
	DATER		tmpdate ;
	LOGFILE		lh ;
	LOGFILE		envsum ;
	LOGZONES	lz ;
	PARAMFILE	mbtab ;
	IDS		id ;
	time_t		daytime ;
	uid_t		uid, euid, uid_maildir, uid_divert ;
	gid_t		gid, egid, gid_maildir ;
	gid_t		gid_mail ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		serial ;
	int		timeout ;
	int		tomsgread ;
	int		msgn ;
	int		nwl_system, nwl_local ;
	int		nbl_system, nbl_local ;
	int		c_processed, c_delivered ;
	char		cluster[NODENAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
	char		stamp[TIMEBUFLEN + 1] ;
	char		boxdname[MAXNAMELEN + 1] ;
	char		boxname[MAXNAMELEN + 1] ;
	char		username_pcs[LOGNAMELEN + 1] ;
} ;

struct msginfo_flags {
	uint		messageid : 1 ;
	uint		spam : 1 ;
	uint		spamdeliver : 1 ;
} ;

struct msginfo {
	DATER		edate ;		/* envelope date */
	time_t		etime ;		/* envelope time */
	time_t		mtime ;		/* message time */
	uint		offset ;	/* file offset */
	uint		mlen ;		/* message length */
	int		clen ;		/* content length */
	int		nlines ;	/* number of lines */
	struct msginfo_flags	f ;
	char		e_from[MAILADDRLEN + 1] ;
	char		h_messageid[MAILADDRLEN + 1] ;
	char		h_returnpath[MAILADDRLEN + 1] ;
	char		h_errorsto[MAILADDRLEN + 1] ;
	char		h_sender[MAILADDRLEN + 1] ;
	char		h_from[MAILADDRLEN + 1] ;
	char		h_replyto[MAILADDRLEN + 1] ;
	char		h_to[MAILADDRLEN + 1] ;
	char		h_subject[MAILADDRLEN + 1] ;
} ;

struct md {
	uint		offset ;
	uint		mlen ;
} ;

struct recip {
	vecitem		mds ;		/* message deliveries */
	uint		offset ;	/* starting mailbox offset */
	uint		n ;		/* number of deliveries */
	int		rs ;		/* delivery status */
	char		recipient[LOGNAMELEN + 1] ;
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

extern int proginfo_start(struct proginfo *,char **,const char *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_rootprogdname(struct proginfo *) ;
extern int proginfo_rootexecname(struct proginfo *,const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


