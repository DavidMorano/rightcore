/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcpcsmotd "
#define	BANNER		"Open-Service MOTD"
#define	SEARCHNAME	"opensvcpcsmotd"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"OPENSVCPCSMOTD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCPCSMOTD_BANNER"
#define	VARSEARCHNAME	"OPENSVCPCSMOTD_NAME"
#define	VARFILEROOT	"OPENSVCPCSMOTD_FILEROOT"
#define	VARLOGTAB	"OPENSVCPCSMOTD_LOGTAB"
#define	VARMSFNAME	"OPENSVCPCSMOTD_MSFILE"
#define	VARUTFNAME	"OPENSVCPCSMOTD_UTFILE"
#define	VARERRORFNAME	"OPENSVCPCSMOTD_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCPCSMOTD_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCPCSMOTD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
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

#define	PIDFNAME	"run/opensvcpcsmotd"
#define	LOGFNAME	"var/log/opensvcpcsmotd"
#define	LOCKFNAME	"spool/locks/opensvcpcsmotd"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


