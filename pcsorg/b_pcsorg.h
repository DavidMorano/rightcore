/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)pcsorg "
#define	BANNER		"PCS Project Information"
#define	SEARCHNAME	"pcsorg"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PCSORG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSORG_BANNER"
#define	VARSEARCHNAME	"PCSORG_NAME"
#define	VAROPTS		"PCSORG_OPTS"
#define	VARFILEROOT	"PCSORG_FILEROOT"
#define	VARLOGTAB	"PCSORG_LOGTAB"
#define	VARMSFNAME	"PCSORG_MSFILE"
#define	VARUTFNAME	"PCSORG_UTFILE"
#define	VARAFNAME	"PCSORG_AF"
#define	VAREFNAME	"PCSORG_EF"

#define	VARDEBUGFNAME	"PCSORG_DEBUGFILE"
#define	VARDEBUGFD1	"PCSORG_DEBUGFD"
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

#define	PIDFNAME	"run/pcsorg"		/* mutex PID file */
#define	LOGFNAME	"log/pcsorg"		/* activity log */
#define	LOCKFNAME	"spool/locks/pcsorgla"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


