/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)pcsuserinfo "
#define	BANNER		"PCS User Information"
#define	SEARCHNAME	"pcsuserinfo"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"PCSUSERINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSUSERINFO_BANNER"
#define	VARSEARCHNAME	"PCSUSERINFO_NAME"
#define	VAROPTS		"PCSUSERINFO_OPTS"
#define	VARFILEROOT	"PCSUSERINFO_FILEROOT"
#define	VARLOGTAB	"PCSUSERINFO_LOGTAB"
#define	VARMSFNAME	"PCSUSERINFO_MSFILE"
#define	VARUTFNAME	"PCSUSERINFO_UTFILE"
#define	VARAFNAME	"PCSUSERINFO_AF"
#define	VAREFNAME	"PCSUSERINFO_EF"

#define	VARDEBUGFNAME	"PCSUSERINFO_DEBUGFILE"
#define	VARDEBUGFD1	"PCSUSERINFO_DEBUGFD"
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
#define	NAMEFNAME	".name"
#define	FULLNAMEFNAME	".fullname"

#define	PIDFNAME	"run/pcsuserinfo"		/* mutex PID file */
#define	LOGFNAME	"log/pcsuserinfo"		/* activity log */
#define	LOCKFNAME	"spool/locks/pcsuserinfo"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


