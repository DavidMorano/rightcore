/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>

#include	<localmisc.h>


#define	LINELEN		256		/* size of input line */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#define	BUFLEN		8192
#define	CMDBUFLEN	(8 * MAXPATHLEN)

#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2



/* program data */

struct proginfo_flags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	FILE	*efp ;
	char	*progname ;		/* program name */
	char	*version ;
	char	*programroot ;		/* program root directory */
	int	debuglevel ;		/* debugging level */
} ;


#endif /* DEFS_INCLUDE */


