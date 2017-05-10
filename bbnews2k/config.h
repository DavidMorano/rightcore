/* config - header defaults */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"2k"
#define	WHATINFO	"@(#)bbnews "
#define	BANNER		"Bulletin Board News"
#define	SEARCHNAME	"bbnews"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"BBNEWS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BBNEWS_BANNER"
#define	VARSEARCHNAME	"BBNEWS_NAME"
#define	VAROPTS		"BBNEWS_OPTS"
#define	VARQUERYTEXT	"BBNEWS_QUERYTEXT"
#define	VARAFNAME	"BBNEWS_AF"
#define	VAREFNAME	"BBNEWS_EF"
#define	VARNEWSGROUPS	"BBNEWS_NEWSGROUPS"
#define	VARERRORFNAME	"BBNEWS_ERRORFILE"
#define	VARNEWSDNAME	"BBNEWS_NEWSDIR"
#define	VARNEWSRC	"BBNEWS_NEWSRC"
#define	VARPMAILCHECK	"BBNEWS_MAILCHECK"

#define	VARDEBUGFNAME	"BBNEWS_DEBUGFILE"
#define	VARDEBUGFD1	"BBNEWS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARBBNEWSDNAME	"BBNEWSDIR"
#define	VARBBNEWSRC	"BBNEWSRC"
#define	VARBBBDS	"BBBDS"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARMAILCHECK	"MAILCHECK"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	VCNAME		"var"
#define	LOGCNAME	"log"

#define	TMPDNAME	"/tmp"
#define	NEWSDNAME	"spool/boards"
#define	MSGDNAME	"msgfile"
#define	KBDNAME		"share/kbdinfo"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	HELPFNAME	"help"
#define	SERIALFNAME	"serial"
#define	USERFNAME	"users"
#define	CMDMAPFNAME	"cmdmap"
#define	CMDHELPFNAME	"cmdhelp"
#define	DEFNEWSRC	".bbnewsrc"
#define	DESCFNAME	".desc"
#define	BBHOSTSFILE	"etc/bb/bbhosts"
#define	BBNOTUSFILE	"etc/bb/bbnames"

#define	MAILHOST	"mtgbcs"
#define	PCSUSERNAME	"pcs"
#define	USERFSUF	"users"

#define	PROG_MAILER	"pcsmail"
#define	PROG_EDITOR	"vi"
#define	PROG_METAMAIL	"metamail"
#define	PROG_BBPOST	"bbpost"
#define	PROG_PAGER	"pg"
#define	PROG_PRT	"prt-pcsmail"

/* what to print when we have "new messages" */
#define	QUERYTEXT	"New bulletin board news articles (%n) are available."

#define	LOGSIZE		(80 * 1024)
#define	DEFTERMLINES	24

/* the number of leading spaces on each line of output */
#define INDENT 		3

#define	NBULLETINS	1000	/* maximum number of bulletins per board */
#define	VMDMODE		0775
#define	SCANPERCENT	40
#define	MINSCANLINES	6
#define	FRAMELINES	3

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
#define	EMIT_NEXT	1		/* go to next article */
#define	EMIT_DONE	2
#define	EMIT_PREVIOUS	3		/* user wants to go back */
#define EMIT_QUIT	4		/* user specified quit */
#define	EMIT_SKIP	5
#define EMIT_SAVE	6

/* time-outs */
#define	TO_READ		5
#define	TO_CONFIG	30
#define	TO_CLOCK	25
#define	TO_MAILCHECK	(1*60)
#define	TO_INFO		4
#define	TO_LOCK		4

/* do we want to have "FASTSCAN" enabled by default? */
#define	OPT_FASTSCAN	1


