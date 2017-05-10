/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testlog "

#define	VARPROGRAMROOT1	"TESTLOG_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTLOG_BANNER"
#define	VARSEARCHNAME	"TESTLOG_NAME"

#define	VARFILEROOT	"TESTLOG_FILEROOT"
#define	VARLOGTAB	"TESTLOG_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"TESTLOG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testlog"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testlog"		/* mutex PID file */
#define	LOGFNAME	"var/log/testlog"	/* activity log */
#define	LOCKFNAME	"spool/locks/testlog"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test Log"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



