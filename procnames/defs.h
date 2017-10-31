/* defs */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>
#include	<signal.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<dater.h>
#include	<expcook.h>
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

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
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

#ifndef	MAILTEXTCOLS
#define	MAILTEXTCOLS	998
#endif

#ifndef	PCS_MSGIDLEN
#define	PCS_MSGIDLEN	(2 * MAXHOSTNAMELEN)
#endif

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
	uint		pcsconf:1 ;		/* PCSCONF */
	uint		headers:1 ;
	uint		atts:1 ;
	uint		emas:1 ;
	uint		verbose:1 ;
	uint		noinput:1 ;
	uint		logprog:1 ;
	uint		pec:1 ;
	uint		mdate:1 ;		/* user specified MSG-date */
	uint		def_from:1 ;		/* default FROM requested */
	uint		crnl:1 ;		/* stupid CRNL mode */
	uint		h_org:1 ;		/* header-organization */
	uint		h_from:1 ;		/* header-from */
	uint		h_replyto:1 ;		/* header-replyto */
	uint		h_sender:1 ;		/* header-sender */
	uint		add_org:1 ;		/* ensure header-organization */
	uint		add_face:1 ;		/* ensure header-face */
	uint		add_from:1 ;		/* ensure header-from */
	uint		add_replyto:1 ;		/* ensure header-replyto */
	uint		add_sender:1 ;		/* ensure header-sender */
	uint		init_face:1 ;		/* initialized header-FACE */
	uint		init_from:1 ;		/* initialized header-FROM */
	uint		init_replyto:1 ;	/* initialized header-REPLYTO */
	uint		init_sender:1 ;		/* initialized header-SENDER */
	uint		init_fromname:1 ;	/* initialized FROM-name */
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
	const char	*nodename ;		/* USERINFO */
	const char	*domainname ;		/* USERINFO */
	const char	*username ;		/* USERINFO */
	const char	*homedname ;		/* USERINFO */
	const char	*gecosname ;		/* USERINFO */
	const char	*realname ;		/* USERINFO */
	const char	*name ;			/* USERINFO */
	const char	*fullname ;		/* USERINFO */
	const char	*mailname ;		/* USERINFO */
	const char	*org ;			/* USERINFO */
	const char	*logid ;		/* USERINFO */
	const char	*hostname ;		/* PROCUSERINFO */
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*lfname ;
	const char	*fromname ;
	const char	*facility ;
	const char	*disclaimer ;
	const char	*hdr_mailer ;
	const char	*hdr_mid ;
	const char	*hdr_subject ;
	const char	*hdr_from ;
	const char	*hdr_replyto ;
	const char	*hdr_sender ;
	void		*efp ;
	void		*ofp ;
	void		*lip ;
	void		*pcp ;
	void		*userlist ;
	logfile		lh ;
	struct timeb	now ;
	DATER		tmpdate, mdate ;
	EXPCOOK		pec ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	ULONG		rand ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	pid_t		pid ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		serial ;
	int		n ;
	char		zname[DATER_ZNAMESIZE + 1] ;
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


