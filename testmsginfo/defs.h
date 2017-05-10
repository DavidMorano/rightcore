/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<netdb.h>

#include	<logfile.h>
#include	<vecstr.h>
#include	<dater.h>
#include	<pcsconf.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<paramfile.h>
#include	<localmisc.h>

#include	"logzones.h"
#include	"ids.h"


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
	uint	quiet : 1 ;
	uint	multirecip : 1 ;
	uint	sysv_ct : 1 ;
	uint	sysv_rt : 1 ;
	uint	setuid : 1 ;
	uint	setgid : 1 ;
	uint	trusted : 1 ;
	uint	pcspoll : 1 ;
	uint	log : 1 ;
	uint	logenv : 1 ;
	uint	logzones : 1 ;
	uint	diverting : 1 ;
	uint	comsat : 1 ;
	uint	spam : 1 ;
	uint	mbtab : 1 ;
	uint	nopollmsg : 1 ;
	uint	optlogzones : 1 ;
	uint	optlogenv : 1 ;
	uint	optlogconf : 1 ;
	uint	optdivert : 1 ;
	uint	optforward : 1 ;
	uint	optlogmsgid : 1 ;
	uint	optnospam : 1 ;
	uint	optnorepeat : 1 ;
	uint	optsyslog : 1 ;
	uint	optin : 1 ;
	uint	wl_system : 1 ;
	uint	wl_local : 1 ;
	uint	bl_system : 1 ;
	uint	bl_local : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
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
	char		*groupname ;
	char		*nodename, *domainname ;
	char		*logid ;
	char		*maildname ;
	char		*deadmaildname ;
	char		*logzonefname ;
	char		*comsatfname ;
	char		*spamfname ;
	char		*mbfname ;
	char		*lockaddr ;
	char		*msgfromaddr ;
	char		*msgsubject ;
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
	IDS		id ;
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
	int		tomsgread ;
	int		msgn ;
	int		nwl_system, nwl_local ;
	int		nbl_system, nbl_local ;
	char		cluster[NODENAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
	char		stamp[TIMEBUFLEN + 1] ;
	char		boxdname[MAXNAMELEN + 1] ;
	char		boxname[MAXNAMELEN + 1] ;
	char		logname_pcs[LOGNAMELEN + 1] ;
} ;

struct msginfo {
	char	*efrom ;
	char	*mid ;
	DATER	edate ;
	time_t	etime ;			/* envelope time */
	time_t	mtime ;			/* message time */
	uint	offset ;
	uint	mlen ;
	int	clen ;
	int	f_messageid ;
	int	f_spam ;
	char	h_messageid[MAILADDRLEN + 1] ;
	char	h_returnpath[MAILADDRLEN + 1] ;
	char	h_errorsto[MAILADDRLEN + 1] ;
	char	h_sender[MAILADDRLEN + 1] ;
	char	h_from[MAILADDRLEN + 1] ;
	char	h_replyto[MAILADDRLEN + 1] ;
	char	h_to[MAILADDRLEN + 1] ;
	char	h_subject[MAILADDRLEN + 1] ;
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


#endif /* DEFS_INCLUDE */


