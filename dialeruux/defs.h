/* defs */


/* revision history:

	= 2009-05-01, David A�D� Morano

	This code was originally written.


*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<ids.h>
#include	<pcsconf.h>
#include	<localmisc.h>


/* local defines */

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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		1024

#define	EX_FORWARDED	1		/* mail is being forwarded */
#define	EX_ACCESS	2		/* could not acccess mail */
#define	EX_NOSPACE	3		/* no space on user's filesystem */
#define	EX_INVALID	4		/* something invalid */
#define	EX_LOCKED	5		/* user's mail is locked */



struct proginfo_flags {
	uint	progdash : 1 ;
	uint	aparams : 1 ;
	uint	akopts : 1 ;
	uint	quiet : 1 ;
	uint	lockfile : 1 ;
	uint	lockopen : 1 ;
	uint	pidfile : 1 ;
	uint	log : 1 ;
	uint	optin : 1 ;
	uint	trusted : 1 ;
	uint	pcspoll : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	char		*pwd ;
	char		*progdname ;
	char		*progename ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*nodename ;
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*helpfname ;
	char		*pidfname ;
	void		*efp ;
	struct pcsconf	*pp ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	logfile		lh ;
	IDS		id ;
	pid_t		pid ;
	time_t		daytime ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		timeout, loglen ;
	int		f_exit ;
	int		signal_num ;
	char		cluster[NODENAMELEN + 1] ;
	char		mailhost[MAXHOSTNAMELEN + 1] ;
	char		mailsvc[SVCNAMELEN + 1] ;
} ;

struct pivars {
	char	*vpr1 ;
	char	*vpr2 ;
	char	*vpr3 ;
	char	*pr ;
	char	*vprname ;
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


