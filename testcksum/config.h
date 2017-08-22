/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcksum "
#define	BANNER		"Test CKSUM"
#define	SEARCHNAME	"testcksum"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTCKSUM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCKSUM_BANNER"
#define	VARSEARCHNAME	"TESTCKSUM_NAME"
#define	VARFILEROOT	"TESTCKSUM_FILEROOT"
#define	VARLOGTAB	"TESTCKSUM_LOGTAB"
#define	VARMSFNAME	"TESTCKSUM_MSFILE"
#define	VARUTFNAME	"TESTCKSUM_UTFILE"
#define	VARERRORFNAME	"TESTCKSUM_ERRORFILE"

#define	VARDEBUGFNAME	"TESTCKSUM_DEBUGFILE"
#define	VARDEBUGFD1	"TESTCKSUM_DEBUGFD"
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

#define	PIDFNAME	"run/testcksum"		/* mutex PID file */
#define	LOGFNAME	"var/log/testcksum"	/* activity log */
#define	LOCKFNAME	"spool/locks/testcksum"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	LOGSIZE		(80*1024)

#define	DEFRUNINT	60
#define	DEFPOLLINT	8
#define	DEFNODES	50

#define	TO_CACHE	2

#define	USAGECOLS	4


