/* defs */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<logfile.h>
#include	<dater.h>
#include	<pcsconf.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<localmisc.h>

#include	"logzones.h"


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
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
	uint		aparams : 1 ;
	uint		quiet : 1 ;
	uint		multirecip : 1 ;
	uint		sysv_ct : 1 ;
	uint		sysv_rt : 1 ;
	uint		setuid : 1 ;
	uint		setgid : 1 ;
	uint		trusted : 1 ;
	uint		log : 1 ;
	uint		logenv : 1 ;
	uint		logzones : 1 ;
	uint		diverting : 1 ;
	uint		comsat : 1 ;
	uint		spam : 1 ;
	uint		optlogzones : 1 ;
	uint		optlogenv : 1 ;
	uint		optdivert : 1 ;
	uint		optlogmsgid : 1 ;
	uint		optnospam : 1 ;
	uint		optnorepeat : 1 ;
	uint		optsyslog : 1 ;
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
	char		*tmpdname ;
	char		*username ;
	char		*nodename, *domainname ;
	char		*logid ;
	char		*maildname ;
	char		*deadmaildname ;
	char		*helpfname ;
	char		*logzonefname ;
	char		*comsatfname ;
	char		*spamfname ;
	char		*lockaddr ;
	char		*fromaddr ;
	char		*protospec ;
	struct userinfo	*uip ;
	struct pcsconf	*pp ;
	void		*efp ;
	struct proginfo_flags	f ;
	struct timeb	now ;
	DATER		tmpdate ;
	LOGFILE		lh ;
	LOGFILE		envsum ;
	LOGZONES	lz ;
	PARAMFILE	mbtab ;
	time_t		daytime ;
	uid_t		uid, euid, uid_maildir, uid_divert ;
	gid_t		gid, egid, gid_maildir ;
	gid_t		gid_mail ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		loglen ;
	int		timeout ;
	int		msgn ;
	char		zname[DATER_ZNAMESIZE + 1] ;
	char		stamp[TIMEBUFLEN + 1] ;
	char		boxdname[MAXNAMELEN + 1] ;
	char		boxname[MAXNAMELEN + 1] ;
} ;

struct msgoff {
	time_t	mtime ;			/* message date */
	uint	offset ;
	uint	mlen ;
	int	f_spam ;
	char	messageid[MAILADDRLEN + 1] ;
	char	from[MAILADDRLEN + 1] ;
} ;

struct md {
	uint	offset ;
	uint	mlen ;
} ;

struct recip {
	vecitem	mds ;			/* message deliveries */
	uint	offset ;		/* starting mailbox offset */
	uint	n ;			/* number of deliveries */
	int	rs ;			/* delivery status */
	char	recipient[LOGNAMELEN + 1] ;
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

extern int proginfo_start(struct proginfo *,char **,const char *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_rootprogdname(struct proginfo *) ;
extern int proginfo_rootexecname(struct proginfo *,const char *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


