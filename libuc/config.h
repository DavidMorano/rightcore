/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testspawnproc "
#define	BANNER		"Test SpawnProc"
#define	SEARCHNAME	"testspawnproc"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTSPAWNPROC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTSPAWNPROC_BANNER"
#define	VARSEARCHNAME	"TESTSPAWNPROC_NAME"
#define	VAROPTS		"TESTSPAWNPROC_OPTS"
#define	VARFILEROOT	"TESTSPAWNPROC_FILEROOT"
#define	VARLOGTAB	"TESTSPAWNPROC_LOGTAB"
#define	VARAFNAME	"TESTSPAWNPROC_AF"
#define	VAREFNAME	"TESTSPAWNPROC_EF"

#define	VARDEBUGFNAME	"TESTSPAWNPROC_DEBUGFILE"
#define	VARDEBUGFD1	"TESTSPAWNPROC_DEBUGFD"
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
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"

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

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/qotd"		/* mutex PID file */
#define	LOGFNAME	"var/log/qotd"		/* activity log */
#define	LOCKFNAME	"spool/locks/qotd"	/* lock mutex file */

#ifndef	COLUMNS
#define	COLUMNS		80			/* output columns */
#endif

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



