/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)numbers "
#define	BANNER		"Numbers (various)"
#define	SEARCHNAME	"numbers"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"NUMBERS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"NUMBERS_BANNER"
#define	VARSEARCHNAME	"NUMBERS_NAME"
#define	VAROPTS		"NUMBERS_OPTS"
#define	VARFILEROOT	"NUMBERS_FILEROOT"
#define	VARLOGTAB	"NUMBERS_LOGTAB"
#define	VARMSFNAME	"NUMBERS_MSFILE"
#define	VARUTFNAME	"NUMBERS_UTFILE"
#define	VARAFNAME	"NUMBERS_AF"
#define	VAREFNAME	"NUMBERS_EF"

#define	VARDEBUGFNAME	"NUMBERS_DEBUGFILE"
#define	VARDEBUGFD1	"NUMBERS_DEBUGFD"
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

#define	PIDFNAME	"run/numbers"		/* mutex PID file */
#define	LOGFNAME	"var/log/numbers"		/* activity log */
#define	LOCKFNAME	"spool/locks/numbers"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)


