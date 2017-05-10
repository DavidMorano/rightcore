/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)fsinfo "
#define	BANNER		"Filesystem Information"
#define	SEARCHNAME	"fsinfo"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"FSINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FSINFO_BANNER"
#define	VARSEARCHNAME	"FSINFO_NAME"
#define	VARFILEROOT	"FSINFO_FILEROOT"
#define	VARLOGTAB	"FSINFO_LOGTAB"
#define	VARMSFNAME	"FSINFO_MSFILE"
#define	VARUTFNAME	"FSINFO_UTFILE"
#define	VARERRORFNAME	"FSINFO_ERRORFILE"

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

#define	PIDFNAME	"run/fsinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/fsinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/fsinfo"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	USAGECOLS	4



