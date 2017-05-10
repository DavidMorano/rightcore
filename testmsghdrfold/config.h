/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)msghdrfold "
#define	BANNER		"MSG HDR Fold"
#define	SEARCHNAME	"msghdrfold"

#define	VARPROGRAMROOT1	"MSGHDRFOLD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MSGHDRFOLD_BANNER"
#define	VARSEARCHNAME	"MSGHDRFOLD_NAME"
#define	VARFILEROOT	"MSGHDRFOLD_FILEROOT"
#define	VARLOGTAB	"MSGHDRFOLD_LOGTAB"
#define	VARMSFNAME	"MSGHDRFOLD_MSFILE"
#define	VARUTFNAME	"MSGHDRFOLD_UTFILE"
#define	VARERRORFNAME	"MSGHDRFOLD_ERRORFILE"

#define	VARDEBUGFNAME	"MSGHDRFOLD_DEBUGFILE"
#define	VARDEBUGFD1	"MSGHDRFOLD_DEBUGFD"
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

#define	PIDFNAME	"run/msghdrfold"	/* mutex PID file */
#define	LOGFNAME	"var/log/msghdrfold"	/* activity log */
#define	LOCKFNAME	"spool/locks/msghdrfold"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50



