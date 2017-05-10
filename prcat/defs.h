/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<signal.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<bfile.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	BUFLEN
#define	BUFLEN		200
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


struct gflags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	noinput : 1 ;
	uint	log : 1 ;
} ;

struct global {
	char		*version ;
	char		*pwd ;
	char		*progdir ;
	char		*progname ;
	char		*pr ;
	char		*tmpdir ;
	char		*nodename, *domainname ;
	bfile		*efp ;
	bfile		*ofp ;
	logfile		lh ;
	struct gflags	f ;
	pid_t		pid ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


