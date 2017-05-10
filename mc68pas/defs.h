/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)
#define	NLENV		40
#define	ENVLEN		2000


#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#define	CMDBUFLEN	(2 * MAXPATHLEN)



/* global data */

struct proginfo_flags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	cpp : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	logfile	lh ;		/* program activity log */
	logfile	eh ;		/* error log */
	pid_t	pid ;
	int	debuglevel ;		/* debugging level */
	char	*progname ;		/* program name */
	char	*programroot ;		/* program root directory */
} ;


#endif /* DEFS_INCLUDE */


