/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcfshome "
#define	BANNER		"Open-Service FS-Info"
#define	SEARCHNAME	"opensvcfshome"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"OPENSVCFSHOME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCFSHOME_BANNER"
#define	VARSEARCHNAME	"OPENSVCFSHOME_NAME"
#define	VARFILEROOT	"OPENSVCFSHOME_FILEROOT"
#define	VARLOGTAB	"OPENSVCFSHOME_LOGTAB"
#define	VARMSFNAME	"OPENSVCFSHOME_MSFILE"
#define	VARUTFNAME	"OPENSVCFSHOME_UTFILE"
#define	VARERRORFNAME	"OPENSVCFSHOME_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCFSHOME_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCFSHOME_DEBUGFD"
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

#define	PIDFNAME	"run/opensvcfshome"
#define	LOGFNAME	"var/log/opensvcfshome"
#define	LOCKFNAME	"spool/locks/opensvcfshome"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


