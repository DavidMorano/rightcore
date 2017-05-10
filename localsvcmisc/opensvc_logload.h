/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvclogload "
#define	BANNER		"Open-Service Organization"
#define	SEARCHNAME	"opensvclogload"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCLOGLOAD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCLOGLOAD_BANNER"
#define	VARSEARCHNAME	"OPENSVCLOGLOAD_NAME"
#define	VARFILEROOT	"OPENSVCLOGLOAD_FILEROOT"
#define	VARLOGTAB	"OPENSVCLOGLOAD_LOGTAB"
#define	VARMSFNAME	"OPENSVCLOGLOAD_MSFILE"
#define	VARUTFNAME	"OPENSVCLOGLOAD_UTFILE"
#define	VARERRORFNAME	"OPENSVCLOGLOAD_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCLOGLOAD_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCLOGLOAD_DEBUGFD"
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

#define	PIDFNAME	"run/opensvclogload"
#define	LOGFNAME	"var/log/opensvclogload"
#define	LOCKFNAME	"spool/locks/opensvclogload"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


