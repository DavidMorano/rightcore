/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)filtername "
#define	BANNER		"Filter Name"
#define	SEARCHNAME	"filtername"

#define	VARPROGRAMROOT1	"FILTERNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FILTERNAME_BANNER"
#define	VARSEARCHNAME	"FILTERNAME_NAME"

#define	VARFILEROOT	"FILTERNAME_FILEROOT"
#define	VARLOGTAB	"FILTERNAME_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARERRORFNAME	"FILTERNAME_ERRORFILE"

#define	VARDEBUGFNAME	"FILTERNAME_DEBUGFILE"
#define	VARDEBUGFD1	"FILTERNAME_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

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

#define	PIDFNAME	"run/filtername"		/* mutex PID file */
#define	LOGFNAME	"var/log/filtername"	/* activity log */
#define	LOCKFNAME	"spool/locks/filtername"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



