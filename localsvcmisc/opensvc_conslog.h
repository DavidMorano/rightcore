/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcconsolog "
#define	BANNER		"Open-Service Login Blurb"
#define	SEARCHNAME	"opensvcconsolog"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCCONSLOG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCCONSLOG_BANNER"
#define	VARSEARCHNAME	"OPENSVCCONSLOG_NAME"
#define	VARFILEROOT	"OPENSVCCONSLOG_FILEROOT"
#define	VARLOGTAB	"OPENSVCCONSLOG_LOGTAB"
#define	VARMSFNAME	"OPENSVCCONSLOG_MSFILE"
#define	VARUTFNAME	"OPENSVCCONSLOG_UTFILE"
#define	VARERRORFNAME	"OPENSVCCONSLOG_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCCONSLOG_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCCONSLOG_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcconsolog"
#define	LOGFNAME	"var/log/opensvcconsolog"
#define	LOCKFNAME	"spool/locks/opensvcconsolog"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


