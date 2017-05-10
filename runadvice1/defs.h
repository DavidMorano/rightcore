/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<vecelem.h>


#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	ARGINFO		struct arginfo


struct machflags {
	uint		rstat:1 ;
	uint		havedisk:1 ;
	uint		checkeddisk:1 ;
	uint		freepages:1 ;
	uint		freeswap:1 ;
	uint		rsh:1 ;
	uint		sarfailed:1 ;
} ;

struct machine {
	struct machflags	f ;
	struct ustatstime	*sp ;
	char		*hostname ;
	time_t		timeout ;
	long		maxloadaverage ;
	long		minloadaverage ;
	int		freepages ;
	int		freeswap ;
} ;


typedef vecelem		machinehead ;


/* global stuff */

struct gflags {
	uint		debug:1 ;
	uint		verbose:1 ;
	uint		machines:1 ;
	uint		sysv_ct:1 ;
} ;

struct global {
	struct gflags		f ;
	logfile		lh ;
	struct paramname	*plist ;
	const char	**envv ;
	const char	*programroot ;
	const char	*cwd ;			/* current working directory */
	const char	*progname ;		/* program name */
	const char	*tmpdir ;		/* temporary directory */
	const char	*logfname ;		/* log file name */
	const char	*logid ;		/* log file ID */
	const char	*prog_advice ;		/* ADVICE program */
	const char	*runfname ;		/* ADVICE "run" file */
	const char	*confname ;		/* ADVICE "control" file */
	const char	*paramfname ;		/* ADVICE "params" file */
	const char	*mainfname ;		/* ADVICE "main" file */
	const char	*maifname ;		/* ADVICE "main" file */
	const char	*outfname ;		/* ADVICE "output" file */
	const char	*ofname ;
	bfile		*efp ;			/* error Basic file */
	bfile		*ofp ;			/* output Basic file */
	int		debuglevel ;		/* debug level */
	int		nparams ;		/* number of paramaters */
	volatile int	f_signal ;		/* signal was caught */
	volatile int	f_pipe ;		/* bad pipe write signal */
} ;


struct runflags {
	uint		local:1 ;		/* is machine the local one? */
	uint		remotetmp:1 ;		/* remote TMP was used */
} ;

struct run {
	struct runflags	f ;
	pid_t		pid ;
	int		slave ;
	char		*hostname ;
} ;


/* internally used defines */

#define PR_OK		0		/* general OK return */
#define PR_BAD		-1		/* general bad return */

#define	GC_BADPATH	-1001		/* no full path of filter file */
#define	GC_TOOMANY	-1002		/* too many filter entries */
#define	GC_TOOMUCH	-1003		/* ran out of string space */
#define	GC_MAXTMP	-1004		/* maximum temporary file attempts */

#define	PR_UNKNOWN	-1005		/* unknown key string encountered */
#define	PR_BADFILTER	-1006		/* bad filter file */
#define	PR_BADNOKEY	-1007		/* bad key macro encountered */


#endif /* DEFS_INCLUDE */


