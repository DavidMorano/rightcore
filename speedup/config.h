/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)speedup"
#define	BANNER		"Speedup "
#define	SEARCHNAME	"speedup"
#define	VARPRNAME	"LOCAL"

#define	BANNER_ASUM	"asum"
#define	BANNER_AMEAN	"amean"
#define	BANNER_HMEAN	"hmean"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SPEEDUP_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SPEEDUP_BANNER"
#define	VARSEARCHNAME	"SPEEDUP_NAME"
#define	VAROPTS		"SPEEDUP_OPTS"
#define	VARFILEROOT	"SPEEDUP_FILEROOT"
#define	VARLOGTAB	"SPEEDUP_LOGTAB"
#define	VARMSFNAME	"SPEEDUP_MSFILE"
#define	VARAFNAME	"SPEEDUP_AF"
#define	VAREFNAME	"SPEEDUP_EF"
#define	VARLFNAME	"SPEEDUP_LF"

#define	VARDEBUGFNAME	"SPEEDUP_DEBUGFILE"
#define	VARDEBUGFD1	"SPEEDUP_DEBUGFD"
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

#define	PIDFNAME	"run/speedup"
#define	LOGFNAME	"var/log/speedup"
#define	LOCKFNAME	"spool/locks/speedup"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_POLL		(8 * 60)

#define	USERFSUF	"user"


