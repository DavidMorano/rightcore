/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)factorial "
#define	BANNER		"Fibonacci Function"
#define	SEARCHNAME	"factorial"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FACTORIAL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FACTORIAL_BANNER"
#define	VARSEARCHNAME	"FACTORIAL_NAME"
#define	VARFILEROOT	"FACTORIAL_FILEROOT"
#define	VARLOGTAB	"FACTORIAL_LOGTAB"
#define	VARMSFNAME	"FACTORIAL_MSFILE"
#define	VARUTFNAME	"FACTORIAL_UTFILE"
#define	VARAFNAME	"FACTORIAL_AF"
#define	VAREFNAME	"FACTORIAL_EF"

#define	VARDEBUGFNAME	"FACTORIAL_DEBUGFILE"
#define	VARDEBUGFD1	"FACTORIAL_DEBUGFD"
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

#define	PIDFNAME	"run/factorial"		/* mutex PID file */
#define	LOGFNAME	"var/log/factorial"		/* activity log */
#define	LOCKFNAME	"spool/locks/factorial"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)


