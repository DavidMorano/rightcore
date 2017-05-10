/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvccotd "
#define	BANNER		"Open Service COTD"
#define	SEARCHNAME	"opensvccotd"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCCOTD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCCOTD_BANNER"
#define	VARSEARCHNAME	"OPENSVCCOTD_NAME"
#define	VAROPTS		"OPENSVCCOTD_OPTS"
#define	VARFILEROOT	"OPENSVCCOTD_FILEROOT"
#define	VARLOGTAB	"OPENSVCCOTD_LOGTAB"
#define	VARMSFNAME	"OPENSVCCOTD_MSFILE"
#define	VARUTFNAME	"OPENSVCCOTD_UTFILE"
#define	VARERRORFNAME	"OPENSVCCOTD_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCCOTD_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCCOTD_DEBUGFD"
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

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/opensvccotd"
#define	LOGFNAME	"var/log/opensvccotd"
#define	LOCKFNAME	"spool/locks/opensvccotd"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


