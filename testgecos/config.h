/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testgecos"
#define	BANNER		"Test GECOS"

#define	VARPROGRAMROOT1	"TESTGECOS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTGECOS_BANNER"
#define	VARSEARCHNAME	"TESTGECOS_NAME"

#define	VARFILEROOT	"TESTGECOS_FILEROOT"
#define	VARLOGTAB	"TESTGECOS_LOGTAB"
#define	VARERRORFNAME	"TESTGECOS_ERRORFILE"

#define	VARDEBUGFNAME	"TESTGECOS_DEBUGFILE"
#define	VARDEBUGFD1	"TESTGECOS_DEBUGFD"
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
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testgecos"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testgecos"		/* mutex PID file */
#define	LOGFNAME	"var/log/testgecos"	/* activity log */
#define	LOCKFNAME	"spool/locks/testgecos"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



