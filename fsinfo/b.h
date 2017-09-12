/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)fsinfo "
#define	BANNER		"Filesystem Information"
#define	SEARCHNAME	"fsinfo"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FSINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FSINFO_BANNER"
#define	VARSEARCHNAME	"FSINFO_NAME"
#define	VARFILEROOT	"FSINFO_FILEROOT"
#define	VARFPATH	"FSINFO_FPATH"
#define	VARLOGTAB	"FSINFO_LOGTAB"
#define	VARMSFNAME	"FSINFO_MSFILE"
#define	VARUTFNAME	"FSINFO_UTFILE"
#define	VARAFNAME	"FSINFO_AF"
#define	VAREFNAME	"FSINFO_EF"

#define	VARDEBUGFNAME	"FSINFO_DEBUGFILE"
#define	VARDEBUGFD1	"FSINFO_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/fsinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/fsinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/fsinfo"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4

#define	DEFSPEC		"fsavail"


