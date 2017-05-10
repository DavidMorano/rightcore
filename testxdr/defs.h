/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>


#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(2 * MAXPATHLEN)

/* program exit status values */

#define	ES_OK		0		/* completed successfully */
#define	ES_BADARG	1		/* bad argument on invocation */
#define	ES_INFO		2		/* information only */
#define	ES_ERROR	3		/* error during processing */
#define	ES_MUTEX	4		/* mutual exclusion conflict */


struct gflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
} ;


struct global {
	struct gflags	f ;
	bfile	*efp ;			/* error Basic file */
	pid_t	pid ;
	uid_t	uid ;
	gid_t	gid ;
	int	debuglevel ;		/* debugging level */
	char	*programroot ;			/* PCS "root" directory */
	char	*domainname ;
	char	*nodename ;
	char	*username ;
	char	*progname ;		/* program name */
	char	*helpfname ;		/* help file path */
	char	*tmpdir ;		/* temporary directory */
} ;


#endif /* DEFS_INCLUDE */


