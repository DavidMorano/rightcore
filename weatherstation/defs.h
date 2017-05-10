/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
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

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		((4 * MAXPATHLEN) + 20)

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint	aparams : 1 ;
	uint	nochange : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	vecstr	stores ;
	char	**envv ;
	char	*pwd ;
	char	*progename ;
	char	*progdname ;
	char	*progname ;
	char	*pr ;
	char	*searchname ;
	char	*version ;
	char	*banner ;
	char	*tmpdname ;
	struct proginfo_flags	f ;
	struct proginfo_flags	open ;
	void	*ofp ;
	void	*efp ;
	int	pwdlen ;
	int	debuglevel ;
	int	verboselevel ;
} ;


struct options {
	uint	lower : 1 ;
	uint	inplace : 1 ;
	uint	doublespace : 1 ;
	uint	half : 1 ;
	uint	leading : 1 ;
} ;


#endif /* DEFS_INCLUDE */


