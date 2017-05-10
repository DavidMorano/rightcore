/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)hello "
#define	BANNER		"Hello"
#define	SEARCHNAME	"hello"

#define	VARPROGRAMROOT1	"DATE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DATE_BANNER"
#define	VARSEARCHNAME	"DATE_NAME"
#define	VARFILEROOT	"DATE_FILEROOT"
#define	VARLOGTAB	"DATE_LOGTAB"

#define	VARDEBUGFD1	"DATE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/hello"		/* mutex PID file */
#define	LOGFNAME	"var/log/hello"		/* activity log */
#define	LOCKFNAME	"spool/locks/hello"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



