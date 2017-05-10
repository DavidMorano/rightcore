/* config - header defaults */

#define	P_MAILPOLLD	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)mailpolld "
#define	SEARCHNAME	"mailpolld"
#define	BANNER		"Mail Poll Daemon"

#define	VARPROGRAMROOT1	"MAILPOLLD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"MAILPOLLD_NAME"
#define	VARPROGMODE	"MAILPOLLD_MODE"
#define	VAROPTS		"MAILPOLLD_OPTS"

#define	VARDEBUGFD1	"MAILPOLLD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"

#define	HELPFNAME	"help"
#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/mailpolld.nodes"
#define	SPAMFNAME	"etc/mailpolld.spam"
#define	LOGFNAME	"log/mailpolld"
#define	USERFNAME	"log/mailpolld.users"
#define	LOGFNAME	"log/mailpolld"
#define	LOGENVFNAME	"log/mailpolld.env"
#define	LOGZONEFNAME	"log/mailpolld.zones"

#define	MSGIDDBNAME	"var/mailpolld"

#define	MAILGROUP	"mail"

#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	DEFBOXDNAME	"mail"
#define	DEFBOXNAME	"spool"

#define	LOGSIZE		400000
#define	LINELEN		256
#define	FIELDLEN	4096
#define	DEFTIMEOUT	20
#define	MAILLOCKAGE	(5 * 60)

#define	MAXMSGID	490

#define	PORTSPEC_COMSAT		"comsat"
#define	PORTSPEC_MAILPOLL	"mailpoll"

#define	PROG_MAILPOLL		"/usr/add-on/pcs/sbin/mailpoll"



