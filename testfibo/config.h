/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testfibo "

#define	VARPROGRAMROOT1	"TESTFIBO_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTFIBO_BANNER"
#define	VARSEARCHNAME	"TESTFIBO_NAME"

#define	VARFILEROOT	"TESTFIBO_FILEROOT"
#define	VARLOGTAB	"TESTFIBO_LOGTAB"

#define	VARDEBUGFD1	"TESTFIBO_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testfibo"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testfibo"		/* mutex PID file */
#define	LOGFNAME	"var/log/testfibo"	/* activity log */
#define	LOCKFNAME	"spool/locks/testfibo"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Login Name"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



