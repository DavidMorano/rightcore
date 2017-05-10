/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)logsys "
#define	BANNER		"LogSys"
#define	SEARCHNAME	"logsys"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOGSYS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGSYS_BANNER"
#define	VARSEARCHNAME	"LOGSYS_NAME"
#define	VAROPTS		"LOGSYS_OPTS"
#define	VARDBNAME	"LOGSYS_DB"
#define	VARLINELEN	"LOGSYS_LINELEN"
#define	VARFILEROOT	"LOGSYS_FILEROOT"
#define	VARLOGTAB	"LOGSYS_LOGTAB"
#define	VARMSFNAME	"LOGSYS_MSFILE"
#define	VARUTFNAME	"LOGSYS_UTFILE"
#define	VARFAC		"LOGSYS_FAC"
#define	VARPRI		"LOGSYS_PRI"
#define	VARLFNAME	"LOGSYS_LF"
#define	VARIFNAME	"LOGSYS_IF"
#define	VARAFNAME	"LOGSYS_AF"
#define	VAREFNAME	"LOGSYS_EF"

#define	VARDEBUGFNAME	"LOGSYS_DEBUGFILE"
#define	VARDEBUGFD1	"LOGSYS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VCNAME		"var"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	SERIALFNAME	"serial"

#define	DBFNAME		"/etc/acct/logsys"
#define	PIDFNAME	"run/logsys"		/* mutex PID file */
#define	LOGFNAME	"var/log/logsys"	/* activity log */
#define	LOCKFNAME	"spool/locks/logsys"	/* lock mutex file */
#define	MSFNAME		"ms"


