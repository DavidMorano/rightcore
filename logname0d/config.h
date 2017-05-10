/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)logname "
#define	BANNER		"Rest "
#define	SEARCHNAME	"logname"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOGNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGNAME_BANNER"
#define	VARSEARCHNAME	"LOGNAME_NAME"
#define	VAROPTS		"LOGNAME_OPTS"
#define	VARFILEROOT	"LOGNAME_FILEROOT"
#define	VARLOGTAB	"LOGNAME_LOGTAB"
#define	VARMSFNAME	"LOGNAME_MSFILE"
#define	VARAFNAME	"LOGNAME_AF"
#define	VAREFNAME	"LOGNAME_EF"
#define	VARLFNAME	"LOGNAME_LF"

#define	VARDEBUGFNAME	"LOGNAME_DEBUGFILE"
#define	VARDEBUGFD1	"LOGNAME_DEBUGFD"
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

#define	PIDFNAME	"run/logname"
#define	LOGFNAME	"var/log/logname"
#define	LOCKFNAME	"spool/locks/logname"
#define	MSFNAME		"ms"

#define	USERFSUF	"user"

#define	LOGSIZE		(80*1024)

#define	TO_POLL		(8 * 60)

#define	USAGECOLS	4


