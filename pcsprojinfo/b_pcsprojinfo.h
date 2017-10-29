/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)pcsprojinfo "
#define	BANNER		"PCS Project Information"
#define	SEARCHNAME	"pcsprojinfo"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PCSPROJINFO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSPROJINFO_BANNER"
#define	VARSEARCHNAME	"PCSPROJINFO_NAME"
#define	VAROPTS		"PCSPROJINFO_OPTS"
#define	VARFILEROOT	"PCSPROJINFO_FILEROOT"
#define	VARLOGTAB	"PCSPROJINFO_LOGTAB"
#define	VARMSFNAME	"PCSPROJINFO_MSFILE"
#define	VARUTFNAME	"PCSPROJINFO_UTFILE"
#define	VARAFNAME	"PCSPROJINFO_AF"
#define	VAREFNAME	"PCSPROJINFO_EF"

#define	VARDEBUGFNAME	"PCSPROJINFO_DEBUGFILE"
#define	VARDEBUGFD1	"PCSPROJINFO_DEBUGFD"
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
#define	NAMEFNAME	".name"
#define	FULLNAMEFNAME	".fullname"
#define	PROJFNAME	".project"

#define	PIDFNAME	"run/pcsprojinfo"		/* mutex PID file */
#define	LOGFNAME	"log/pcsprojinfo"		/* activity log */
#define	LOCKFNAME	"spool/locks/pcsprojinfola"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


