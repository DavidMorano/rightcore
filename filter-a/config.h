/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)filter-a "
#define	BANNER		"Filter-A"
#define	SEARCHNAME	"filter-a"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FILTERA_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FILTERA_BANNER"
#define	VARSEARCHNAME	"FILTERA_NAME"
#define	VAROPTS		"FILTERA_OPTS"
#define	VARFILEROOT	"FILTERA_FILEROOT"
#define	VARLOGTAB	"FILTERA_LOGTAB"
#define	VAREFNAME	"FILTERA_EF"

#define	VARDEBUGFD1	"FILTERA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/filter-a"		/* mutex PID file */
#define	LOGFNAME	"var/log/filter-a"	/* activity log */
#define	LOCKFNAME	"spool/locks/filter-a"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


