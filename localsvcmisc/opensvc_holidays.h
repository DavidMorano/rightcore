/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcholidays "
#define	BANNER		"Open-Service Holidays"
#define	SEARCHNAME	"opensvcholidays"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCHOLIDAYS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCHOLIDAYS_BANNER"
#define	VARSEARCHNAME	"OPENSVCHOLIDAYS_NAME"
#define	VARFILEROOT	"OPENSVCHOLIDAYS_FILEROOT"
#define	VARLOGTAB	"OPENSVCHOLIDAYS_LOGTAB"
#define	VARMSFNAME	"OPENSVCHOLIDAYS_MSFILE"
#define	VARUTFNAME	"OPENSVCHOLIDAYS_UTFILE"
#define	VARERRORFNAME	"OPENSVCHOLIDAYS_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCHOLIDAYS_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCHOLIDAYS_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcholidays"
#define	LOGFNAME	"var/log/opensvcholidays"
#define	LOCKFNAME	"spool/locks/opensvcholidays"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


