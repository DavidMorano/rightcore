/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)initinfo "
#define	BANNER		"Initialization Information"
#define	SEARCHNAME	"initinfo"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"INITINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"INITINFO_BANNER"
#define	VARSEARCHNAME	"INITINFO_NAME"
#define	VAROPTS		"INITINFO_OPTS"
#define	VARFILEROOT	"INITINFO_FILEROOT"
#define	VARLOGTAB	"INITINFO_LOGTAB"
#define	VARMSFNAME	"INITINFO_MSFILE"
#define	VAREFNAME	"INITINFO_EF"

#define	VARDEBUGFNAME	"INITINFO_DEBUGFILE"
#define	VARDEBUGFD1	"INITINFO_DEBUGFD"
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

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/initinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/initinfo"		/* activity log */
#define	LOCKFNAME	"spool/locks/initinfo"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4


