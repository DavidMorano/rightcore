/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)conslog "
#define	BANNER		"Console Logger"
#define	SEARCHNAME	"conslog"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CONSLOG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CONSLOG_BANNER"
#define	VARSEARCHNAME	"CONSLOG_NAME"
#define	VAROPTS		"CONSLOG_OPTS"
#define	VARAFNAME	"CONSLOG_AF"
#define	VAREFNAME	"CONSLOG_EF"
#define	VARIFNAME	"CONSLOG_IF"
#define	VARDBNAME	"CONSLOG_DB"
#define	VARFAC		"CONSLOG_FAC"
#define	VARPRI		"CONSLOG_PRI"
#define	VARLINELEN	"CONSLOG_LINELEN"
#define	VARFILEROOT	"CONSLOG_FILEROOT"
#define	VARLOGTAB	"CONSLOG_LOGTAB"
#define	VARMSFNAME	"CONSLOG_MSFILE"
#define	VARUTFNAME	"CONSLOG_UTFILE"

#define	VARDEBUGFNAME	"CONSLOG_DEBUGFILE"
#define	VARDEBUGFD1	"CONSLOG_DEBUGFD"
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

#define	DBFNAME		"/etc/acct/conslogs"
#define	PIDFNAME	"run/conslog"		/* mutex PID file */
#define	LOGFNAME	"var/log/conslog"	/* activity log */
#define	LOCKFNAME	"spool/locks/conslog"	/* lock mutex file */
#define	MSFNAME		"ms"


