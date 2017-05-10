/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testsfio "
#define	BANNER		"Test SFIO"
#define	SEARCHNAME	"testsfio"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTSFIO_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTSFIO_BANNER"
#define	VARSEARCHNAME	"TESTSFIO_NAME"
#define	VAROPTS		"TESTSFIO_OPTS"
#define	VARFILEROOT	"TESTSFIO_FILEROOT"
#define	VARLOGTAB	"TESTSFIO_LOGTAB"
#define	VARERRORFNAME	"TESTSFIO_ERRORFILE"

#define	VARDEBUGFNAME	"TESTSFIO_DEBUGFILE"
#define	VARDEBUGFD1	"TESTSFIO_DEBUGFD"
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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testsfio"		/* mutex PID file */
#define	LOGFNAME	"var/log/testsfio"		/* activity log */
#define	LOCKFNAME	"spool/locks/testsfio"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



