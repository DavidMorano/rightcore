/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mkboottime "
#define	BANNER		"Make BootTime"
#define	SEARCHNAME	"mkboottime"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MKBOOTTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKBOOTTIME_BANNER"
#define	VARSEARCHNAME	"MKBOOTTIME_NAME"
#define	VAROPTS		"MKBOOTTIME_OPTS"
#define	VARFILEROOT	"MKBOOTTIME_FILEROOT"
#define	VARLOGTAB	"MKBOOTTIME_LOGTAB"
#define	VARMSFNAME	"MKBOOTTIME_MSFILE"
#define	VARUTFNAME	"MKBOOTTIME_UTFILE"
#define	VARAFNAME	"MKBOOTTIME_AF"
#define	VAREFNAME	"MKBOOTTIME_EF"

#define	VARDEBUGFNAME	"MKBOOTTIME_DEBUGFILE"
#define	VARDEBUGFD1	"MKBOOTTIME_DEBUGFD"
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

#define	PIDFNAME	"run/mkboottime"		/* mutex PID file */
#define	LOGFNAME	"var/log/mkboottime"		/* activity log */
#define	LOCKFNAME	"spool/locks/mkboottime"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


