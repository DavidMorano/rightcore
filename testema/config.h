/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testema "
#define	BANNER		"Test EMA"
#define	SEARCHNAME	"testema"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTEMA_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTEMA_BANNER"
#define	VARSEARCHNAME	"TESTEMA_NAME"
#define	VARERRFILE	"TESTEMA_ERRFILE"
#define	VARFILEROOT	"TESTEMA_FILEROOT"
#define	VARLOGTAB	"TESTEMA_LOGTAB"
#define	VARAFNAME	"TESTEMA_AF"
#define	VAREFNAME	"TESTEMA_EF"
#define	VARERRORFNAME	"TESTEMA_ERRORFILE"

#define	VARDEBUGFNAME	"TESTEMA_DEBUGFILE"
#define	VARDEBUGFD1	"TESTEMA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

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
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testema"		/* mutex PID file */
#define	LOGFNAME	"var/log/testema"	/* activity log */
#define	LOCKFNAME	"spool/locks/testema"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"
#define	PO_HEADER	"header"
#define	PO_SUBPART	"subpart"


