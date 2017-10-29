/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)pcsname "
#define	BANNER		"PCS Name"
#define	SEARCHNAME	"pcsname"
#define	VARPRNAME	"PCS"

#define	VARPROGRAMROOT1	"PCSNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSNAME_BANNER"
#define	VARSEARCHNAME	"PCSNAME_NAME"
#define	VARFILEROOT	"PCSNAME_FILEROOT"
#define	VARLOGTAB	"PCSNAME_LOGTAB"
#define	VARMSFNAME	"PCSNAME_MSFILE"
#define	VARUTFNAME	"PCSNAME_UTFILE"
#define	VARERRORFNAME	"PCSNAME_ERRORFILE"

#define	VARDEBUGFNAME	"PCSNAME_DEBUGFILE"
#define	VARDEBUGFD1	"PCSNAME_DEBUGFD"
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
#define	NAMEFNAME	".name"
#define	FULLNAMEFNAME	".fullname"

#define	PIDFNAME	"run/pcsname"		/* mutex PID file */
#define	LOGFNAME	"log/pcsname"		/* activity log */
#define	LOCKFNAME	"spool/locks/pcsnamela"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



