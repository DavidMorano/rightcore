/* config */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)nettime "
#define	BANNER		"Net TimeAverage"
#define	SEARCHNAME	"nettime"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"NETTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NETTIME_BANNER"
#define	VARSEARCHNAME	"NETTIME_NAME"
#define	VAROPTS		"NETTIME_OPTS"
#define	VARFILEROOT	"NETTIME_FILEROOT"
#define	VARLOGTAB	"NETTIME_LOGTAB"
#define	VARMSFNAME	"NETTIME_MSFILE"
#define	VARLOGFNAME	"NETTIME_LOGFNAME"
#define	VARLFNAME	"NETTIME_LF"
#define	VARAFNAME	"NETTIME_AF"
#define	VAREFNAME	"NETTIME_EF"
#define	VARERRORFNAME	"NETTIME_ERRORFILE"

#define	VARDEBUGFNAME	"NETTIME_DEBUGFILE"
#define	VARDEBUGFD1	"NETTIME_DEBUGFD"
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

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/nettime"		/* mutex PID file */
#define	LOGFNAME	"var/log/nettime"	/* activity log */
#define	LOCKFNAME	"spool/locks/nettime"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	TO_OPEN		6
#define	TO_READ		5

#define	INETSVC_TIME	"time"


