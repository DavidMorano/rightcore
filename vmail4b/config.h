/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the global header file which is included into every source
	file.  The declarations here are defined in 'proginfo.c' (for the most
	part).


*******************************************************************************/


#define	VERSION		"4b"
#define	WHATINFO	"@(#)VMAIL "
#define	BANNER		"Visual Mail"
#define	SEARCHNAME	"vmail"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"VMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"VMAIL_BANNER"
#define	VARSEARCHNAME	"VMAIL_NAME"
#define	VAROPTS		"VMAIL_OPTS"
#define	VARCONF		"VMAIL_CONF"
#define	VARTMPDNAMEP	"VMAIL_TMPDIR"
#define	VARMAILDNAMEP	"VAMIL_MAILDIR"
#define	VARMAILUSERSP	"VMAIL_MAILUSERS"
#define	VARFOLDERDNAME1	"VAMIL_MAILFOLDERDIR"
#define	VARMAILBOXDEF1	"VMAIL_MAILBOXDEF"
#define	VARMAILBOXIN1	"VMAIL_MAILBOXIN"
#define	VARMAILCHECK1	"VMAIL_MAILCHECK"
#define	VARKEYBOARDP	"VMAIL_KEYBOARD"
#define	VARAFNAME	"VMAIL_AF"
#define	VAREFNAME	"VMAIL_EF"
#define	VARERRORFNAME	"VMAIL_ERRORFILE"

#define	VARDEBUGFNAME	"VMAIL_DEBUGFILE"
#define	VARDEBUGFD1	"VMAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARFOLDERDNAME2	"MAILFOLDERDIR"
#define	VARMAILUSERS	"MAILUSERS"
#define	VARMAILBOXDEF2	"MAILBOXDEF"
#define	VARMAILBOXIN2	"MAILBOXIN"
#define	VARMAILCHECK2	"MAILCHECK"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARTERM		"TERM"
#define	VARKEYBOARD	"KEYBOARD"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARHZ		"HZ"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define MAILDNAME	"/var/mail"
#define	LOGCNAME	"log"
#define	FOLDERDNAME	"mail"
#define	MSGDNAME	"msgfile"
#define	KBDNAME		"share/kbdinfo"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"
#define	HELPFNAME	"help"
#define	CMDMAPFNAME	"cmdmap"

#ifndef	CMDHELPFNAME
#define	CMDHELPFNAME	"lib/vmail/cmdhelp"
#endif

#define	USERFSUF	"users"

/* programs */

#define	PROG_SHELL	"ksh"
#define	PROG_GETMAIL	"pcsgetmail"
#define	PROG_EDITOR	"vi"
#define	PROG_MAILER	"pcsmail"
#define	PROG_METAMAIL	"metamail"
#define	PROG_PAGER	"vless"
#define	PROG_POSTSPAM	"postspam"

#ifndef	LOGSIZE
#define	LOGSIZE		(80*1024)
#endif

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

/* mail spool area stuff */

#define	MAILLOCKAGE	(5 * 60)

/* mail host stuff */

#define	MAILHOST	"mailhost"
#define	ORGDOMAIN	"lucent.com"

#ifndef	ORGANIZATION
#define	ORGANIZATION	"RichtCore Network Services"
#endif

/* local system stuff */

#ifndef	MAILGNAME
#define	MAILGNAME	"mail"
#endif

#ifndef MAILGID				/* default mail-gid */
#define MAILGID		6
#endif

/* name of default old mailbox */
#define OLDBOX		"old"

/* maximum number of messages in mailboxes */
#define	MAXMESS		9999

#define	VMDMODE		0700		/* msg-dir creation mode */

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#define	CMDBUFLEN	(2 * MAXPATHLEN)

/* max size of message in WSIZE-line pages */
#define	MAXPAGE		500		/* maximum number of pages */

#define	FIELDLEN	4000		/* header field value length */

#define	DEFTERMTYPE	"vt100"		/* default terminal types */

/* maximum number of terminal lines supported */
#define	TERMLINES_DEF	24		/* must not be less than seven (7)! */
#define	TERMLINES_MAX	128		/* any number you want! */

#define	SCANPERCENT	25		/* default percent of lines for SCAN */

/* terminal I/O definitions */

/* columns of fields: start with the number zero ('0'), not one ('1') */
#define	COL_NEWMAIL	2		/* "new-mail" indicator */
#define	COL_DATESTR	56		/* date string */
#define	COL_DELETE	0		/* mail-msg "delete" mark */
#define	COL_CUR		1		/* current mail-msg mark */
#define	COL_CURRENT	2		/* start of current mail-scan info */
#define	COL_MAILBOX	29		/* mailbox name */
#define	COL_NMSGS	2		/* number of messages info */
#define	COL_MOREMSGS	60		/* more-messages info */
#define	COL_MSGFROM	2		/* MSG-from */
#define	COL_MSGNUM	29		/* MSG-number */
#define	COL_MSGLINES	46		/* MSG-lines */
#define	COL_MSGMORE	60		/* more-lines info */

#define	COL_SCANFROM	2
#define	COL_SCANSUBJECT	29
#define	COL_SCANDATE	60
#define	COL_SCANLINES	76

/* time-outs */
#define	TO_CONFIG	30		/* configuration file */
#define	TO_CLOCK	3		/* clock-update */
#define	TO_READ		3		/* main read */
#define	TO_MAILCHECK	4		/* mail-check */
#define	TO_INFO		4
#define	TO_LOCK		4

#define	FRAMELINES	6		/* this is really fixed! */
#define	MINSCANLINES	2		/* minimum scan lines */
#define	MINVIEWLINES	2		/* minimum view lines */

#define	PO_MAILDIRS	"maildirs"
#define	PO_MAILUSERS	"mailusers"
#define	PO_OPTION	"option"

#define	DEFOPT_USECLEN		TRUE	/* use "clen" by default */
#define	DEFOPT_USECLINES	TRUE	/* use "clines" by default */
#define	DEFOPT_WINADJ		TRUE	/* adjust window-size dynamically */

#define	MB_DEFAULT	"new"
#define	MB_INPUT	"new"
#define	MB_SPAM		"spam"
#define	MB_TRASH	"trash"


