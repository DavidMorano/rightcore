/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)msinfo "
#define	BANNER		"Machine Satus Information"
#define	SEARCHNAME	"msinfo"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MSINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MSINFO_BANNER"
#define	VARSEARCHNAME	"MSINFO_NAME"
#define	VAROPTS		"MSINFO_OPTS"
#define	VARFILEROOT	"MSINFO_FILEROOT"
#define	VARLOGTAB	"MSINFO_LOGTAB"
#define	VARMSFNAME	"MSINFO_MSFILE"
#define	VARAFNAME	"MSINFO_AF"
#define	VAREFNAME	"MSINFO_EF"
#define	VAROFNAME	"MSINFO_OF"
#define	VARIFNAME	"MSINFO_IF"

#define	VARDEBUGFNAME	"MSINFO_DEBUGFILE"
#define	VARDEBUGFD1	"MSINFO_DEBUGFD"
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
#define	VCNAME		"var"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/msinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/msinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/msinfo"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	TO_POLL		8

#define	DEFNODES	50
#define	MAXNODES	(16 * 1024)


