/* config */


/****************************************************************************

	this is the global header file which is included into every 
	source file.   the declarations here are defined in "defs.c".


******************************************************************************/


#define	VERSION		"3k"
#define	WHATINFO	"@(#)VMAIL "

#define	VARPROGRAMROOT1	"VMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"vmail"

#define	LOGFNAME	"log/vmail"
#define	USERFNAME	"log/vmail.users"
#define	CMDHELPFNAME	"lib/vmail/cmdhelp"


/* programs */

#define	PROG_GETMAIL	"getmail"
#define	PROG_EDITOR	"vi"
#define	PROG_MAILER	"pcsmail"
#define	PROG_METAMAIL	"metamail"


/* mail spool area stuff */

#define MAILDNAME	"/var/mail"
#define	MAILLOCKAGE	(5 * 60)


/* mail host stuff */

#define	MAILHOST	"mtgbcs.mt.lucent.com"
#define	ORGDOMAIN	"lucent.com"
#define	ORGANIZATION	"Lucent Technologies"


/* local system stuff */

#ifndef MAILGROUP
#define MAILGROUP	6
#endif


#define	DEFMAILBOX	"new"
#define	DEFMBOX		"mail/new"

#define	TMPDIR		"/tmp"

 
/* maximum number of messages in mailboxes */

#define	MAXMESS		5000


/* name of default old mailbox */

#define OLDBOX		"old"


/* max size of message in WSIZE-line pages */

#define	MAXPAGE		500		/* maximum number of pages */

#define	FIELDLEN	4000		/* header field value length */


/* maximum number of terminal lines supported */

#define	TERMLINES_DEF	24		/* must not be less than seven (7) ! */
#define	TERMLINES_MAX	128		/* any number you want ! */

#define	DEFTERMTYPE	"vt100"		/* default terminal types */


/* terminal I/O definitions */
/* note that columns start with the number zero ('0'), not one ('1') */

#define NEWCOL		2		/* col for new mail message */
#define COUNTCOL	2		/* col for message count */
#define DCOL		1		/* col for delete mark */
#define MORECOL 	29		/* col for MORE message */
#define TIMECOL 	50		/* col for time */

#define	COL_DELETE	0
#define	COL_CUR	1
#define	COL_NEWMAIL	2
#define	COL_CURRENT	2
#define	COL_MAILBOX	29
#define	COL_LINE	46
#define	COL_MORE	60
#define	COL_TIME	60
#define	COL_MESSAGES	55

#define WMIN 4				/* min size for message window */
#define WMAX 14				/* max size for message window */


/* globals having to do with mail checking */

#define CKTIME 47			/* time between mail checks */



