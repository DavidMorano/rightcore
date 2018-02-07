/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rest "
#define	BANNER		"Rest "
#define	SEARCHNAME	"rest"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"REST_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"REST_BANNER"
#define	VARSEARCHNAME	"REST_NAME"
#define	VAROPTS		"REST_OPTS"
#define	VARFILEROOT	"REST_FILEROOT"
#define	VARLOGTAB	"REST_LOGTAB"
#define	VARMSFNAME	"REST_MSFILE"
#define	VARAFNAME	"REST_AF"
#define	VAREFNAME	"REST_EF"
#define	VARLFNAME	"REST_LF"

#define	VARDEBUGFNAME	"REST_DEBUGFILE"
#define	VARDEBUGFD1	"REST_DEBUGFD"
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

#define	PIDFNAME	"run/rest"		/* mutex PID file */
#define	LOGFNAME	"var/log/rest"		/* activity log */
#define	LOCKFNAME	"spool/locks/rest"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	USERFSUF	"user"

#define	LOGSIZE		(80*1024)

#define	TO_POLL		(8 * 60)


