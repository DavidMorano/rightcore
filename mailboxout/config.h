/* config - header defaults */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mailboxout "
#define	BANNER		"Mailbox Out"
#define	SEARCHNAME	"mailboxout"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"MAILBOXOUT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"MAILBOXOUT_NAME"
#define	VAROPTS		"MAILBOXOUT_OPTS"
#define	VARAFNAME	"MAILBOXOUT_AF"
#define	VAREFNAME	"MAILBOXOUT_EF"
#define	VARERRORFNAME	"MAILBOXOUT_ERRORFILE"

#define	VARDEBUGFNAME	"MAILBOXOUT_DEBUGFILE"
#define	VARDEBUGFD1	"MAILBOXOUT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"

#define	MSGIDDBNAME	"var/mailboxout"

#define	HELPFNAME	"help"
#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/mailboxout.nodes"
#define	SPAMFNAME	"etc/mailboxout.spam"
#define	LOGFNAME	"log/mailboxout"
#define	USERFNAME	"log/mailboxout.users"
#define	LOGFNAME	"log/mailboxout"
#define	LOGENVFNAME	"log/mailboxout.env"
#define	LOGZONEFNAME	"log/mailboxout.zones"

#define	MAILGROUP	"mail"

#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	FIELDLEN	4096
#define	DEFTIMEOUT	20
#define	MAILLOCKAGE	(5 * 60)

#define	MAXMSGID	490


