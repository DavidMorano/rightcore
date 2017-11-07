/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rmshm "
#define	BANNER		"Remove Shared-Memory segment"
#define	SEARCHNAME	"rmshm"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"RMSHM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RMSHM_BANNER"
#define	VARSEARCHNAME	"RMSHM_NAME"
#define	VARFILEROOT	"RMSHM_FILEROOT"
#define	VARLOGTAB	"RMSHM_LOGTAB"
#define	VARMSFNAME	"RMSHM_MSFILE"
#define	VARAFNAME	"RMSHM_AF"
#define	VAREFNAME	"RMSHM_EF"
#define	VAROFNAME	"RMSHM_OF"
#define	VARIFNAME	"RMSHM_IF"

#define	VARDEBUGFNAME	"RMSHM_DEBUGFILE"
#define	VARDEBUGFD1	"RMSHM_DEBUGFD"
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

#define	PIDFNAME	"run/rmshm"		/* mutex PID file */
#define	LOGFNAME	"var/log/rmshm"		/* activity log */
#define	LOCKFNAME	"spool/locks/rmshm"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50


