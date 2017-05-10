/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testhdbstr "
#define	BANNER		"TEST HDB-String"
#define	SEARCHNAME	"testhdbstr"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTHDBSTR_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTHDBSTR_BANNER"
#define	VARSEARCHNAME	"TESTHDBSTR_NAME"
#define	VAROPTS		"TESTHDBSTR_OPTS"
#define	VARFILEROOT	"TESTHDBSTR_FILEROOT"
#define	VARLOGTAB	"TESTHDBSTR_LOGTAB"
#define	VARMSFNAME	"TESTHDBSTR_MSFILE"
#define	VARUTFNAME	"TESTHDBSTR_UTFILE"
#define	VAREFNAME	"TESTHDBSTR_EF"

#define	VARDEBUGFNAME	"TESTHDBSTR_DEBUGFILE"
#define	VARDEBUGFD1	"TESTHDBSTR_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

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

#define	PIDFNAME	"run/la"		/* mutex PID file */
#define	LOGFNAME	"var/log/la"		/* activity log */
#define	LOCKFNAME	"spool/locks/la"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2
#define	TO_LOADAVG	1

#define	USAGECOLS	4



