/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#define	LINELEN		256		/* size of input line */
#define	BUFLEN		4096
#define	BUFSIZE		BUFLEN
#define	SAVESIZE	128


struct gflags {
	uint	sysvct : 1 ;
	uint	sysvrt : 1 ;
	uint	log : 1 ;
} ;

struct global {
	struct gflags	f ;
	bfile	*efp ;			/* error Basic file */
	bfile	*ofp ;			/* output Basic file */
	logfile	lh ;
	pid_t	pid ;
	time_t	daytime ;		/* system time of day */
	int	uid ;
	int	gid ;
	int	debuglevel ;
	int	f_alarm ;		/* interrupt semaphore */
	char	*programroot ;		/* program root directory */
	char	*logfname ;		/* the log file name */
	char	*logid ;		/* log ID for log file entries */
	char	*progname ;		/* program name */
	char	*timestring ;		/* current time string (CTIME) */
	char	*mailhost ;
} ;



/* possible transport mechanisms */

#define	TRANS_NONE	0
#define	TRANS_RSH	1
#define	TRANS_REXEC	2
#define	TRANS_RSLOW	3


static char	*trans_name[] = {
	"NONE",
	"RSH",
	"REXEC",
	"RSLOW",
	NULL
} ;


#endif /* DEFS_INCLUDE */


