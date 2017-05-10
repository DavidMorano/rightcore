/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sysfs "
#define	BANNER		"System File-System"
#define	SEARCHNAME	"sysfs"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SYSFS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSFS_BANNER"
#define	VARSEARCHNAME	"SYSFS_NAME"
#define	VAROPTS		"SYSFS_OPTS"
#define	VARFILEROOT	"SYSFS_FILEROOT"
#define	VARLOGTAB	"SYSFS_LOGTAB"
#define	VARMSFNAME	"SYSFS_MSFILE"
#define	VARDBFNAME	"SYSFS_DBFILE"
#define	VARSFNAME	"SYSFS_SHELLS"
#define	VARAFNAME	"SYSFS_AF"
#define	VAREFNAME	"SYSFS_EF"

#define	VARDEBUGFNAME	"SYSFS_DEBUGFILE"
#define	VARDEBUGFD1	"SYSFS_DEBUGFD"
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

#define	PIDFNAME	"run/sysfs"
#define	LOGFNAME	"var/log/sysfs"
#define	LOCKFNAME	"spool/locks/sysfs"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2
#define	TO_LOADAVG	1


