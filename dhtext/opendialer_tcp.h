/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialertcp "
#define	BANNER		"Open-Dialer TCP"
#define	SEARCHNAME	"opendialertcp"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERTCP_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERTCP_BANNER"
#define	VARSEARCHNAME	"OPENDIALERTCP_NAME"
#define	VARFILEROOT	"OPENDIALERTCP_FILEROOT"
#define	VARLOGTAB	"OPENDIALERTCP_LOGTAB"
#define	VARMSFNAME	"OPENDIALERTCP_MSFILE"
#define	VARUTFNAME	"OPENDIALERTCP_UTFILE"
#define	VARERRORFNAME	"OPENDIALERTCP_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERTCP_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERTCP_DEBUGFD"
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

#define	PIDFNAME	"run/opendialertcp"
#define	LOGFNAME	"var/log/opendialertcp"
#define	LOCKFNAME	"spool/locks/opendialertcp"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



