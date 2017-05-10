/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)KSHLD"
#define	BANNER		"KSH Load"
#define	SEARCHNAME	"kshld"

#define	VARPROGRAMROOT1	"KSHLD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"KSHLD_BANNER"
#define	VARSEARCHNAME	"KSHLD_NAME"

#define	VARFILEROOT	"KSHLD_FILEROOT"
#define	VARLOGTAB	"KSHLD_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"KSHLD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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

#define	PIDFNAME	"run/userinfo"		/* mutex PID file */
#define	LOGFNAME	"var/log/userinfo"	/* activity log */
#define	LOCKFNAME	"spool/locks/userinfo"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


