/* config -- header defaults */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"4a"
#define	WHATINFO	"@(#)dmail "
#define	SEARCHNAME	"dmail"
#define	BANNER		"Deliver Mail"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"DMAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DMAIL_BANNER"
#define	VARSEARCHNAME	"DMAIL_NAME"
#define	VARPROGMODE	"DMAIL_MODE"
#define	VAROPTS		"DMAIL_OPTS"
#define	VARMAILDNAMEP	"DMAIL_MAILDIR"
#define	VARDMAILDNAMEP	"DMAIL_DEADMAILDIR"
#define	VARCOMSATFNAME	"DMAIL_COMSATCONF"
#define	VARCOMSATPORT	"DMAIL_COMSATPORT"
#define	VARMAILBOX	"DMAIL_MAILBOX"
#define	VARAFNAME	"DMAIL_AF"
#define	VAREFNAME	"DMAIL_EF"
#define	VARCFNAME	"DMAIL_CF"
#define	VARLFNAME	"DMAIL_LF"
#define	VARLOGFNAME	"DMAIL_LOGFILE"
#define	VARERRORFNAME	"DMAIL_ERRORFILE"

#define	VARDEBUGFNAME	"DMAIL_DEBUGFILE"
#define	VARDEBUGFD1	"DMAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARUSERNAME	"USERNAME"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUUMACHINE	"UU_MACHINE"
#define	VARUUUSER	"UU_USER"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARDMAILDNAME	"DEADMAILDIR"
#define	VARCMAILDNAME	"COPYMAILDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define MAILDNAME	"/var/mail"
#define DEADMAILDNAME	"/var/adm/deadmail"
#define COPYMAILDNAME	"/var/pcs/mail"
#define	VARDNAME	"var"
#define	LOGCNAME	"log"

#define	REPORTDNAME	"/var/tmp/reports"

#define	HELPFNAME	"help"
#define	MBFNAME		"mbtab"
#define	WLFNAME		"whitelist"
#define	BLFNAME		"blacklist"

#define	RMTABFNAME	"/etc/rmtab"
#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/%S.comsat"
#define	SPAMFNAME	"etc/%S.spam"
#define	LOGENVFNAME	"env"
#define	LOGZONEFNAME	"zones"

#define	MAILGNAME	"mail"
#define	MAILGID		6

#define	USERFSUF	"users"
#define	DIVERTUSER	"adm"
#define	SPAMUSER	"spam"

#define	LOGSIZE		(80*1024)

#define	DISARGLEN	50

#define	PO_MAILDIRS	"maildirs"

#define	TO_SPOOL	(1*60)
#define	TO_MSGREAD	10

#define	MAXMSGID	490

#define	PROTONAME_COMSAT	"udp"

#define	PORTSPEC_COMSAT		"biff"
#define	PORTSPEC_MAILPOLL	"mailpoll"

#define	EMA_POSTMASTER		"postmaster"

#define	PROTOSPEC_POSTFIX	"postfix"

#define	DEFMAILFOLDER	"mail"

#define	ADMINUSER	"admin"

#define	DEFBOXNAME	"new"

#define	OPT_LOGPROG	TRUE
#define	OPT_LOGMSG	TRUE
#define	OPT_LOGZONE	TRUE
#define	OPT_LOGENV	TRUE
#define	OPT_LOGMSGID	TRUE
#define	OPT_LOGSYS	TRUE
#define	OPT_DIVERT	TRUE
#define	OPT_FORWARD	FALSE
#define	OPT_MAILHIST	TRUE		/* what the hell is this? */
#define	OPT_NOREPEAT	TRUE
#define	OPT_NOSPAM	TRUE
#define	OPT_DELIVER	TRUE
#define	OPT_COPY	TRUE
#define	OPT_SPAMBOX	TRUE


