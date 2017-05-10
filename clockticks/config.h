/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)clockticks "

#define	VARPROGRAMROOT1	"CLOCKTICKS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CLOCKTICKS_BANNER"
#define	VARSEARCHNAME	"CLOCKTICKS_NAME"

#define	VARFILEROOT	"CLOCKTICKS_FILEROOT"
#define	VARLOGTAB	"CLOCKTICKS_LOGTAB"

#define	VARDEBUGFD1	"CLOCKTICKS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"clockticks"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/clockticks"		/* mutex PID file */
#define	LOGFNAME	"var/log/clockticks"	/* activity log */
#define	LOCKFNAME	"spool/locks/clockticks"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Clock Ticks"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




