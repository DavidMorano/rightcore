/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testconfigvars "
#define	BANNER		"Test ConfigVars"
#define	SEARCHNAME	"testconfigvars"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTCONFIGVARS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTCONFIGVARS_BANNER"
#define	VARSEARCHNAME	"TESTCONFIGVARS_NAME"
#define	VAROPTS		"TESTCONFIGVARS_OPTS"
#define	VARFILEROOT	"TESTCONFIGVARS_FILEROOT"
#define	VARLOGTAB	"TESTCONFIGVARS_LOGTAB"

#define	VARDEBUGFD1	"TESTCONFIGVARS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testconfigvars"		/* mutex PID file */
#define	LOGFNAME	"var/log/testconfigvars"	/* activity log */
#define	LOCKFNAME	"spool/locks/testconfigvars"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


