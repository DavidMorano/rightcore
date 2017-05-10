/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialerfinger "
#define	BANNER		"Open-Dialer Finger"
#define	SEARCHNAME	"opendialerfinger"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERFINGER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERFINGER_BANNER"
#define	VARSEARCHNAME	"OPENDIALERFINGER_NAME"
#define	VARFILEROOT	"OPENDIALERFINGER_FILEROOT"
#define	VARLOGTAB	"OPENDIALERFINGER_LOGTAB"
#define	VARMSFNAME	"OPENDIALERFINGER_MSFILE"
#define	VARUTFNAME	"OPENDIALERFINGER_UTFILE"
#define	VARERRORFNAME	"OPENDIALERFINGER_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERFINGER_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERFINGER_DEBUGFD"
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

#define	PIDFNAME	"run/opendialerfinger"
#define	LOGFNAME	"var/log/opendialerfinger"
#define	LOCKFNAME	"spool/locks/opendialerfinger"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2


