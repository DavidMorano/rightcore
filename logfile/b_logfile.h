/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)logfile "
#define	BANNER		"LogFile"
#define	SEARCHNAME	"logfile"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOGFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGFILE_BANNER"
#define	VARSEARCHNAME	"LOGFILE_NAME"
#define	VAROPTS		"LOGFILE_OPTS"
#define	VARDBNAME	"LOGFILE_DB"
#define	VARLINELEN	"LOGFILE_LINELEN"
#define	VARFILEROOT	"LOGFILE_FILEROOT"
#define	VARLOGTAB	"LOGFILE_LOGTAB"
#define	VARLOGSIZE	"LOGFILE_LOGSIZE"
#define	VARMSFNAME	"LOGFILE_MSFILE"
#define	VARUTFNAME	"LOGFILE_UTFILE"
#define	VARLOGFNAME	"LOGFILE_LOGFILE"
#define	VARLFNAME	"LOGFILE_LF"
#define	VARAFNAME	"LOGFILE_AF"
#define	VAREFNAME	"LOGFILE_EF"

#define	VARDEBUGFNAME	"LOGFILE_DEBUGFILE"
#define	VARDEBUGFD1	"LOGFILE_DEBUGFD"
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

#define	REPORTDNAME	"/var/tmp/reports"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	SERIALFNAME	"serial"

#define	DBFNAME		"/etc/acct/logfiles"
#define	PIDFNAME	"run/logfile"		/* mutex PID file */
#define	LOGFNAME	"var/log/logfile"	/* activity log */
#define	LOCKFNAME	"spool/locks/logfile"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	DEFPROGFNAME	"/usr/bin/ksh"

#define	ADMINUSER	"admin"

#define	DISARGLEN	50

#define	LOGSIZE		(80*1024)
#define	INTFLUSH	4

#define	TO_READ		(5*60)


