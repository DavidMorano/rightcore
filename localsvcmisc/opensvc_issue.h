/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcissue "
#define	BANNER		"Open-Service IOTD"
#define	SEARCHNAME	"opensvcissue"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCISSUE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCISSUE_BANNER"
#define	VARSEARCHNAME	"OPENSVCISSUE_NAME"
#define	VARFILEROOT	"OPENSVCISSUE_FILEROOT"
#define	VARLOGTAB	"OPENSVCISSUE_LOGTAB"
#define	VARMSFNAME	"OPENSVCISSUE_MSFILE"
#define	VARUTFNAME	"OPENSVCISSUE_UTFILE"
#define	VARERRORFNAME	"OPENSVCISSUE_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCISSUE_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCISSUE_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcissue"
#define	LOGFNAME	"var/log/opensvcissue"
#define	LOCKFNAME	"spool/locks/opensvcissue"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


