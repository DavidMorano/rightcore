/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)la "
#define	BANNER		"Load Average"
#define	SEARCHNAME	"la"

#define	VARPROGRAMROOT1	"LA_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LA_BANNER"
#define	VARSEARCHNAME	"LA_NAME"
#define	VARFILEROOT	"LA_FILEROOT"
#define	VARLOGTAB	"LA_LOGTAB"
#define	VARMSFNAME	"LA_MSFILE"
#define	VARERRORFNAME	"LA_ERRORFILE"

#define	VARDEBUGFNAME	"LA_DEBUGFILE"
#define	VARDEBUGFD1	"LA_DEBUGFD"
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

#define	PIDFNAME	"run/la"		/* mutex PID file */
#define	LOGFNAME	"var/log/la"		/* activity log */
#define	LOCKFNAME	"spool/locks/la"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4



