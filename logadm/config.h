/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)logadm "
#define	BANNER		"Login Administation"
#define	SEARCHNAME	"logadm"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"LOGADM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGADM_BANNER"
#define	VARSEARCHNAME	"LOGADM_NAME"
#define	VAROPTS		"LOGADM_OPTS"
#define	VARFILEROOT	"LOGADM_FILEROOT"
#define	VARLOGTAB	"LOGADM_LOGTAB"
#define	VARMSFNAME	"LOGADM_MSFILE"
#define	VARUTFNAME	"LOGADM_UTFILE"
#define	VARAFNAME	"LOGADM_AF"
#define	VAREFNAME	"LOGADM_EF"

#define	VARDEBUGFNAME	"LOGADM_DEBUGFILE"
#define	VARDEBUGFD1	"LOGADM_DEBUGFD"
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

#define	PIDFNAME	"run/la"		/* mutex PID file */
#define	LOGFNAME	"var/log/la"		/* activity log */
#define	LOCKFNAME	"spool/locks/la"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2

#define	USAGECOLS	4


