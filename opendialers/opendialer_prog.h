/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialerprog "
#define	BANNER		"Open-Dialer Program"
#define	SEARCHNAME	"opendialerprog"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERPROG_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERPROG_BANNER"
#define	VARSEARCHNAME	"OPENDIALERPROG_NAME"
#define	VARFILEROOT	"OPENDIALERPROG_FILEROOT"
#define	VARLOGTAB	"OPENDIALERPROG_LOGTAB"
#define	VARMSFNAME	"OPENDIALERPROG_MSFILE"
#define	VARUTFNAME	"OPENDIALERPROG_UTFILE"
#define	VARERRORFNAME	"OPENDIALERPROG_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERPROG_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERPROG_DEBUGFD"
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

#define	PIDFNAME	"run/opendialerprog"
#define	LOGFNAME	"var/log/opendialerprog"
#define	LOCKFNAME	"spool/locks/opendialerprog"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



