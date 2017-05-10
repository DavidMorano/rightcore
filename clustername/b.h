/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)clustername "
#define	BANNER		"Cluster Name"
#define	SEARCHNAME	"clustername"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CLUSTERNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CLUSTERNAME_BANNER"
#define	VARSEARCHNAME	"CLUSTERNAME_NAME"
#define	VAROPTS		"CLUSTERNAME_OPTS"
#define	VARFILEROOT	"CLUSTERNAME_FILEROOT"
#define	VARLOGTAB	"CLUSTERNAME_LOGTAB"
#define	VARAFNAME	"CLUSTERNAME_AF"
#define	VAREFNAME	"CLUSTERNAME_EF"

#define	VARDEBUGFNAME	"CLUSTERNAME_DEBUGFILE"
#define	VARDEBUGFD1	"CLUSTERNAME_DEBUGFD"
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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/clustername"		/* mutex PID file */
#define	LOGFNAME	"var/log/clustername"		/* activity log */
#define	LOCKFNAME	"spool/locks/clustername"	/* lock mutex file */

#ifndef	NODEFNAME
#define	NODEFNAME	"etc/node"
#endif

#ifndef	CLUSTERFNAME1
#define	CLUSTERFNAME1	"etc/cluster"
#endif
#ifndef	CLUSTERFNAME2
#define	CLUSTERFNAME2	"etc/clusters"
#endif

#define	LOGSIZE		(80*1024)


