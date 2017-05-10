/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opendialeruss "
#define	BANNER		"Open-Dialer USS"
#define	SEARCHNAME	"opendialeruss"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"OPENDIALERUSS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENDIALERUSS_BANNER"
#define	VARSEARCHNAME	"OPENDIALERUSS_NAME"
#define	VARFILEROOT	"OPENDIALERUSS_FILEROOT"
#define	VARLOGTAB	"OPENDIALERUSS_LOGTAB"
#define	VARMSFNAME	"OPENDIALERUSS_MSFILE"
#define	VARUTFNAME	"OPENDIALERUSS_UTFILE"
#define	VARERRORFNAME	"OPENDIALERUSS_ERRORFILE"

#define	VARDEBUGFNAME	"OPENDIALERUSS_DEBUGFILE"
#define	VARDEBUGFD1	"OPENDIALERUSS_DEBUGFD"
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

#define	PIDFNAME	"run/opendialeruss"
#define	LOGFNAME	"var/log/opendialeruss"
#define	LOCKFNAME	"spool/locks/opendialeruss"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2



