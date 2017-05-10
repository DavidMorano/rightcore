/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<userinfo.h>


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
#define	CMDBUFLEN	(8 * MAXPATHLEN)

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint	log : 1 ;
	uint	verbose : 1 ;
	uint	quiet : 1 ;
	uint	stderror : 1 ;
	uint	wantenvelope : 1 ;
	uint	seekable : 1 ;
	uint	postmark : 1 ;		/* "received" type of postmark */
} ;

struct proginfo {
	vecstr	stores ;
	cchar	**envv ;
	cchar	*pwd ;
	cchar	*progename ;
	cchar	*progdname ;
	cchar	*progname ;		/* program name */
	cchar	*pr ;			/* program root directory */
	cchar	*searchname ;
	cchar	*banner ;
	cchar	*version ;
	cchar	*rootname ;
	cchar	*nodename ;
	cchar	*domainname ;
	cchar	*username ;
	cchar	*groupname ;
	cchar	*tmpdname ;		/* temporary directory */
	cchar	*header_mailer ;	/* "x-mailer:" */
	cchar	*header_article ;
	cchar	*header_newsgroups ;
	cchar	*address_from ;
	cchar	*r_transport, *r_machine, *r_user ;
	struct userinfo	*up ;		/* user information */
	void		*efp ;		/* error Basic file */
	struct proginfo_flags	f ;
	logfile		lh ;
	int	pwdlen ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
} ;


#endif /* DEFS_INCLUDE */


