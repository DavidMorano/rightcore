/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sanity "
#define	BANNER		"Sanity "
#define	SEARCHNAME	"sanity"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SANITY_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SANITY_BANNER"
#define	VARSEARCHNAME	"SANITY_NAME"
#define	VAROPTS		"SANITY_OPTS"
#define	VARFILEROOT	"SANITY_FILEROOT"
#define	VARLOGTAB	"SANITY_LOGTAB"
#define	VARMSFNAME	"SANITY_MSFILE"
#define	VARAFNAME	"SANITY_AF"
#define	VAREFNAME	"SANITY_EF"

#define	VARDEBUGFNAME	"SANITY_DEBUGFILE"
#define	VARDEBUGFD1	"SANITY_DEBUGFD"
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
#define	VARCOLUMNS	"COLUMNS"

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

#define	PIDFNAME	"run/la"		/* mutex PID file */
#define	LOGFNAME	"var/log/la"		/* activity log */
#define	LOCKFNAME	"spool/locks/la"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFINDENT	2
#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50


