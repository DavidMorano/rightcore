/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcdaytime "
#define	BANNER		"Open-Service Daytime"
#define	SEARCHNAME	"opensvcdaytime"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCDAYTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCDAYTIME_BANNER"
#define	VARSEARCHNAME	"OPENSVCDAYTIME_NAME"
#define	VARFILEROOT	"OPENSVCDAYTIME_FILEROOT"
#define	VARLOGTAB	"OPENSVCDAYTIME_LOGTAB"
#define	VARMSFNAME	"OPENSVCDAYTIME_MSFILE"
#define	VARUTFNAME	"OPENSVCDAYTIME_UTFILE"
#define	VARERRORFNAME	"OPENSVCDAYTIME_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCDAYTIME_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCDAYTIME_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcdaytime"
#define	LOGFNAME	"var/log/opensvcdaytime"
#define	LOCKFNAME	"spool/locks/opensvcdaytime"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


