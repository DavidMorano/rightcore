/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)opensvcnotices "
#define	BANNER		"Open-Service Notices"
#define	SEARCHNAME	"opensvcnotices"
#define	VARPRNAME	"PCS"

#define	VARPROGRAMROOT1	"OPENSVCNOTICES_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OPENSVCNOTICES_BANNER"
#define	VARSEARCHNAME	"OPENSVCNOTICES_NAME"
#define	VARFILEROOT	"OPENSVCNOTICES_FILEROOT"
#define	VARLOGTAB	"OPENSVCNOTICES_LOGTAB"
#define	VARMSFNAME	"OPENSVCNOTICES_MSFILE"
#define	VARUTFNAME	"OPENSVCNOTICES_UTFILE"
#define	VARERRORFNAME	"OPENSVCNOTICES_ERRORFILE"

#define	VARDEBUGFNAME	"OPENSVCNOTICES_DEBUGFILE"
#define	VARDEBUGFD1	"OPENSVCNOTICES_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARCOLUMNS	"COLUMNS"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
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

#define	PIDFNAME	"run/opensvcnotices"
#define	LOGFNAME	"var/log/opensvcnotices"
#define	LOCKFNAME	"spool/locks/opensvcnotices"
#define	BBNEWSDNAME	"spool/boards"

#define	LOGSIZE		(80*1024)
#define	COLUMNS		80

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50
#define	DEFPERIOD	(24*3600)

#define	TO_CACHE	2


