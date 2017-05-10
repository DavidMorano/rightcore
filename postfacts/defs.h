/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<bfile.h>
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





struct proginfo_flags {
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	log : 1 ;
	uint	no : 1 ;
} ;

struct proginfo {
	char		**envv ;
	char		*pwd ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*version ;
	char		*banner ;
	char		*searchname ;
	char		*tmpdname ;
	char		*helpfname ;
	char		*mailername ;
	char		*prog_rbbpost ;
	char		*prog_msgs ;
	char		*newsgroup ;
	char		*spooldname ;
	bfile		*efp ;
	struct proginfo_flags	f ;
	time_t		daytime ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */


