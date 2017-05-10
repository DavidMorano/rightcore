/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rename "
#define	BANNER		"Rename (File)"
#define	SEARCHNAME	"rename"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"RENAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RENAME_BANNER"
#define	VARSEARCHNAME	"RENAME_NAME"
#define	VAROPTS		"RENAME_OPTS"
#define	VARFILEROOT	"RENAME_FILEROOT"
#define	VARLOGTAB	"RENAME_LOGTAB"
#define	VARMSFNAME	"RENAME_MSFILE"
#define	VARAFNAME	"RENAME_AF"
#define	VAREFNAME	"RENAME_EF"

#define	VARDEBUGFNAME	"RENAME_DEBUGFILE"
#define	VARDEBUGFD1	"RENAME_DEBUGFD"
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

#define	PIDFNAME	"run/rename"		/* mutex PID file */
#define	LOGFNAME	"var/log/rename"	/* activity log */
#define	LOCKFNAME	"spool/locks/rename"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFBASENAME	"xxx"
#define	ZOMBIEPREFIX	"zzz"


