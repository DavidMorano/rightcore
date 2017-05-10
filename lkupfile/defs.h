/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<paramopt.h>
#include	<svctab.h>
#include	<ids.h>
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

#define	STDINFNAME	"*STDIN*"
#define	STDOUTFNAME	"*STDOUT*"
#define	STDERRFNAME	"*STDERR*"

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint	kopts : 1 ;
	uint	aparams : 1 ;
	uint	log : 1 ;
	uint	nochange : 1 ;
	uint	quiet : 1 ;
	uint	print : 1 ;
	uint	suffix : 1 ;
	uint	follow : 1 ;		/* follow symbolic links */
	uint	fdbcheck : 1 ;		/* check file DB */
	uint	fdbopen : 1 ;		/* is it open ? */
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
	char		*username ;
	char		*groupname ;
	char		*tmpdname ;
	char		*prog_lpget ;
	void		*efp ;
	void		*ofp ;
	struct proginfo_flags	f ;
	struct proginfo_flags	open ;
	LOGFILE		lh ;
	VECSTR		fdb ;
	IDS		ids ;
	int		pwdlen ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		namelen ;
	int		c_files, c_processed ;
} ;



#ifdef	__cplusplus
extern "C" {
#endif

extern int	fdbinit(struct proginfo *,const char *) ;
extern int	fdbfetch(struct proginfo *,const char *,char *,int) ;
extern int	fdbfree(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


