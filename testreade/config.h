/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testreade "
#define	BANNER		"Test READE"
#define	SEARCHNAME	"testreade"

#define	VARPROGRAMROOT1	"TESTREADE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTREADE_BANNER"
#define	VARSEARCHNAME	"TESTREADE_NAME"
#define	VARFILEROOT	"TESTREADE_FILEROOT"
#define	VARLOGTAB	"TESTREADE_LOGTAB"
#define	VARMSFNAME	"TESTREADE_MSFILE"
#define	VARERRORFNAME	"TESTREADE_ERRORFILE"

#define	VARDEBUGFNAME	"TESTREADE_DEBUGFILE"
#define	VARDEBUGFD1	"TESTREADE_DEBUGFD"
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

#define	PIDFNAME	"run/testreade"		/* mutex PID file */
#define	LOGFNAME	"var/log/testreade"	/* activity log */
#define	LOCKFNAME	"spool/locks/testreade"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50



