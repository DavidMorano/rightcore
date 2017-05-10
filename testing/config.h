/* config */


/****************************************************************************

	This is the global header file which is included into every 
	source file.  The declarations here are defined in 'defs.c'
	(for the most part).


******************************************************************************/


#define	VERSION		"4"
#define	WHATINFO	"@(#)VMAIL "
#define	BANNER		"Visual Mail"
#define	SEARCHNAME	"vmail"

#define	VARPROGRAMROOT1	"VMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VAROPTS		"VMAIL_OPTS"
#define	VARSEARCHNAME	"VMAIL_NAME"
#define	VARERRORFNAME	"VMAIL_ERRORFILE"

#define	VARDEBUGFNAME	"VMAIL_DEBUGFILE"
#define	VARDEBUGFD1	"VMAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME1	"VMAIL_TMPDIR"
#define	VARMAILDNAME1	"VAMIL_MAILDIR"
#define	VARFOLDERDNAME1	"VAMIL_MAILFOLDERDIR"
#define	VARMAILUSER1	"VMAIL_MAILUSERNAME"
#define	VARMAILBOXDEF1	"VMAIL_MAILBOXDEF"
#define	VARMAILBOXIN1	"VMAIL_MAILBOXIN"
#define	VARMAILCHECK1	"VMAIL_MAILCHECK"

#define	VARTMPDNAME2	"TMPDIR"
#define	VARMAILDNAME2	"MAILDIR"
#define	VARFOLDERDNAME2	"MAILFOLDERDIR"
#define	VARMAILUSER2	"MAILUSERNAME"
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
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARTERM		"TERM"
#define	VARLINES	"LINES"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define MAILDNAME	"/var/mail"
#define	LOGDNAME	"log"
#define	FOLDERDNAME	"mail"
#define	MSGDNAME	"msgfile"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/vmail"
#define	USERFNAME	"log/vmail.users"
#ifndef	CMDHELPFNAME
#define	CMDHELPFNAME	"lib/vmail/cmdhelp"
#endif

/* programs */

#define	PROG_SHELL	"ksh"
#define	PROG_GETMAIL	"getmail"
#define	PROG_EDITOR	"vi"
#define	PROG_MAILER	"pcsmail"
#define	PROG_METAMAIL	"metamail"
#define	PROG_PAGER	"less"

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

/* mail spool area stuff */

#define	MAILLOCKAGE	(5 * 60)

/* mail host stuff */

#define	MAILHOST	"mailhost"
#define	ORGDOMAIN	"lucent.com"
#define	ORGANIZATION	"Lucent Technologies"

/* local system stuff */

#ifndef	MAILGNAME
#define	MAILGNAME	"mail"
#endif

#ifndef MAILGID
#define MAILGID		6
#endif

#define	MBNAME_DEF	"new"
#define	MBNAME_IN	"new"
 
/* name of default old mailbox */
#define OLDBOX		"old"

/* maximum number of messages in mailboxes */
#define	MAXMESS		9999

#define	VMDMODE		0700		/* msg-dir creation mode */

/* max size of message in WSIZE-line pages */
#define	MAXPAGE		500		/* maximum number of pages */

#define	FIELDLEN	4000		/* header field value length */

#define	DEFTERMTYPE	"vt100"		/* default terminal types */

/* maximum number of terminal lines supported */
#define	TERMLINES_DEF	24		/* must not be less than seven (7)! */
#define	TERMLINES_MAX	128		/* any number you want! */

#define	SCANPERCENT	25		/* default percent of lines for SCAN */

/* terminal I/O definitions */
/* note that columns start with the number zero ('0'), not one ('1') */

#define	COL_NEWMAIL	2		/* "new-mail" indicator */
#define	COL_DATESTR	56		/* date string */
#define	COL_DELETE	0		/* mail-msg "delete" mark */
#define	COL_CUR	1		/* current mail-msg mark */
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

#define	TO_READ		3		/* main read */
#define	TO_CHECKTIME	3		/* time-update */
#define	TO_CHECKMAIL	4		/* mail-check */

#define	FRAMELINES	6		/* this is really fixed! */
#define	MINSCANLINES	2		/* minimum scan lines */
#define	MINVIEWLINES	2		/* minimum view lines */

#define WMIN 4				/* min size for message window */
#define WMAX 14				/* max size for message window */



