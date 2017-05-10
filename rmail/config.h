/* config -- header defaults */


#define	P_RMAIL		1

#define	VERSION		"0"
#define	WHATINFO	"@(#)rmail "
#define	BANNER		"Remote Mail"

#define	VARPROGRAMROOT1	"RMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"RMAIL_NAME"
#define	VARPROGMODE	"RMAIL_MODE"
#define	VAROPTS		"RMAIL_OPTS"
#define	VARMAILBOX	"RMAIL_MAILBOX"

#define	VARDEBUGFD1	"RMAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	SEARCHNAME	"rmail"

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"

#define	HELPFNAME	"help"
#define	MBFNAME		"mbtab"
#define	WLFNAME		"whitelist"
#define	BLFNAME		"blacklist"

#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/rmail.nodes"
#define	SPAMFNAME	"etc/rmail.spam"
#define	LOGFNAME	"log/rmail"
#define	USERFNAME	"log/rmail.users"
#define	LOGFNAME	"log/rmail"
#define	LOGENVFNAME	"log/rmail.env"
#define	LOGZONEFNAME	"log/rmail.zones"
#define	MSGIDDBNAME	"var/rmail"

#define	MAILGNAME	"mail"
#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	LINELEN		256
#define	FIELDLEN	4096

#define	LOGSIZE		(80*1024)

#define	MAILLOCKAGE	(5 * 60)

#define	DEFTIMEOUT	(10 * 60)
#define	TO_LOCK		(10 * 60)
#define	TO_MSGREAD	10

#define	MAXMSGID	490

#define	PORTSPEC_COMSAT		"biff"
#define	PORTSPEC_MAILPOLL	"mailpoll"



