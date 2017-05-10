/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testsocket "
#define	SEARCHNAME	"testsocket"
#define	BANNER		"Test Socket"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTSOCKET_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTSOCKET_BANNER"
#define	VARSEARCHNAME	"TESTSOCKET_NAME"
#define	VAROPTS		"TESTSOCKET_OPTS"
#define	VARFILEROOT	"TESTSOCKET_FILEROOT"
#define	LOGFILEVAR	"TESTSOCKET_LOGFILE"
#define	VARLOGTAB	"TESTSOCKET_LOGTAB"
#define	VARMSFNAME	"TESTSOCKET_MSFILE"
#define	VARUTFNAME	"TESTSOCKET_UTFILE"
#define	VARERRORFNAME	"TESTSOCKET_ERRORFILE"

#define	VARDEBUGFNAME	"TESTSOCKET_DEBUGFILE"
#define	VARDEBUGFD1	"TESTSOCKET_DEBUGFD"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	HELPFNAME	"help"

#define	LOGFNAME	"log/testsocket"		/* activity log */
#define	PIDFNAME	"spool/run/testsocket"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testsocket"	/* lock mutex file */
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)
#define	TO_RUN		20

#define	PORTSPEC	"comsat"
#define	SVCSPEC		"comsat"

#define	PORT_COMSAT	512

#define	DIALTIME	20



