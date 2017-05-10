/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvczoneabbr "
#define	BANNER		"Open-Service FS-Info"
#define	SEARCHNAME	"opensvczoneabbr"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCZONEABBR_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCZONEABBR_BANNER"
#define	VARSEARCHNAME	"OPENSVCZONEABBR_NAME"
#define	VARFILEROOT	"OPENSVCZONEABBR_FILEROOT"
#define	VARLOGTAB	"OPENSVCZONEABBR_LOGTAB"
#define	VARMSFNAME	"OPENSVCZONEABBR_MSFILE"
#define	VARUTFNAME	"OPENSVCZONEABBR_UTFILE"
#define	VARERRORFNAME	"OPENSVCZONEABBR_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCZONEABBR_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCZONEABBR_DEBUGFD"
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

#define	PIDFNAME	"run/opensvczoneabbr"
#define	LOGFNAME	"var/log/opensvczoneabbr"
#define	LOCKFNAME	"spool/locks/opensvczoneabbr"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


