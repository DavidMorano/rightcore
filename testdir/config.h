/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testdir "
#define	BANNER		"Test Directory Operations"
#define	SEARCHNAME	"testdir"

#define	VARPROGRAMROOT1	"TESTDIR_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTDIR_BANNER"
#define	VARSEARCHNAME	"TESTDIR_NAME"
#define	VARFILEROOT	"TESTDIR_FILEROOT"
#define	VARLOGTAB	"TESTDIR_LOGTAB"
#define	VARERRORFNAME	"TESTDIR_ERRORFILE"

#define	VARDEBUGFNAME	"TESTDIR_DEBUGFILE"
#define	VARDEBUGFD1	"TESTDIR_DEBUGFD"
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

#define	PIDFNAME	"run/testdir"
#define	LOGFNAME	"var/log/testdir"
#define	LOCKFNAME	"spool/locks/testdir"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */




