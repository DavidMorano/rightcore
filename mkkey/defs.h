/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<logfile.h>
#include	<bfile.h>


#define	LINELEN		2048		/* size of input line */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#define	BUFLEN		8192
#define	CMDBUFLEN	(8 * MAXPATHLEN)


struct proginfo_flags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	stderror : 1 ;
	uint	removelabel : 1 ;
	uint	wholefile : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	logfile	lh ;
	bfile	*efp ;			/* error Basic file */
	char	*version ;
	char	*progname ;		/* program name */
	char	*searchname ;
	char	*programroot ;		/* program root directory */
	char	*nodename ;
	char	*domainname ;
	char	*tmpdir ;		/* temporary directory */
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;
	int	minwordlen ;
	int	maxwordlen ;
	int	keys ;
	int	eigenwords ;		/* default EIGENWORDS */
} ;


#endif /* DEFS_INCLUDE */


