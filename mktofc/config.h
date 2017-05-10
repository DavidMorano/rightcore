/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mktofc "
#define	BANNER		"Make To Folded Case"
#define	SEARCHNAME	"la"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"MKTOFC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKTOFC_BANNER"
#define	VARSEARCHNAME	"MKTOFC_NAME"
#define	VARFILEROOT	"MKTOFC_FILEROOT"
#define	VARLOGTAB	"MKTOFC_LOGTAB"
#define	VARMSFNAME	"MKTOFC_MSFILE"
#define	VARUTFNAME	"MKTOFC_UTFILE"
#define	VARERRORFNAME	"MKTOFC_ERRORFILE"

#define	VARDEBUGFNAME	"MKTOFC_DEBUGFILE"
#define	VARDEBUGFD1	"MKTOFC_DEBUGFD"
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

#define	PIDFNAME	"run/mktofc"		/* mutex PID file */
#define	LOGFNAME	"var/log/mktofc"		/* activity log */
#define	LOCKFNAME	"spool/locks/mktofc"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50



