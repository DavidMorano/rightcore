/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)openintcopyout "
#define	BANNER		"Open-Service Hello"
#define	SEARCHNAME	"openintcopyout"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"OPENINTCOPYOUT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENINTCOPYOUT_BANNER"
#define	VARSEARCHNAME	"OPENINTCOPYOUT_NAME"
#define	VARFILEROOT	"OPENINTCOPYOUT_FILEROOT"
#define	VARLOGTAB	"OPENINTCOPYOUT_LOGTAB"
#define	VARMSFNAME	"OPENINTCOPYOUT_MSFILE"
#define	VARUTFNAME	"OPENINTCOPYOUT_UTFILE"
#define	VARERRORFNAME	"OPENINTCOPYOUT_ERRORFILE"

#define	VARDEBUGFNAME	"OPENINTCOPYOUT_DEBUGFILE"
#define	VARDEBUGFD1	"OPENINTCOPYOUT_DEBUGFD"
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

#define	PIDFNAME	"run/openintcopyout"
#define	LOGFNAME	"var/log/openintcopyout"
#define	LOCKFNAME	"spool/locks/openintcopyout"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2

#define	USAGECOLS	4



