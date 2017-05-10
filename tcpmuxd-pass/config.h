/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)tcpmuxd-pass "
#define	BANNER		"TCPMUXDPASS"
#define	SEARCHNAME	"tcpmuxdpass"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"TCPMUXDPASS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TCPMUXDPASS_BANNER"
#define	VARSEARCHNAME	"TCPMUXDPASS_NAME"
#define	VAROPTS		"TCPMUXDPASS_OPTS"
#define	VARFILEROOT	"TCPMUXDPASS_FILEROOT"
#define	VARLOGTAB	"TCPMUXDPASS_LOGTAB"
#define	VARLFNAME	"TCPMUXDPASS_LF"
#define	VAREFNAME	"TCPMUXDPASS_EF"
#define	VARERRORFNAME	"TCPMUXDPASS_ERRORFILE"

#define	VARDEBUGFNAME	"TCPMUXDPASS_DEBUGFILE"
#define	VARDEBUGFD1	"TCPMUXDPASS_DEBUGFD"
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

#define	PIDFNAME	"run/tcpmuxdpass"		/* mutex PID file */
#define	LOGFNAME	"var/log/tcpmuxdpass"		/* activity log */
#define	LOCKFNAME	"spool/locks/tcpmuxdpass"	/* lock mutex file */
#define	REQFNAME	"/var/tmp/tcpmuxd/req"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50


