/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)background "
#define	BANNER		"Background"
#define	SEARCHNAME	"background"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BACKGROUND_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BACKGROUND_BANNER"
#define	VARSEARCHNAME	"BACKGROUND_NAME"
#define	VARFILEROOT	"BACKGROUND_FILEROOT"
#define	VARLOGTAB	"BACKGROUND_LOGTAB"
#define	VARMSFNAME	"BACKGROUND_MSFILE"
#define	VARERRFIL	"BACKGROUND_ERRFILE"
#define	VARAFNAME	"BACKGROUND_AF"
#define	VAREFNAME	"BACKGROUND_EF"

#define	VARDEBUGFNAME	"BACKGROUND_DEBUGFILE"
#define	VARDEBUGFD1	"BACKGROUND_DEBUGFD"
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

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
#define	MSDNAME		"var"
#define	LOGCNAME	"log"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/background"		/* NOT USED ! */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"background"			/* mutex PID file */
#define	LOGFNAME	"background"			/* activity log */

#define	LOCKFNAME	"spool/locks/background"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFMARKINT	(8 * 24 * 3600)

#define	TO_LOCK		4
#define	TO_SPEED	(24 * 3600)


