/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
#include	<logfile.h>
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

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	BUFLEN		MAX((8 * 1024),MAXPATHLEN)





struct proginfo_flags {
	uint	quiet : 1 ;
	uint	logfile : 1 ;
	uint	systems : 1 ;
	uint	dialer : 1 ;
	uint	outfile : 1 ;
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
	char		*domainname ;
	char		*nodename ;
	char		*username ;
	char		*helpfname ;
	char		*pidfname ;
	void		*efp ;
	struct proginfo_flags	f ;
	struct proginfo_flags	open ;
	logfile		lh ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		f_exit, f_signal ;
	int		signal_num ;
} ;


#endif /* DEFS_INCLUDE */


