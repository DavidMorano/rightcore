/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecstr.h>
#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	ZNAMELEN
#define	ZNAMELEN	8
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif




struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progdname ;
	char		*progename ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*username ;
	char		*version ;
	char		*banner ;
	char		*helpfname ;
	void		*efp ;
	struct proginfo_flags	f ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	char		zname[ZNAMELEN + 1] ;
} ;


#endif /* DEFS_INCLUDE */


