/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)truncate "
#define	BANNER		"Truncate"
#define	SEARCHNAME	"truncate"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TRUNCATE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TRUNCATE_BANNER"
#define	VARSEARCHNAME	"TRUNCATE_NAME"
#define	VAROPTS		"TRUNCATE_OPTS"
#define	VARFILEROOT	"TRUNCATE_FILEROOT"
#define	VARLOGTAB	"TRUNCATE_LOGTAB"
#define	VARMSFNAME	"TRUNCATE_MSFILE"
#define	VARUTFNAME	"TRUNCATE_UTFILE"
#define	VARLFNAME	"TRUNCATE_xLAF"
#define	VARAFNAME	"TRUNCATE_AF"
#define	VAREFNAME	"TRUNCATE_EF"
#define	VARERRORFNAME	"TRUNCATE_ERRORFILE"

#define	VARDEBUGFNAME	"TRUNCATE_DEBUGFILE"
#define	VARDEBUGFD1	"TRUNCATE_DEBUGFD"
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

#define	PIDFNAME	"run/la"		/* mutex PID file */
#define	LOGFNAME	"var/log/la"		/* activity log */
#define	LOCKFNAME	"spool/locks/la"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2

#define	USAGECOLS	4


