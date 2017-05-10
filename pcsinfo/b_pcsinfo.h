/* config -- header defaults */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	P_PCSINFO	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)pcsinfo "
#define	BANNER		"PCS Info"
#define	SEARCHNAME	"pcsinfo"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"PCSINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSINFO_BANNER"
#define	VARSEARCHNAME	"PCSINFO_NAME"
#define	VARPROGMODE	"PCSINFO_MODE"
#define	VAROPTS		"PCSINFO_OPTS"
#define	VARMAILBOX	"PCSINFO_MAILBOX"
#define	VARAFNAME	"PCSINFO_AF"
#define	VAREFNAME	"PCSINFO_EF"
#define	VARERRORFNAME	"PCSINFO_ERRORFILE"

#define	VARDEBUGFNAME	"PCSINFO_DEBUGFILE"
#define	VARDEBUGFD1	"PCSINFO_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
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
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"

#define	HELPFNAME	"help"
#define	MBFNAME		"mbtab"
#define	WLFNAME		"whitelist"
#define	BLFNAME		"blacklist"

#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/pcsinfo.nodes"
#define	SPAMFNAME	"etc/pcsinfo.spam"
#define	LOGFNAME	"log/pcsinfo"
#define	USERFNAME	"log/pcsinfo.users"
#define	LOGFNAME	"log/pcsinfo"
#define	LOGENVFNAME	"log/pcsinfo.env"
#define	LOGZONEFNAME	"log/pcsinfo.zones"

#define	MSGIDDBNAME	"var/pcsinfo"

#define	MAILGROUP	"mail"
#define	MAILGNAME	"mail"

#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	FIELDLEN	4096

#define	LOGSIZE		(80*1024)

#define	MAILLOCKAGE	(5 * 60)

#define	DEFTIMEOUT	(10 * 60)

#define	TO_LOCK		(10 * 60)
#define	TO_MSGREAD	10

#define	MAXMSGID	490

#define	PORTSPEC_COMSAT		"biff"
#define	PORTSPEC_MAILPOLL	"mailpoll"


