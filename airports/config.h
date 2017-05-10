/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)airports "
#define	BANNER		"Airport Logging Server"

#define	VARPROGRAMROOT1	"AIRPORTS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"AIRPORTS_BANNER"
#define	VARSEARCHNAME	"AIRPORTS_NAME"
#define	VARSTDERRFNAME	"AIRPORTS_ERRFILE"
#define	VARDDNAME	"AIRPORTS_DDIR"
#define	VARSLFNAME	"AIRPORTS_SLFILE"

#define	VARFILEROOT	"AIRPORTS_FILEROOT"
#define	VARLOGTAB	"AIRPORTS_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"AIRPORTS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"airports"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
#define	SLDNAME		"/var/log"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/airports"
#define	LOCKDNAME	"var/spool/locks"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	SLFNAME		"local0"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"%N"			/* mutex PID file */
#define	LOGFNAME	"var/log/airports"	/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	POLLINT		2
#define	RUNINT		3600
#define	MARKINT		(4 * 3600)
#define	LOCKINT		(5 * 60)
#define	PIDINT		(5 * 60)



