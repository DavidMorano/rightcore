/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testpthread "

#define	VARPROGRAMROOT1	"TESTPTHREAD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTPTHREAD_BANNER"
#define	VARSEARCHNAME	"TESTPTHREAD_NAME"

#define	VARFILEROOT	"TESTPTHREAD_FILEROOT"
#define	VARLOGTAB	"TESTPTHREAD_LOGTAB"

#define	VARDEBUGFD1	"TESTPTHREAD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testpthread"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testpthread"		/* mutex PID file */
#define	LOGFNAME	"var/log/testpthread"	/* activity log */
#define	LOCKFNAME	"spool/locks/testpthread"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test Pthread"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




