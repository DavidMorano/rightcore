/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#ifndef	INC_LOGFILE
#include	<logfile.h>
#endif


#define	LINELEN		256		/* size of input line */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#define	BUFLEN		8192
#define	CMDBUFLEN	(8 * MAXPATHLEN)

#define	POLLTIME	30000		/* poll timeout */

#define	SANITYFAILURES	5

#define	LOCKTIMEOUT	(5 * 60)


/* global data */

struct gflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	keepalive : 1 ;
	uint	quiet : 1 ;
	uint	sanity : 1 ;
	uint	remote : 1 ;
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
	char	*progname ;		/* program name */
	char	*version ;
	char	*programroot ;		/* program root directory */
	char	*logfname ;		/* log file name */
	char	*tmpdir ;		/* temporary directory */
	char	*netfname ;		/* net file name */
	char	*timestring ;		/* current time string (CTIME) */
} ;


struct worm {
	char	wormfname[MAXPATHLEN + 1] ;
	offset_t	offset ;
	int	wfd ;
	int	f_delete ;
} ;


struct jobinfo {
	bfile	*jfp ;
	int	f_remotedomain ;
	char	*nodename, *domainname ;
} ;


#endif /* DEFS_INCLUDE */


