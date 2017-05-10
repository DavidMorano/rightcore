/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)initinfo "
#define	BANNER		"Initialization Information"
#define	SEARCHNAME	"initinfo"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"INITINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"INITINFO_BANNER"
#define	VARSEARCHNAME	"INITINFO_NAME"
#define	VAROPTS		"INITINFO_OPTS"
#define	VARFILEROOT	"INITINFO_FILEROOT"
#define	VARLOGTAB	"INITINFO_LOGTAB"
#define	VARMSFNAME	"INITINFO_MSFILE"
#define	VARERRORFNAME	"INITINFO_ERRORFILE"

#define	VARDEBUGFNAME	"INITINFO_DEBUGFILE"
#define	VARDEBUGFD1	"INITINFO_DEBUGFD"
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

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/initinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/initinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/initinfo"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4


