/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)termenq "
#define	BANNER		"Terminal Enquire"
#define	SEARCHNAME	"termenq"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	VARPROGRAMROOT1	"TERMENQ_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TERMENQ_BANNER"
#define	VARSEARCHNAME	"TERMENQ_NAME"
#define	VAROPTS		"TERMENQ_OPTS"
#define	VARFILEROOT	"TERMENQ_FILEROOT"
#define	VARLOGTAB	"TERMENQ_LOGTAB"
#define	VARMSFNAME	"TERMENQ_MSFILE"
#define	VARUTFNAME	"TERMENQ_UTFILE"
#define	VARTERMLINE	"TERMENQ_LINE"
#define	VARTERMDB	"TERMENQ_DB"
#define	VARAFNAME	"TERMENQ_AF"
#define	VAREFNAME	"TERMENQ_EF"

#define	VARDEBUGFNAME	"TERMENQ_DEBUGFILE"
#define	VARDEBUGFD1	"TERMENQ_DEBUGFD"
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

#define	PIDFNAME	"run/termenq"		/* mutex PID file */
#define	LOGFNAME	"var/log/termenq"	/* activity log */
#define	LOCKFNAME	"spool/locks/termenq"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	TO_OPEN		2
#define	TO_READ		2

#define	OPT_LOGPROG	TRUE


