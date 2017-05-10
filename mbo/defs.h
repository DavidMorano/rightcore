/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/timeb.h>

#include	<logfile.h>
#include	<userinfo.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<pcsconf.h>
#include	<vecitem.h>
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
#define	LOGNAMELEN	32
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFSIZE		4096
#define	NNODES		20		/* default COMSAT nodes */

#ifndef	ADDRLEN
#define	ADDRLEN		(2 * MAXHOSTNAMELEN)
#endif

#define	EX_FORWARDED	1		/* mail is being forwarded */
#define	EX_ACCESS	2		/* could not acccess mail */
#define	EX_NOSPACE	3		/* no space on user's filesystem */
#define	EX_INVALID	4		/* something invalid */
#define	EX_LOCKED	5		/* user's mail is locked */


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		multirecip:1 ;
	uint		sysv_ct:1 ;
	uint		sysv_rt:1 ;
	uint		setuid:1 ;
	uint		setgid:1 ;
	uint		trusted:1 ;
	uint		log:1 ;
	uint		logenv:1 ;
	uint		logzones:1 ;
	uint		diverting:1 ;
	uint		comsat:1 ;
	uint		spam:1 ;
	uint		optlogzones:1 ;
	uint		optlogenv:1 ;
	uint		optdivert:1 ;
	uint		optlogmsgid:1 ;
	uint		optnospam:1 ;
	uint		optnorepeat:1 ;
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
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*logid ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*deadmaildname ;
	const char	*logzonefname ;
	const char	*comsatfname ;
	const char	*spamfname ;
	const char	*lockaddr ;
	const char	*fromaddr ;
	const char	*protospec ;
	USERINFO	*uip ;
	PCSCONF		*pp ;
	void		*efp ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	struct timeb	now ;
	DATER		tmpdate ;
	LOGFILE		lh ;
	LOGFILE		envsum ;
	LOGZONES	lz ;
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
	const char	zname[DATER_ZNAMESIZE + 1] ;
	const char	stamp[TIMEBUFLEN + 1] ;
} ;

struct msgoff {
	time_t	mtime ;			/* message date */
	int	offset ;
	int	mlen ;
	int	f_spam ;
	char	messageid[ADDRLEN + 1] ;
	char	from[ADDRLEN + 1] ;
} ;

struct md {
	uint		offset ;
	uint		mlen ;
} ;

struct recip {
	vecitem	mds ;			/* message deliveries */
	uint		offset ;	/* starting mailbox offset */
	uint		n ;		/* number of deliveries */
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
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


