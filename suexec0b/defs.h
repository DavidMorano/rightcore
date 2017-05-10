/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<paramopt.h>
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

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN * 2)
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	PASSLEN		32
#define	PROMPTLEN	100

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint		progdash : 1 ;
	uint		akopts : 1 ;
	uint		aparams : 1 ;
	uint		quiet : 1 ;
	uint		sevenbit : 1 ;
	uint		setuid : 1 ;
	uint		setgid : 1 ;
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
	char		*nodename ;
	char		*hostname ;
	char		*username ;
	char		*groupname ;
	char		*tmpdname ;
	void		*efp ;
	void		*ofp ;
	struct proginfo_flags	have, f, changed, final ;
	struct proginfo_flags	open ;
	uid_t		uid, euid ;
	gid_t		gid, egid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


