/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialerhello "
#define	BANNER		"Open-Dialer Hello"
#define	SEARCHNAME	"opendialerhello"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERHELLO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERHELLO_BANNER"
#define	VARSEARCHNAME	"OPENDIALERHELLO_NAME"
#define	VARFILEROOT	"OPENDIALERHELLO_FILEROOT"
#define	VARLOGTAB	"OPENDIALERHELLO_LOGTAB"
#define	VARMSFNAME	"OPENDIALERHELLO_MSFILE"
#define	VARUTFNAME	"OPENDIALERHELLO_UTFILE"
#define	VARERRORFNAME	"OPENDIALERHELLO_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERHELLO_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERHELLO_DEBUGFD"
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

#define	PIDFNAME	"run/opendialerhello"
#define	LOGFNAME	"var/log/opendialerhello"
#define	LOCKFNAME	"spool/locks/opendialerhello"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



