/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testsrvreg "

#define	VARPROGRAMROOT1	"TESTSRVREG_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTSRVREG_NAME"
#define	VARFILEROOT	"TESTSRVREG_FILEROOT"
#define	VARLOGTAB	"TESTSRVREG_LOGTAB"

#define	VARDEBUGFD1	"TESTSRVREG_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testsrgreg"

#define	BANNER		"Test Server Registry"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/testsrgreg"		/* mutex PID file */
#define	LOGFNAME	"var/log/testsrgreg"		/* activity log */
#define	LOCKFNAME	"spool/locks/testsrgreg"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)




