/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvclogwelcome "
#define	BANNER		"Open-Service Organization"
#define	SEARCHNAME	"opensvclogwelcome"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCLOGWELCOME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCLOGWELCOME_BANNER"
#define	VARSEARCHNAME	"OPENSVCLOGWELCOME_NAME"
#define	VARFILEROOT	"OPENSVCLOGWELCOME_FILEROOT"
#define	VARLOGTAB	"OPENSVCLOGWELCOME_LOGTAB"
#define	VARMSFNAME	"OPENSVCLOGWELCOME_MSFILE"
#define	VARUTFNAME	"OPENSVCLOGWELCOME_UTFILE"
#define	VARERRORFNAME	"OPENSVCLOGWELCOME_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCLOGWELCOME_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCLOGWELCOME_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/opensvclogwelcome"
#define	LOGFNAME	"var/log/opensvclogwelcome"
#define	LOCKFNAME	"spool/locks/opensvclogwelcome"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


