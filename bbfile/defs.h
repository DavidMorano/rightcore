/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<dirent.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<date.h>

#include	"localmisc.h"



/* standard stuff */

#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* program execution modes */

#define	PM_READ		0
#define	PM_HEADER	1
#define	PM_NAMES	3
#define	PM_COUNT	4
#define	PM_SUBSCRIPTION	5
#define	PM_OVERLAST	6


/* Header Indices */

#define	HI_SUBJECT	0
#define	HI_DATE		1
#define	HI_FROM		2
#define	HI_CONTENTTYPE	3
#define	HI_ARTICLEID	4
#define	HI_MSGID	5


/* Save Modes */

#define	SMODE_MAILBOX	0
#define	SMODE_OUT	1


/* miscellaneous */

#define COLS		80
#define	DATE1970	((24 * 10) * 3600)

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#define BUFSIZE		((2*MAXPATHLEN) + 4)
#define	BUFLEN		BUFSIZE
#define	LINELEN		200
#define	CMDBUFLEN	((2 * MAXPATHLEN) + 100)
#define	LOGIDLEN	32
#define	JOBIDLEN	32


/* article currency modes */

#define CM_OLD 		0		/* old articles only */
#define CM_NEW 		1		/* news articles only (default) */
#define CM_ALL 		2		/* all articles */


/* sort modes */

#define	SORTMODE_MTIME		0		/* modification time */
#define	SORTMODE_ATIME		1		/* arrival time */
#define	SORTMODE_PTIME		2		/* post time */
#define	SORTMODE_CTIME		3		/* compose time */
#define	SORTMODE_NOW		4		/* now */
#define	SORTMODE_SUPPLIED	5		/* supplied */
#define	SORTMODE_SPOOL		SORTMODE_MTIME	/* spool time */


/* emit subroutine returns */

#define	EMIT_OK		0		/* take normal action */
#define	EMIT_NEXT	0		/* go to next article */
#define	EMIT_BAD	-1		/* unspecified error occurred */
#define	EMIT_DONE	-2
#define	EMIT_PREVIOUS	-3		/* user wants to go back */
#define EMIT_QUIT	-4		/* user specified quit */
#define	EMIT_SKIP	-5
#define EMIT_SAVE	-6



/* the following items should not be printed */

struct proginfo_flags {
	uint	quiet : 1 ;
	uint	log : 1 ;
	uint	exit : 1 ;
	uint	terminal : 1 ;
	uint	ansiterm : 1 ;
	uint	popscreen : 1 ;
	uint	old : 1 ;
	uint	all : 1 ;
	uint	every : 1 ;
	uint	reverse : 1 ;
	uint	newprogram : 1 ;
	uint	interactive : 1 ;
	uint	mailbox : 1 ;
	uint	catchup : 1 ;
	uint	nopage : 1 ;
	uint	sysv_rt : 1 ;
	uint	sysv_ct : 1 ;
	uint	combine : 1 ;
	uint	count : 1 ;		/* not used */
	uint	extrascan : 1 ;
	uint	query : 1 ;
	uint	newmessages : 1 ;
	uint	subscribe : 1 ;		/* subscriptio/unsubscription mode */
	uint	description : 1 ;
	uint	header : 1 ;		/* listing header values */
	uint	readtime : 1 ;		/* readable user file time format */
	uint	addenv : 1 ;
	uint	envdate : 1 ;
	uint	envfrom : 1 ;
} ;

struct proginfo {
	char		**envv ;
	char		*version ;
	char		*pwd ;
	char		*progdir ;
	char		*progname ;
	char		*searchname ;
	char		*pr ;			/* program root */
	char		*banner ;
	char		*tmpdir ;
	char		*nodename ;
	char		*domainname ;
	char		*jobid ;
	char		*newsdir ;
	char		*helpfname ;
	char		*prog_editor ;
	char		*prog_mailer ;
	char		*prog_metamail ;
	char		*prog_bbpost ;
	char		*prog_pager ;
	char		*prog_print ;
	char		*prefix ;
	char		*termtype ;
	char		*mailhost ;
	char		*fromnode ;
	char		*querytext ;
	char		*envfrom ;
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	struct userinfo	*uip ;
	struct proginfo_flags	f ;
	struct timeb	now ;
	logfile		lh ;
	DATE		envdate, tmpdate ;
	int		pid ;
	int		progmode ;
	int		debuglevel ;
	int		verboselevel ;
	int		termlines ;
	int		showlines ;
	int		f_exit ;
	int		sortmode ;		/* sorting mode used */
	int		header ;		/* list this header type */
	int		whichenvdate ;		/* which date for envelope */
	char		zname[DATE_ZNAMESIZE + 1] ;
} ;


/* this is the in-memory version of the newly created user's list file */

struct userstat {
	struct dirstat	*dsp ;
	time_t		mtime ;
} ;


#if	defined(BSD)
#define	MAP_FAILED	((void *) (-1))
#endif


#endif /* DEFS_INCLUDE */


