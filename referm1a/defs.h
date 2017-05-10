/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<logfile.h>
#include	<bfile.h>


#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#define	LINELEN		2048		/* size of input line */
#define	BIBLEN		2048
#define	BUFLEN		8192
#define	CMDBUFLEN	(8 * MAXPATHLEN)


struct gflags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	stderror : 1 ;
} ;

struct global {
	logfile	lh ;
	struct gflags	f ;
	bfile		*efp ;		/* error Basic file */
	int	debuglevel ;		/* debugging level */
	int	minwordlen ;
	int	maxwordlen ;
	int	keys ;
	int	eigenwords ;		/* default EIGENWORDS */
	char	*nodename ;
	char	*domainname ;
	char	*progname ;		/* program name */
	char	*programroot ;		/* program root directory */
	char	*version ;
	char	*tmpdir ;		/* temporary directory */
} ;


#endif /* DEFS_INCLUDE */


