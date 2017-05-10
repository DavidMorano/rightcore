/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)utime "
#define	BANNER		"UNIX Time"
#define	SEARCHNAME	"utime"
#defien	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"UTIME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"UTIME_BANNER"
#define	VARSEARCHNAME	"UTIME_NAME"
#define	VAROPTS		"UTIME_OPTS"
#define	VARFILEROOT	"UTIME_FILEROOT"
#define	VARLOGTAB	"UTIME_LOGTAB"
#define	VARMSFNAME	"UTIME_MSFILE"
#define	VARERRORFNAME	"UTIME_ERRORFILE"

#define	VARDEBUGFNAME	"UTIME_DEBUGFILE"
#define	VARDEBUGFD1	"UTIME_DEBUGFD"
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

#define	PIDFNAME	"run/utime"		/* mutex PID file */
#define	LOGFNAME	"var/log/utime"		/* activity log */
#define	LOCKFNAME	"spool/locks/utime"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4


