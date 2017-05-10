/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)netload "
#define	BANNER		"Network Load"
#define	SEARCHNAME	"netload"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"NETLOAD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NETLOAD_BANNER"
#define	VARSEARCHNAME	"NETLOAD_NAME"
#define	VAROPTS		"NETLOAD_OPTS"
#define	VARFILEROOT	"NETLOAD_FILEROOT"
#define	VARLOGTAB	"NETLOAD_LOGTAB"
#define	VARMSFNAME	"NETLOAD_MSFILE"
#define	VARDBFNAME	"NETLOAD_DBFILE"
#define	VARAFNAME	"NETLOAD_AF"
#define	VAREFNAME	"NETLOAD_EF"

#define	VARDEBUGFNAME	"NETLOAD_DEBUGFILE"
#define	VARDEBUGFD1	"NETLOAD_DEBUGFD"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/netload"		/* mutex PID file */
#define	LOGFNAME	"var/log/netload"	/* activity log */
#define	LOCKFNAME	"spool/locks/netload"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2
#define	TO_LOADAVG	1

#define	USAGECOLS	4


