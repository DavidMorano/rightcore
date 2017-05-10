/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testcontext "

#define	VARPROGRAMROOT1	"TESTCONTEXT_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCONTEXT_BANNER"
#define	VARSEARCHNAME	"TESTCONTEXT_NAME"

#define	VARFILEROOT	"TESTCONTEXT_FILEROOT"
#define	VARLOGTAB	"TESTCONTEXT_LOGTAB"

#define	VARDEBUGFD1	"TESTCONTEXT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testcontext"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testcontext"		/* mutex PID file */
#define	LOGFNAME	"var/log/testcontext"	/* activity log */
#define	LOCKFNAME	"spool/locks/testcontext"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Login Name"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



