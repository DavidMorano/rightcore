/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)ismailaddr "
#define	BANNER		"Is Mail Address"
#define	SEARCHNAME	"ismailaddr"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ISMAILADDR_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISMAILADDR_BANNER"
#define	VARSEARCHNAME	"ISMAILADDR_NAME"
#define	VAROPTS		"ISMAILADDR_OPTS"
#define	VARFILEROOT	"ISMAILADDR_FILEROOT"
#define	VARLOGTAB	"ISMAILADDR_LOGTAB"
#define	VARLOGFNAME	"ISMAILADDR_LOGFILE"
#define	VARMSFNAME	"ISMAILADDR_MSFILE"
#define	VARAFNAME	"ISMAILADDR_AF"
#define	VAREFNAME	"ISMAILADDR_EF"

#define	VARDEBUGFNAME	"ISMAILADDR_DEBUGFILE"
#define	VARDEBUGFD1	"ISMAILADDR_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VCNAME		"var"
#define	LOGDNAME	"log"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	LOGFNAME	"ismailaddr"
#define	SERIALFNAME	"serial"

#define	LNFNAME		"etc/localnames"
#define	PIDFNAME	"run/ismailaddr"
#define	MSFNAME		"var/ms"
#define	LOCKFNAME	"spool/locks/ismailaddr"

#define	LOGSIZE		(80*1024)


