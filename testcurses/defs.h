/* INCLUDE FILE defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<logfile.h>
#include	<pcsconf.h>
#include	<bfile.h>

#include	"termstore.h"
#include	"ds.h"


#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(2 * MAXPATHLEN)
#define	CMDBUFLEN	(2 * MAXPATHLEN)
#define	TZLEN		100


#define	NUO		4

#define	UOV_DEBUG	0
#define	UOV_BB		1


#define	NSTAT		4

#define	STATV_LOCKFAIL	0
#define	STATV_READONLY	1
#define	STATV_WINCHANGE 2
#define	STATV_INPUT	3
#define	STATV_SVR4	4
#define	STATV_630	5
#define	STATV_SYSVCT	6


struct proginfo_flags {
	uint	log : 1 ;
	uint	bb : 1 ;
	uint	sysv_rt : 1 ;
	uint	sysv_ct : 1 ;
	uint	verbose : 1 ;
} ;

struct proginfo {
	struct proginfo_flags	f ;
	ds		*dsp ;
	struct pcsconf	p ;
	logfile	lh ;
	struct termstore	ts ;
	bfile	*efp ;			/* error Basic file */
	bfile	*ofp ;			/* output Basic file */
	pid_t	pid ;
	time_t	daytime ;		/* system time of day */
	uid_t	uid ;
	gid_t	gid ;
	gid_t	gid_mail ;
	int	debuglevel ;		/* debugging level */
	int	tfd ;			/* terminal file descriptor */
	int	mailcheck ;		/* timer interval between mail checks */
	int	termlines ;		/* number of lines in terminal */
	int	f_winchange ;		/* interrupt semaphore */
	int	f_alarm ;		/* interrupt semaphore */
	char	stat[NSTAT] ;		/* program status */
	char	*pcs ;			/* PCS "root" directory */
	char	*domainname ;
	char	*nodename ;
	char	*username ;
	char	*homedir ;		/* user's HOME directory */
	char	*progname ;		/* program name */
	char	*termtype ;		/* user's terminal type */
	char	*folder ;		/* mail folder directory */
	char	*helpfname ;		/* help file path */
	char	*mbox ;			/* user's MBOX variable */
	char	*imbox ;		/* user specified "in" box */
	char	*mailuser ;		/* mail user box name in spool area */
	char	*spoolfile ;
	char	*gecosname ;
	char	*mailname ;
	char	*logid ; 		/* ID for logging purposes */
	char	*tmpdir ;		/* temporary directory */
	char	*prog_getmail ;
	char	*prog_mailer ;
	char	*prog_editor ;
	char	*prog_metamail ;
	char	tz[TZLEN + 1] ;		/* local time zone */
} ;


/* pointers to start of current invocation */

struct current {
	FILE	*fp ;		/* ioptr to its actual file */
	char	mailbox[MAXNAMELEN + 1] ;	/* name of current mailbox */
	int	msgno ;		/* current message pointer */
	int	pageno ;	/* current page in message */
	int	pages ;
} ;


#ifndef DEFS

/* message structure for random access into current mailbox */

extern struct current  curr ;

extern long	messbeg[] ;	/* byte ptrs to beginnings of messages */
extern long	messend[] ;	/* byte ptrs to endings of messages */

extern int	messord[]; 	/* message ordering (presort may reorder) */
extern int	messlen[] ;	/* message length in lines */
extern int	mlength[] ;	/* message length in bytes */
extern int	nummess ;	/* number of messages */


/* intermediate state during reading within a mailbox */

extern int   messdel[] ;	/* 1 marks a to-be-deleted message, else 0 */


/* global variables holding screen parameters */

extern int	cursline ;			/* line of --> cursor */
extern int	firstmsg ;			/* first message displayed */


/* global holding file pointers to pages */

extern long	pages[MAXPAGE] ;


/* global variables holding tokens for the logical expression search */

extern int	isop[] ;	/* 1 if token is operator, else 0 */
extern int	etoken[] ;	/* operator num  or  header num */
extern char	*hvalpt[] ;	/* ptr to value string of header in etoken */
extern char	hvalues[] ;	/* string space for value strings */
extern int	numletok ;	/* number of tokens */


/* logical expression tables */

extern char	operator[]  ;
extern char	*header[]  ;


/* non-integer function returns */
/* returns mailbox directory (in boxname.c) */

extern char	*maildir() ;
extern char	searchstr[]  ;


#endif /* DEFS_INCLUDE */


