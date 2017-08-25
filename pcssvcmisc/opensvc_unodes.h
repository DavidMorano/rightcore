/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcunodes "
#define	BANNER		"Open-Service User-Nodes"
#define	SEARCHNAME	"opensvcunodes"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"OPENSVCUNODES_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCUNODES_BANNER"
#define	VARSEARCHNAME	"OPENSVCUNODES_NAME"
#define	VARFILEROOT	"OPENSVCUNODES_FILEROOT"
#define	VARLOGTAB	"OPENSVCUNODES_LOGTAB"
#define	VARMSFNAME	"OPENSVCUNODES_MSFILE"
#define	VARUTFNAME	"OPENSVCUNODES_UTFILE"
#define	VARERRORFNAME	"OPENSVCUNODES_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCUNODES_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCUNODES_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcunodes"
#define	LOGFNAME	"var/log/opensvcunodes"
#define	LOCKFNAME	"spool/locks/opensvcunodes"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


