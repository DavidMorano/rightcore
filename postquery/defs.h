/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<logfile.h>
#include	<bfile.h>



#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	4096
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	LINELEN		2048		/* size of input line */

#define	BUFLEN		8192



struct proginfo_flags {
	uint	log : 1 ;
	uint	quiet : 1 ;
	uint	stderror : 1 ;
	uint	removelabel : 1 ;
	uint	wholefile : 1 ;
} ;

struct proginfo {
	char	**envv ;
	char	*pwd ;
	char	*progdname ;		/* program directory */
	char	*progname ;		/* program name */
	char	*pr ;			/* program root directory */
	char	*version ;
	char	*banner ;
	char	*searchname ;
	char	*nodename ;
	char	*domainname ;
	char	*username ;
	char	*tmpdname ;		/* temporary directory */
	bfile	*efp ;			/* error Basic file */
	struct proginfo_flags	f ;
	logfile	lh ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;
	int	minwordlen ;
	int	maxwordlen ;
	int	keys ;
	int	eigenwords ;		/* default EIGENWORDS */
} ;

struct postentry {
	uint	noff ;
	uint	next ;
} ;


#endif /* DEFS_INCLUDE */


