/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcqotd "
#define	BANNER		"Open Service QOTD"
#define	SEARCHNAME	"opensvcqotd"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCQOTD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCQOTD_BANNER"
#define	VARSEARCHNAME	"OPENSVCQOTD_NAME"
#define	VAROPTS		"OPENSVCQOTD_OPTS"
#define	VARFILEROOT	"OPENSVCQOTD_FILEROOT"
#define	VARLOGTAB	"OPENSVCQOTD_LOGTAB"
#define	VARMSFNAME	"OPENSVCQOTD_MSFILE"
#define	VARUTFNAME	"OPENSVCQOTD_UTFILE"
#define	VARERRORFNAME	"OPENSVCQOTD_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCQOTD_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCQOTD_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/opensvcqotd"
#define	LOGFNAME	"var/log/opensvcqotd"
#define	LOCKFNAME	"spool/locks/opensvcqotd"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


