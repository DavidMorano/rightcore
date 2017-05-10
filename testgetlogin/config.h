/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)testgetlogin "

#define	VARPROGRAMROOT1	"TESTGETLOGIN_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARFILEROOT	"TESTGETLOGIN_FILEROOT"
#define	VARLOGTAB	"TESTGETLOGIN_LOGTAB"

#define	VARDEBUGFD1	"TESTGETLOGIN_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testgetlogin"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testgetlogin"		/* mutex PID file */
#define	LOGFNAME	"var/log/testgetlogin"	/* activity log */
#define	LOCKFNAME	"spool/locks/testgetlogin"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test GetLogin"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */





