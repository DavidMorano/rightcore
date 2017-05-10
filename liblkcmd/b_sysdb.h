/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sysdb "
#define	BANNER		"System Database"
#define	SEARCHNAME	"sysdb"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	VARPROGRAMROOT1	"SYSDB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSDB_BANNER"
#define	VARSEARCHNAME	"SYSDB_NAME"
#define	VAROPTS		"SYSDB_OPTS"
#define	VARFILEROOT	"SYSDB_FILEROOT"
#define	VARLOGTAB	"SYSDB_LOGTAB"
#define	VARMSFNAME	"SYSDB_MSFILE"
#define	VARDBFNAME	"SYSDB_DBFILE"
#define	VARSFNAME	"SYSDB_SHELLS"
#define	VARAFNAME	"SYSDB_AF"
#define	VAREFNAME	"SYSDB_EF"

#define	VARDEBUGFNAME	"SYSDB_DEBUGFILE"
#define	VARDEBUGFD1	"SYSDB_DEBUGFD"
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
#define	LOGCNAME	"log"
#define	SYSCNAME	"sys"
#define	UHDNAME		"userhomes"
#define	UNFNAME		"usernames"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"
#define	PASSWDFNAME	"/etc/passwd"
#define	GROUPFNAME	"/etc/group"
#define	PROJECTFNAME	"/etc/project"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/sysdb"
#define	LOGFNAME	"var/log/sysdb"
#define	LOCKFNAME	"spool/locks/sysdb"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50
#define	DEFPROTO	"tcp"

#define	TO_CACHE	2
#define	TO_LOADAVG	1


