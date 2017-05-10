/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mbproc "
#define	BANNER		"MailBox Process"
#define	SEARCHNAME	"mbproc"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MBPROC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MBPROC_BANNER"
#define	VARSEARCHNAME	"MBPROC_NAME"
#define	VAROPTS		"MBPROC_OPTS"
#define	VARFILEROOT	"MBPROC_FILEROOT"
#define	VARLOGTAB	"MBPROC_LOGTAB"
#define	VARMSFNAME	"MBPROC_MSFILE"
#define	VARNSHOW	"MBPROC_NSHOW"
#define	VARMAILUSERSP	"MBPROC_MAILUSERS"
#define	VARMAILDNAMESP	"MBPROC_MAILDIRS"
#define	VARAFNAME	"MBPROC_AF"
#define	VAREFNAME	"MBPROC_EF"
#define	VARCFNAME	"MBPROC_CF"
#define	VARLFNAME	"MBPROC_LF"

#define	VARDEBUGFNAME	"MBPROC_DEBUGFILE"

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
#define	VARMAILUSERS	"MAILUSERS"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MAILDNAME	"/var/mail"
#define	MSDNAME		"var"
#define	LOGCNAME	"log"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/%S"
#define	LOCKDNAME	"var/spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"mbproc"		/* mutex PID file */
#define	LOGFNAME	"mbproc"		/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	INTRUN		60
#define	INTPOLL		8
#define	INTMARK		(1 * 24 * 3600)

#define	TO_MAILLOCK	(5 * 60)

#define	YEARTIMEBASE	1990


