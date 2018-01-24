/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<sys/stat.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<dater.h>
#include	<logzones.h>
#include	<localmisc.h>

#include	"ng.h"
#include	"retpath.h"


#ifndef	TYPEDEF_UTIME
#define	TYPEDEF_UTIME
typedef unsigned long	utime_t	;
#endif

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

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real-name length */
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
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

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
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

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#ifndef	MAILTEXTCOLS
#define	MAILTEXTCOLS	998
#endif

#ifndef	PCS_MSGIDLEN
#define	PCS_MSGIDLEN	(2 * MAXHOSTNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	EMAENTRY
#define	EMAENTRY	EMA_ENT
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	DATE1970	(24 * 3600)

#define BUFSIZE		((2*MAXPATHLEN) + 4)
#define	BUFLEN		BUFSIZE

#if	defined(BSD)
#define	MAP_FAILED	((void *) (-1))
#endif

/* program exit status values */

#define	EX_BADARG	1		/* bad argument on invocation */
#define	EX_ERROR	3		/* error during processing */

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo

#define	TDINFO		struct tdinfo


struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		errfile:1 ;
	uint		outfile:1 ;
	uint		pcsconf:1 ;
	uint		cfname:1 ;
	uint		logprog:1 ;
	uint		logmsg:1 ;
	uint		logenv:1 ;
	uint		logzone:1 ;
	uint		sysv_rt:1 ;
	uint		sysv_ct:1 ;
	uint		update:1 ;
	uint		extrascan:1 ;
	uint		trusted:1 ;
	uint		crnl:1 ;
	uint		spam:1 ;
	uint		artexpires:1 ;
	uint		artmaint:1 ;
	uint		artcores:1 ;
	uint		mime:1 ;
	uint		dis_inline:1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;
	const char	*progdname ;
	const char	*progname ;
	const char	*pr ;			/* program root */
	const char	*searchname ;
	const char	*version ;
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*gecosname ;		/* PASSWD GECOS-name */
	const char	*realname ;		/* PASSWD read-name */
	const char	*name ;			/* USERINFO name */
	const char	*fullname ;		/* USERINFO mail-name */
	const char	*mailname ;		/* USERINFO full-name */
	const char	*org ;
	const char	*groupname ;
	const char	*logid ;
	const char	*hostname ;
	const char	*orgdomainname ;
	const char	*tmpdname ;
	const char	*newsdname ;
	const char	*maildname ;
	const char	*cfname ;
	const char	*lfname ;
	const char	*prog_rslow ;
	const char	*prog_mailer ;
	const char	*envfromaddr ;
	const char	*msgfromaddr ;
	const char	*msgsubject ;
	const char	*msgfromname ;
	const char	*uu_machine ;
	const char	*uu_user ;
	const char	*fromnode ;
	const char	*mailnode ;
	const char	*uucpnode ;
	const char	*mailhost ;
	const char	*uucphost ;
	const char	*userhost ;
	const char	*r_transport, *r_machine, *r_user ;
	void		*efp ;
	void		*buffer ;		/* general buffer */
	void		*contextp ;		/* SHELL context */
	void		*lip ;			/* local information */
	void		*uip ;			/* for USERINFO */
	void		*userlist ;		/* user-list state */
	void		*config ;		/* for MSU */
	void		*pcsconf ;		/* save space when not needed */
	void		*namecache ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	struct timeb	now ;
	LOGFILE		lh ;
	LOGFILE		envsum ;
	DATER		td ;
	LOGZONES	lz ;
	NG		ngs ;			/* specified newsgroups */
	time_t		daytime ;
	time_t		ti_expires ;
	pid_t		pid ;
	uid_t		uid, euid ;
	uid_t		uid_pcs ;
	gid_t		gid, egid ;
	gid_t		gid_pcs ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		logsize ;
	int		subjectmode ;
	int		nmsgs ;
	int		pserial ;
	int		n ;
	char		zname[DATER_ZNAMESIZE + 1] ;
	char		expdate[TIMEBUFLEN+1] ;
	char		stamp[TIMEBUFLEN + 1] ;
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	int		argc ;
	int		ai, ai_max, ai_pos ;
	const char	**argv ;
} ;

struct tdinfo {
	dev_t		dev ;
	int		tdlen ;
	char		tdname[MAXPATHLEN+1] ;
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


