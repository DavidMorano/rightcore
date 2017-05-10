/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


#define	LINELEN		256		/* size of input line */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#define	BUFLEN		8192
#define	CMDBUFLEN	(2 * MAXPATHLEN)

#define	POLLTIME	30000		/* poll timeout */


struct gflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	keepalive : 1 ;
	uint	quiet : 1 ;
	uint	sanity : 1 ;
} ;

struct global {
	logfile	lh ;
	struct gflags	f ;
	bfile		*efp ;		/* error Basic file */
	time_t	daytime ;		/* system time of day */
	time_t	keeptime ;		/* keepalive timeout */
	int	debuglevel ;		/* debugging level */
	uid_t	uid ;			/* real UID */
	uid_t	euid ;			/* effective UID */
	char	*tmpdir ;		/* temporary directory */
	char	*programroot ;		/* program root directory */
	char	*logfname ;		/* log file name */
	char	*netfname ;		/* net file name */
	char	*progname ;		/* program name */
	char	*timestring ;		/* current time string (CTIME) */
} ;


#endif /* DEFS_INCLUDE */


