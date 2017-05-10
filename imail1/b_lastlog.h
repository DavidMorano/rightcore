/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)LASTLOG "
#define	BANNER		"Last Login"
#define	SEARCHNAME	"lastlog"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LASTLOG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LASTLOG_BANNER"
#define	VARSEARCHNAME	"LASTLOG_NAME"
#define	VAROPTS		"LASTLOG_OPTS"
#define	VARLFNAME	"LASTLOG_LF"
#define	VARAFNAME	"LASTLOG_AF"
#define	VAREFNAME	"LASTLOG_EF"
#define	VARDFNAME	"LASTLOG_DF"
#define	VARERRORFNAME	"LASTLOG_ERRORFILE"

#define	VARDEBUGFNAME	"LASTLOG_DEBUGFILE"
#define	VARDEBUGFD1	"LASTLOG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	LOGDNAME	"var/log"

#define	LOGCNAME	"log"

#define	CONFIGFNAME	"etc/lastlog/conf"
#define	LOGFNAME	"log/lastlog"
#define	USERFNAME	"log/lastlog.users"
#define	HELPFNAME	"help"
#define	LASTLOGFNAME	"/var/adm/lastlog"

#define	USERFSUF	"users"

#define	LOGSIZE		(80*1024)

#define	OPT_LOGPROG	TRUE


