/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcomsat "
#define	SEARCHNAME	"testcomsat"
#define	BANNER		"Test Comsat"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTCOMSAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCOMSAT_BANNER"
#define	VARSEARCHNAME	"TESTCOMSAT_NAME"
#define	VAROPTS		"TESTCOMSAT_OPTS"
#define	VARFILEROOT	"TESTCOMSAT_FILEROOT"
#define	LOGFILEVAR	"TESTCOMSAT_LOGFILE"
#define	VARLOGTAB	"TESTCOMSAT_LOGTAB"
#define	VARMSFNAME	"TESTCOMSAT_MSFILE"
#define	VARUTFNAME	"TESTCOMSAT_UTFILE"
#define	VARERRORFNAME	"TESTCOMSAT_ERRORFILE"

#define	VARDEBUGFNAME	"TESTCOMSAT_DEBUGFILE"
#define	VARDEBUGFD1	"TESTCOMSAT_DEBUGFD"
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

#define	LOGFNAME	"log/testcomsat"		/* activity log */
#define	PIDFNAME	"spool/run/testcomsat"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testcomsat"	/* lock mutex file */
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)
#define	TO_RUN		20

#define	PORTSPEC	"comsat"
#define	SVCSPEC		"comsat"

#define	PORT_COMSAT	512

#define	DIALTIME	20



