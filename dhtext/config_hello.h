/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)openinthello "
#define	BANNER		"Open-Service Hello"
#define	SEARCHNAME	"openinthello"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"OPENINTHELLO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENINTHELLO_BANNER"
#define	VARSEARCHNAME	"OPENINTHELLO_NAME"
#define	VARFILEROOT	"OPENINTHELLO_FILEROOT"
#define	VARLOGTAB	"OPENINTHELLO_LOGTAB"
#define	VARMSFNAME	"OPENINTHELLO_MSFILE"
#define	VARUTFNAME	"OPENINTHELLO_UTFILE"
#define	VARERRORFNAME	"OPENINTHELLO_ERRORFILE"

#define	VARDEBUGFNAME	"OPENINTHELLO_DEBUGFILE"
#define	VARDEBUGFD1	"OPENINTHELLO_DEBUGFD"
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

#define	PIDFNAME	"run/openinthello"
#define	LOGFNAME	"var/log/openinthello"
#define	LOCKFNAME	"spool/locks/openinthello"
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2

#define	USAGECOLS	4



