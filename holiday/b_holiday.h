/* config */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)holiday "
#define	BANNER		"Holiday"
#define	SEARCHNAME	"holiday"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"HOLIDAY_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HOLIDAY_BANNER"
#define	VARSEARCHNAME	"HOLIDAY_NAME"
#define	VAROPTS		"HOLIDAY_OPTS"
#define	VARDBNAME	"HOLIDAY_DB"
#define	VARLINELEN	"HOLIDAY_LINELEN"
#define	VARFILEROOT	"HOLIDAY_FILEROOT"
#define	VARLOGTAB	"HOLIDAY_LOGTAB"
#define	VARMSFNAME	"HOLIDAY_MSFILE"
#define	VARUTFNAME	"HOLIDAY_UTFILE"
#define	VARAFNAME	"HOLIDAY_AF"
#define	VAREFNAME	"HOLIDAY_EF"

#define	VARDEBUGFNAME	"HOLIDAY_DEBUGFILE"
#define	VARDEBUGFD1	"HOLIDAY_DEBUGFD"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	DBFNAME		"/etc/acct/holidays"
#define	PIDFNAME	"run/holiday"		/* mutex PID file */
#define	LOGFNAME	"var/log/holiday"	/* activity log */
#define	LOCKFNAME	"spool/locks/holiday"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	MAXDBENTRIES	32000		/* hack: assume as max DB entries */

#define	DEFPRECISION	5		/* default precision numbers */
#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50


