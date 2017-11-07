/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sysval "
#define	BANNER		"System Value"
#define	SEARCHNAME	"sysval"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SYSVAL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSVAL_BANNER"
#define	VARSEARCHNAME	"SYSVAL_NAME"
#define	VAROPTS		"SYSVAL_OPTS"
#define	VARFILEROOT	"SYSVAL_FILEROOT"
#define	VARLOGTAB	"SYSVAL_LOGTAB"
#define	VARMSFNAME	"SYSVAL_MSFILE"
#define	VARUTFNAME	"SYSVAL_UTFILE"
#define	VARAFNAME	"SYSVAL_AF"
#define	VAREFNAME	"SYSVAL_EF"
#define	VAROFNAME	"SYSVAL_OF"
#define	VARIFNAME	"SYSVAL_IF"

#define	VARDEBUGFNAME	"SYSVAL_DEBUGFILE"
#define	VARDEBUGFD1	"SYSVAL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARPLATFORM	"PLATFORM"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARHZ		"HZ"

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

#define	PIDFNAME	"run/sysval"		/* mutex PID file */
#define	LOGFNAME	"var/log/sysval"	/* activity log */
#define	LOCKFNAME	"spool/locks/sysval"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFINTRUN	60
#define	DEFINTPOLL	8
#define	DEFNODES	50
#define	DEFTTL		(5*60)

#define	TO_CACHE	3
#define	TO_TZ		(60*60)			/* time-out for TZ */

#define	USAGECOLS	4


