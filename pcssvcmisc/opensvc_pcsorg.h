/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcpcsorg "
#define	BANNER		"Open-Service Organization"
#define	SEARCHNAME	"opensvcpcsorg"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"OPENSVCPCSORG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCPCSORG_BANNER"
#define	VARSEARCHNAME	"OPENSVCPCSORG_NAME"
#define	VARFILEROOT	"OPENSVCPCSORG_FILEROOT"
#define	VARLOGTAB	"OPENSVCPCSORG_LOGTAB"
#define	VARMSFNAME	"OPENSVCPCSORG_MSFILE"
#define	VARUTFNAME	"OPENSVCPCSORG_UTFILE"
#define	VARERRORFNAME	"OPENSVCPCSORG_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCPCSORG_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCPCSORG_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcpcsorg"
#define	LOGFNAME	"var/log/opensvcpcsorg"
#define	LOCKFNAME	"spool/locks/opensvcpcsorg"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


